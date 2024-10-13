#ifndef _IO_MODULE_H_
#define _IO_MODULE_H_

#include "io_interface.h"

#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>

namespace deric
{
typedef struct {
    int epollSize;
} IoModuleConfig_s;

class IoModule
{
public:
    static IoModule& getInstance();

    int init(const IoModuleConfig_s& config);

    int deinit();

    int start();

    int stop();

    void wakeupModule();

    int addIoMember(std::shared_ptr<IoMember> ioMember);

    int removeIoMember(std::shared_ptr<IoMember> IoMember);

private:
    enum class State {
        CREATED,
        INITIALIZED,
        STARTED
    };

    void waitThreadFunc();

    int modifyEpollEvent(int op, int fd, unsigned int event);

    IoModule();

    ~IoModule();

    IoModule(const IoModule&) = delete;

    IoModule(IoModule&&) = delete;

    IoModule operator=(const IoModule&) = delete;

    State m_state;
    std::mutex m_instanceMutex;
    std::mutex m_ioMembersMutex;
    int m_wakeUpPipeFd[2];
    int m_epollFd;
    int m_maxEpollSize;
    struct epoll_event* m_epollEvents;
    std::unordered_map<int, std::shared_ptr<IoMember>> m_ioMembers;
};
}

#endif