#ifndef _RPC_CLIENT_IMPL_H_
#define _RPC_CLIENT_IMPL_H_

#include "client_interface.h"
#include "tcp_io_connection.h"
#include <memory>

namespace deric
{
namespace rpc
{
class RpcClientImpl : public ClientInterface,
                      public IoConnectionClientInterface,
                      public std::enable_shared_from_this<RpcClientImpl>
{
public:
    int connect(const ClientConnectionConfig_s& _config) override;

    int disconnect() override;

    int sendMsg(const std::string& msg) override;

    int registerMessageCallback(const ClientMessageCallbackType& func) override;

    int handleEvent(IoConnectionEvent_e event, void* data) override;

    int handleData(const char *pData, int len) override;

    void setIoConnection(std::shared_ptr<IoMember> ioConnection) override;

    int startIoClient() override;

    int stopIoClient() override;

private:
    std::shared_ptr<TcpIoConnection> m_ioConnection;
    ClientMessageCallbackType m_msgCallback;
};
}
}

#endif