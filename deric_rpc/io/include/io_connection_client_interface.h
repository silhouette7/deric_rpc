#ifndef _IO_CONNECTION_CLIENT_INTERFACE_
#define _IO_CONNECTION_CLIENT_INTERFACE_

#include "io_interface.h"
#include <memory>

namespace deric
{
namespace rpc
{
class IoConnectionClientInterface
{
public:
    virtual ~IoConnectionClientInterface() {}

    virtual int handleEvent(IoConnectionEvent_e event, void* data) = 0;

    virtual int handleData(const char *pData, int len) = 0;

    virtual void setIoConnection(std::shared_ptr<IoMember> ioConnection) = 0;

    virtual int startIoClient() = 0;

    virtual int stopIoClient() = 0;
};
}
}

#endif