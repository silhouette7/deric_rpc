#include <unistd.h>
#include <thread>
#include <vector>

#include "deric_debug.h"
#include "io_module.h"

static const char IO_MODULE_STOP_FLAG = '0';
static const char IO_MODULE_WAKEUP_FLAG = '1';

namespace deric
{
namespace rpc
{
    IoModule::IoModule() :
        m_state(COMPONENT_STATE_CREATED),
        m_instanceMutex(),
        m_ioMembersMutex(),
        m_wakeUpPipeFd{-1, -1},
        m_epollFd(-1),
        m_maxEpollSize(0),
        m_epollEvents(nullptr),
        m_ioMembers()
    {
        DEBUG_INFO("construct");
    }

    IoModule::~IoModule() {
    }

    IoModule& IoModule::getInstance() {
        static IoModule m_IoModule;
        return m_IoModule;
    }

    int IoModule::init(const IoModuleConfig_s& config) {
        int res = 0;
        m_instanceMutex.lock();
    
        do {
            if (m_state >= COMPONENT_STATE_INITIALIZED) {
                DEBUG_INFO("already initialized");
                break;
            }

            res = pipe(m_wakeUpPipeFd);
            if (0 > res) {
                DEBUG_ERROR("pipe create fail, res: %d", res);
                break;
            }

            m_epollFd = epoll_create1(0);
            if (0 > m_epollFd) {
                DEBUG_ERROR("epoll create fail, res: %d", m_epollFd);
                close(m_wakeUpPipeFd[0]);
                close(m_wakeUpPipeFd[1]);
                res = -1;
                break;
            }

            m_maxEpollSize = config.epollSize;
            m_epollEvents = new struct epoll_event[m_maxEpollSize];
            if (!m_epollEvents) {
                DEBUG_ERROR("no enough memory to alloc epoll events");
                close(m_epollFd);
                close(m_wakeUpPipeFd[0]);
                close(m_wakeUpPipeFd[1]);
                res = -1;
                break;
            }

            m_state = COMPONENT_STATE_INITIALIZED;
        } while(0);

        DEBUG_INFO("io module initialize res: %d", res);
        m_instanceMutex.unlock();
        return res;
    }

    int IoModule::deinit() {
        int res = 0;
        m_instanceMutex.lock();
    
        do {
            if (m_state != COMPONENT_STATE_INITIALIZED) {
                DEBUG_ERROR("invalid state %d", m_state);
                res = -1;
                break;
            }

            close(m_epollFd);
            m_epollFd = -1;

            if (m_epollEvents) {
                delete []m_epollEvents;
                m_epollEvents = nullptr;
            }

            close(m_wakeUpPipeFd[0]);
            m_wakeUpPipeFd[0] = 0;
            close(m_wakeUpPipeFd[1]);
            m_wakeUpPipeFd[1] = 0;

            m_state = COMPONENT_STATE_CREATED;
        } while(0);

        DEBUG_INFO("io module deinitialize res: %d", res);
        m_instanceMutex.unlock();
        return res;
    }

    int IoModule::start() {
        int res = 0;
        m_instanceMutex.lock();

        do {
            if (m_state != COMPONENT_STATE_INITIALIZED) {
                DEBUG_ERROR("invalid state: %d", m_state);
                res = -1;
                break;
            }

            res = modifyEpollEvent(EPOLL_CTL_ADD, m_wakeUpPipeFd[0], EPOLLIN);
            if (res < 0) {
                DEBUG_ERROR("add pipe failed");
                break;
            }
            std::thread ioThread(&IoModule::waitThreadFunc, this);
            ioThread.detach();
        } while(0);

        DEBUG_INFO("io module start res: %d", res);
        m_instanceMutex.unlock();
        return res;
    }

    int IoModule::stop() {
        int res = 0;
        m_instanceMutex.lock();

        do {
            if (m_state < COMPONENT_STATE_STARTED) {
                DEBUG_ERROR("io module is not stated yet");
                res = -1;
                break;
            }
            
            if (m_wakeUpPipeFd[1] < 0) {
                DEBUG_ERROR("invalid wake up fd");
                res = -1;
                break;
            }

            res = write(m_wakeUpPipeFd[1], &IO_MODULE_STOP_FLAG, 1);
            if (res < 0) {
                DEBUG_ERROR("fail to write wake up fd");
                break;
            }

            m_state = COMPONENT_STATE_INITIALIZED;
        } while(0);

        m_instanceMutex.unlock();
        return res;
    }

    void IoModule::wakeupModule() {
        m_instanceMutex.lock();
        if (m_state < COMPONENT_STATE_STARTED) {
            DEBUG_ERROR("io module is not stated yet");
        }
        else if (m_wakeUpPipeFd[1] < 0) {
            DEBUG_ERROR("invalid wake up fd");
        }
        else if (0 > write(m_wakeUpPipeFd[1], &IO_MODULE_WAKEUP_FLAG, 1)) {
            DEBUG_ERROR("fail to write wake up fd");
        }
        m_instanceMutex.unlock();
    }

