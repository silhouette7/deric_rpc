#ifndef _RPC_SERVICE_IMPL_H_
#define _RPC_SERVICE_IMPL_H_

#include <optional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "rpc_service_client_entry.h"
#include "service_interface.h"
#include "tcp_io_server.h"
#include "tcp_io_connection.h"

namespace deric::rpc
{
class RpcServiceImpl : public ServiceInterface
{
public:
    constexpr static int MAX_CONNECTION_NUMBER = 1000;

    RpcServiceImpl(std::string_view serviceName, std::string_view ip, int port, int maxConnectionNumber = MAX_CONNECTION_NUMBER);

    ~RpcServiceImpl();

    RpcServiceImpl(const RpcServiceImpl&) = delete;

    RpcServiceImpl& operator=(const RpcServiceImpl&) = delete;

    int start() override;

    int stop() override;

    int registerMethod(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func) override;

    int registerMethod(const std::string& funcName, ServiceFunctionType&& func) override;

    int unregisterMethod(const std::string& funcName) override;

    int registerEvent(const std::string& eventName) override;

    int unregisterEvent(const std::string& eventName) override;

    int postEvent(const std::string& eventName) override;

    int clientRegisterToEvent(const std::string& eventName, uint32_t clientId);

    int clientUnregisterToEvent(const std::string& eventName, uint32_t clientId);

    void handleNewConnect(const std::shared_ptr<TcpIoConnection>& connect);

    void handleClientClose(const uint32_t clientId);

    void handleIoServerEvent(const TcpIoServer::TcpIoServerEvent& event);

    std::optional<std::string> handleCommand(const std::string& cmd, std::string_view data);

private:
    void onHandleClientClose(const uint32_t clientId);

    uint32_t getNewClientId();

    int getNewMessageId();

    std::string m_serviceName;
    std::unique_ptr<TcpIoServer> m_ioServer;
    std::unordered_map<uint32_t, std::shared_ptr<RpcServiceClientEntry>> m_clients;
    std::unordered_map<std::string, ServiceInterface::ServiceFunctionType> m_functions;
    std::unordered_map<std::string, std::vector<uint32_t>> m_events;
    std::mutex m_clientsMutex;
    std::mutex m_functionsMutex;
    std::mutex m_eventMutex;
};
}

#endif