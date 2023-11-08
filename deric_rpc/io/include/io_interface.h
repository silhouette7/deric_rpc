#ifndef _IO_INTERFACE_H_
#define _IO_INTERFACE_H_

namespace deric
{
namespace rpc
{
typedef enum {
    IO_MEMBER_ERROR_EPOLL_FAILED = 0
} IoMemberError_e;

typedef enum {
    IO_MEMBER_ERROR_ACTION_NULL = 0,
    IO_MEMBER_ERROR_ACTION_DELETE = 1
} IoMemberErrorAction;

typedef enum {
    TCP_IO_CONNECTION_EVENT_CLOSE = 0
} IoConnectionEvent_e;

class IoMember
{
public:
    virtual ~IoMember() {}

    virtual int getFd() = 0;

    virtual int onReadAvailable() = 0;

    virtual int onWriteAvailable() = 0;

    virtual int onControlAvailable() = 0;

    virtual int sendData(const char* data, int len) = 0;

    virtual IoMemberErrorAction onIoError(IoMemberError_e) = 0;
};
}
}

#endif