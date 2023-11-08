#ifndef _IO_SERVER_CLIENT_INTERFACE_H_
#define _IO_SERVER_CLIENT_INTERFACE_H_

#include <memory>

#include "io_connection_client_interface.h"

namespace deric
{
namespace rpc
{
class IoServerClientInterface
{
public:
    virtual ~IoServerClientInterface() {}

    virtual void createIoConnectionClient(std::shared_ptr<IoConnectionClientInterface>& connectionClient) = 0;

    virtual void removeIoConnectionClient(std::shared_ptr<IoConnectionClientInterface> connectionClient) = 0;
};
}
}

#endif