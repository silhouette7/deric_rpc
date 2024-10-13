#include <unistd.h>
#include <thread>
#include <vector>

#include "deric_debug.h"
#include "io_module.h"

static constexpr char IO_MODULE_STOP_FLAG = '0';
static constexpr char IO_MODULE_WAKEUP_FLAG = '1';

namespace deric
{
    IoModule::IoModule() :
        m_state(State::CREATED),
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
        std::lock_guard<std::mutex> g(m_instanceMutex);
        int pipeFd[2] = {0, 0};
        int epollFd = 0;

        if (m_state >= State::INITIALIZED) {
            DEBUG_INFO("already initialized");
            return -1;
        }

        if (0 > pipe(pipeFd)) {
            DEBUG_ERROR("pipe create fail");
            return -1;
        }

        epollFd = epoll_create1(0);
        if (0 > epollFd) {
            DEBUG_ERROR("epoll create fail, res: %d", epollFd);
            close(pipeFd[0]);
            close(pipeFd[1]);
            return -1;
        }

        m_maxEpollSize = config.epollSize;
        m_epollEvents = new(std::nothrow) struct epoll_event[m_maxEpollSize];
        if (!m_epollEvents) {
            DEBUG_ERROR("no enough memory to alloc epoll events");
            close(epollFd);
            close(pipeFd[0]);
            close(pipeFd[1]);
            return -1;
        }

        m_epollFd = epollFd;
        m_wakeUpPipeFd[0] = pipeFd[0];
        m_wakeUpPipeFd[1] = pipeFd[1];
        m_state = State::INITIALIZED;

        DEBUG_INFO("io module initialize successfully");
        return 0;
    }

    int IoModule::deinit() {
        std::lock_guard<std::mutex> g(m_instanceMutex);
    
        if (m_state != State::INITIALIZED) {
            DEBUG_ERROR("invalid state");
            return -1;
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

        m_state = State::CREATED;

        DEBUG_INFO("io module deinitialize successfully");
        return 0;
    }

    int IoModule::start() {
        std::lock_guard<std::mutex> g(m_instanceMutex);

        if (m_state != State::INITIALIZED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (0 > modifyEpollEvent(EPOLL_CTL_ADD, m_wakeUpPipeFd[0], EPOLLIN)) {
            DEBUG_ERROR("add pipe failed");
            return -1;
        }
        std::thread ioThread(&IoModule::waitThreadFunc, this);
        ioThread.detach();

        DEBUG_INFO("io module start successfully");
        return 0;
    }

    int IoModule::stop() {
        std::lock_guard<std::mutex> g(m_instanceMutex);

        if (m_state < State::STARTED) {
            DEBUG_ERROR("io module is not stated yet");
            return -1;
        }
        
        if (m_wakeUpPipeFd[1] < 0) {
            DEBUG_ERROR("invalid wake up fd");
            return -1;
        }

        if (0 > write(m_wakeUpPipeFd[1], &IO_MODULE_STOP_FLAG, 1)) {
            DEBUG_ERROR("fail to write wake up fd");
            return -1;
        }

        m_state = State::INITIALIZED;

        return 0;
    }

    void IoModule::wakeupModule() {
        std::lock_guard<std::mutex> g(m_instanceMutex);
        if (m_state < State::STARTED) {
            DEBUG_ERROR("io module is not stated yet");
        }
        else if (m_wakeUpPipeFd[1] < 0) {
            DEBUG_ERROR("invalid wake up fd");
        }
        else if (0 > write(m_wakeUpPipeFd[1], &IO_MODULE_WAKEUP_FLAG, 1)) {
            DEBUG_ERROR("fail to write wake up fd");
        }
    }

    int IoModule::addIoMember(std::shared_ptr<IoMember> ioMember) {
        if (!ioMember) {
            DEBUG_ERROR("error parameter");
            return -1;
        }

        int fd = ioMember->getFd();
        if (fd < 0) {
            DEBUG_ERROR("error parameter");
            return -1;
        }

        {
            std::lock_guard<std::mutex> g(m_ioMembersMutex);
            if (m_ioMembers.find(fd) != m_ioMembers.end()) {
                DEBUG_ERROR("io member of fd: %d has been added already", fd);
                return -1;
            }
            m_ioMembers[fd] = ioMember;
        }

        modifyEpollEvent(EPOLL_CTL_ADD, fd, EPOLLIN);

        DEBUG_INFO("io member of fd: %d has been added successfully", fd);

        wakeupModule();

        return 0;
    }

    int IoModule::removeIoMember(std::shared_ptr<IoMember> ioMember) {
        if (!ioMember) {
            DEBUG_ERROR("error parameter");
            return -1;
        }

        int fd = ioMember->getFd();
        {
            std::lock_guard<std::mutex> g(m_ioMembersMutex);
            if (m_ioMembers.find(fd) == m_ioMembers.end()) {
                DEBUG_ERROR("io member of fd: %d has not been added before", fd);
                return -1;
            }
            m_ioMembers.erase(fd);
        }

        modifyEpollEvent(EPOLL_CTL_DEL, fd, EPOLLIN);
        DEBUG_INFO("io member fd - %d is removed", fd);
        return 0;
    }

    void IoModule::waitThreadFunc() {
        m_instanceMutex.lock();
        DEBUG_INFO("start io module thread");
        m_state = State::STARTED;
        m_instanceMutex.unlock();

        char wakeup_flag = IO_MODULE_WAKEUP_FLAG;
        std::shared_ptr<IoMember> cur_member(nullptr);

        while (true) {
            int epoll_count = epoll_wait(m_epollFd, m_epollEvents, m_maxEpollSize, -1);
            for (int i = 0; i < epoll_count; ++i) {
                int cur_fd = m_epollEvents[i].data.fd;
                unsigned int cur_events = m_epollEvents[i].events;

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
                    if ((cur_events & EPOLLERR)) {
                        DEBUG_ERROR("epoll event error: 0x%x happened in fd: %d", cur_events, cur_fd);
                        cur_member->onIoError(IO_MEMBER_ERROR_POLL_FAILED);
                        continue;
                    }

                    cur_member->onReadAvailable();
                    cur_member = nullptr;
                }
            }

            if (wakeup_flag == IO_MODULE_STOP_FLAG) {
                break;
            }
        }

        m_state = State::INITIALIZED;
        DEBUG_INFO("exit");
    }

    int IoModule::modifyEpollEvent(int op, int fd, unsigned int event) {
        if ((m_epollFd < 0) || (m_state < State::INITIALIZED)) {
            DEBUG_ERROR("epoll is not created yet");
            return -1;
        }

        if (fd < 0) {
            DEBUG_ERROR("invalid fd: %d", fd);
            return -1;
        }

        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        if (0 > epoll_ctl(m_epollFd, op, fd, &ev)) {
            DEBUG_ERROR("epoll ctl operation: %d for fd: %d to %d failed", op, fd, m_epollFd);
            return -1;
        }

        return 0;
    }
}