    int IoModule::addIoMember(std::shared_ptr<IoMember> ioMember) {
        int res = 0;
        int fd = -1;

        do {
            if (!ioMember) {
                DEBUG_ERROR("error parameter");
                res = -1;
                break;
            }

            fd = ioMember->getFd();
            if (fd < 0) {
                DEBUG_ERROR("error parameter");
                res = -1;
                break;
            }

            m_ioMembersMutex.lock();
            if (m_ioMembers.find(fd) != m_ioMembers.end()) {
                DEBUG_ERROR("io member of fd: %d has been added already", fd);
                break;
            }
            m_ioMembers[fd] = ioMember;
            m_ioMembersMutex.unlock();

            res = modifyEpollEvent(EPOLL_CTL_ADD, fd, EPOLLIN);

            DEBUG_INFO("io member of fd: %d has been added successfully", fd);

            wakeupModule();
        } while(0);

        return res;
    }

    int IoModule::removeIoMember(std::shared_ptr<IoMember> ioMember) {
        int res = 0;
        int fd = -1;

        do {
            if (!ioMember) {
                DEBUG_ERROR("error parameter");
                res = -1;
                break;
            }
            fd = ioMember->getFd();

            m_ioMembersMutex.lock();
            if (m_ioMembers.find(fd) == m_ioMembers.end()) {
                DEBUG_ERROR("io member of fd: %d has not been added before", fd);
                res = -1;
                break;
            }
            m_ioMembers.erase(fd);
            m_ioMembersMutex.unlock();

            res = modifyEpollEvent(EPOLL_CTL_DEL, fd, EPOLLIN);
        } while(0);

        return res;
    }

    void IoModule::waitThreadFunc() {
        m_instanceMutex.lock();
        DEBUG_INFO("start io module thread");
        m_state = COMPONENT_STATE_STARTED;
        m_instanceMutex.unlock();

        int epoll_count = 0;
        int i = 0;
        int cur_fd = -1;
        unsigned int cur_events = 0;
        char wakeup_flag = IO_MODULE_WAKEUP_FLAG;
        std::vector<int> close_fds;
        std::shared_ptr<IoMember> cur_member(nullptr);

        while (true) {
            epoll_count = epoll_wait(m_epollFd, m_epollEvents, m_maxEpollSize, -1);
            for (i = 0; i < epoll_count; ++i) {
                cur_fd = m_epollEvents[i].data.fd;
                cur_events = m_epollEvents[i].events;

                if (cur_fd == m_wakeUpPipeFd[0]) {
                    DEBUG_INFO("wake up");
                    (void)read(cur_fd, &wakeup_flag, 1);
                    continue;
                }

                {
                    std::lock_guard<std::mutex> g(m_ioMembersMutex);
                    if (m_ioMembers.find(cur_fd) == m_ioMembers.end()) {
                        DEBUG_ERROR("wrong fd");
                        continue;
                    }
                    else {
                        cur_member = m_ioMembers[cur_fd];
                    }
                }

                if (cur_member) {
                    if ((cur_events & EPOLLERR) || (cur_events & EPOLLHUP) || (!(cur_events & EPOLLIN))) {
                        DEBUG_ERROR("epoll event error: 0x%x happened in fd: %d", cur_events, cur_fd);
                        if (IO_MEMBER_ERROR_ACTION_DELETE == cur_member->onIoError(IO_MEMBER_ERROR_EPOLL_FAILED)) {
                            DEBUG_INFO("disconnected fd: %d", cur_fd);
                            close_fds.push_back(cur_fd);
                        }
                        continue;
                    }

                    int res = cur_member->onReadAvailable();
                    if (0 > res) {
                        DEBUG_ERROR("fail to handle event");
                    }
                    cur_member = nullptr;
                }
            }

            for(int fd : close_fds) {                
                (void)modifyEpollEvent(EPOLL_CTL_DEL, fd, EPOLLIN);
                m_ioMembersMutex.lock();
                m_ioMembers.erase(fd);
                m_ioMembersMutex.unlock();
            }
            close_fds.clear();

            if (wakeup_flag == IO_MODULE_STOP_FLAG) {
                break;
            }
        }

        m_instanceMutex.lock();
        m_state = COMPONENT_STATE_INITIALIZED;
        DEBUG_INFO("exit");
        m_instanceMutex.unlock();
    }

    int IoModule::modifyEpollEvent(int op, int fd, unsigned int event) {
        int res = -1;
        do {
            if ((m_epollFd < 0) || (m_state < COMPONENT_STATE_INITIALIZED)) {
                DEBUG_ERROR("epoll is not created yet");
                res = -1;
                break;
            }

            if (fd < 0) {
                DEBUG_ERROR("invalid fd: %d", fd);
                res = -1;
                break;
            }

            struct epoll_event ev;
            ev.events = event;
            ev.data.fd = fd;
            res = epoll_ctl(m_epollFd, op, fd, &ev);
            if (res < 0) {
                DEBUG_ERROR("epoll ctl operation: %d for fd: %d to %d failed, res: %d", op, fd, m_epollFd, res);
            }
        } while(0);

        return res;
    }
}
}