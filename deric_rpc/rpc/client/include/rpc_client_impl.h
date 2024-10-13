#ifndef _RPC_CLIENT_IMPL_H_
#define _RPC_CLIENT_IMPL_H_

#include "client_interface.h"
#include "tcp_io_connection.h"

#include <memory>

namespace deric::rpc
{
class RpcClientImpl : public ClientInterface
{
public:
    RpcClientImpl() = default;

    ~RpcClientImpl();

    int connect(std::string_view ip, std::string_view port) override;

    int disconnect() override;

    int sendMsg(std::string_view msg) override;

    int registerMessageCallback(const ClientMessageCallbackType& func) override;

    int registerMessageCallback(ClientMessageCallbackType&& func) override;

    void handleData(const std::shared_ptr<TcpIoConnection>& connect, const std::string& data);

    void handleConnect(const std::shared_ptr<TcpIoConnection>& connect);

    void handleClose(const std::shared_ptr<TcpIoConnection>& connect);

private:
    std::shared_ptr<TcpIoConnection> m_ioConnection;
    ClientMessageCallbackType m_msgCallback;
};
}

#endif