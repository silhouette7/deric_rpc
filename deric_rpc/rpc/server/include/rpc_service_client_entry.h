#ifndef _RPC_SERVICE_CLIENT_ENTRY_
#define _RPC_SERVICE_CLIENT_ENTRY_

#include "tcp_io_connection.h"

#include <memory>
#include <string>
#include <string_view>

namespace deric::rpc
{
class RpcServiceImpl;

class RpcServiceClientEntry : public std::enable_shared_from_this<RpcServiceClientEntry>
{
public:
    RpcServiceClientEntry(uint32_t clientId, RpcServiceImpl& serviceImpl, const std::shared_ptr<TcpIoConnection>& connect);

    ~RpcServiceClientEntry();
 
    int start();

    int stop();

    uint32_t getClientId();

    void handleData(const std::shared_ptr<TcpIoConnection>& connect, const std::string& data);

    void handleConnect(const std::shared_ptr<TcpIoConnection>& connect);

    int sendData(const std::string& data);

private:
    uint32_t m_clientId;
    RpcServiceImpl& m_serviceImpl;
    std::shared_ptr<TcpIoConnection> m_ioConnection;
};
}


#endif