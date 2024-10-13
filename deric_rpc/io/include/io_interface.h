#ifndef _IO_INTERFACE_H_
#define _IO_INTERFACE_H_

#include <string_view>

namespace deric
{
typedef enum {
    IO_MEMBER_ERROR_POLL_FAILED = 0
} IoMemberError_e;

class IoMember
{
public:
    virtual ~IoMember() {}

    virtual int getFd() = 0;

    virtual void onReadAvailable() = 0;

    virtual void onWriteAvailable() = 0;

    virtual void onControlAvailable() = 0;

    virtual void onIoError(IoMemberError_e) = 0;

    virtual int sendData(std::string_view) = 0;
};
}

#endif