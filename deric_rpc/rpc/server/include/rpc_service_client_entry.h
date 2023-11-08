#ifndef _RPC_SERVICE_CLIENT_ENTRY_
#define _RPC_SERVICE_CLIENT_ENTRY_

#include <memory>

#include "io_connection_client_interface.h"
#include "service_client_entry_interface.h"
#include "tcp_io_connection.h"
#include "rpc_service_impl.h"

namespace deric
{
namespace rpc
{
class RpcServiceClientEntry : public IoConnectionClientInterface,
                              public ServiceClientEntryInterface
{
public:
    RpcServiceClientEntry(uint32_t clientId, std::weak_ptr<RpcServiceImpl> serviceImpl);

    ~RpcServiceClientEntry();

    int handleEvent(IoConnectionEvent_e event, void* data) override;

    int handleData(const char *pData, int len) override;

    int startIoClient() override;

    int stopIoClient() override;
 
    int startClientEntry() override;

    int stopClientEntry() override;

    uint32_t getClientId() override;

    int sendData(const std::string& data) override;

    void setIoConnection(std::shared_ptr<IoMember> ioConnection);

private:
    int start();

    int stop();

    uint32_t m_clientId;
    std::weak_ptr<RpcServiceImpl> m_serviceImpl;
    std::shared_ptr<TcpIoConnection> m_ioConnection;
};
}
}


#endif