#ifndef _RPC_SERVICE_IMPL_H_
#define _RPC_SERVICE_IMPL_H_

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

#include "component_public.h"
#include "tcp_io_server.h"
#include "tcp_io_connection.h"
#include "service_interface.h"
#include "service_client_entry_interface.h"

namespace deric
{
namespace rpc
{
class RpcServiceImpl : public ServiceInterface,
                       public IoServerClientInterface,
                       public std::enable_shared_from_this<RpcServiceImpl>
{
public:
    RpcServiceImpl(const std::string& serviceName);

    ~RpcServiceImpl() override;

    RpcServiceImpl(const RpcServiceImpl&) = delete;

    RpcServiceImpl(RpcServiceImpl&&) = delete;

    RpcServiceImpl& operator=(const RpcServiceImpl&) = delete;

    int init(const ServiceConfig_s &serviceConfig) override;

    int deInit() override;

    int start() override;

    int stop() override;

    int registerMethod(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func) override;

    int unregisterMethod(const std::string& funcName) override;

    int postEvent(const std::string& eventName, const char* pData, int len) override;

    void createIoConnectionClient(std::shared_ptr<IoConnectionClientInterface>& connectionClient) override;

    void removeIoConnectionClient(std::shared_ptr<IoConnectionClientInterface> connectionClient) override;

    void removeIoConnectionClient(uint32_t clientId);

    int handleData(const char* pData, int len, std::string& resultString);

private:
    int handleCommand(const std::string& funcName, const char* pData, int len, std::string& resultString);

    int handleRegister(const std::string& eventName, const char* pData, int len, std::string& resultString);

    uint32_t getNewClientId();

    std::string m_serviceName;
    ComponentState_e m_state;
    std::shared_ptr<RpcSerialer> m_serialer;
    std::shared_ptr<TcpIoServer> m_ioServer;
    std::unordered_map<uint32_t, std::shared_ptr<ServiceClientEntryInterface>> m_clients;
    std::unordered_map<std::string, std::function<int (const char*, int, std::string&)>> m_functions;
    std::mutex m_instanceMutex;
    std::mutex m_clientsMutex;
    std::mutex m_functionsMutex;
};
}
}

#endif