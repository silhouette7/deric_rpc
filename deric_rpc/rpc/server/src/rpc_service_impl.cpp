#include "rpc_service_impl.h"

#include "deric_debug.h"
#include "thread_pool.h"

namespace deric::rpc
{
    RpcServiceImpl::RpcServiceImpl(std::string_view serviceName, std::string_view ip, int port, int maxConnectionNumber) :
        m_serviceName(serviceName),
        m_ioServer(std::make_unique<TcpIoServer>(ip, port, maxConnectionNumber)),
        m_clients(),
        m_functions(),
        m_events(),
        m_clientsMutex(),
        m_functionsMutex(),
        m_eventMutex()
    {
        m_ioServer->setNewConnectCallback([this](const std::shared_ptr<TcpIoConnection>& connect){handleNewConnect(connect);});
        m_ioServer->setEventCallback([this](const TcpIoServer::TcpIoServerEvent& event){handleIoServerEvent(event);});

        DEBUG_INFO("construct");
    }

    RpcServiceImpl::~RpcServiceImpl() {
        DEBUG_INFO("deconstruct");
    }

    int RpcServiceImpl::start() {
        if (m_ioServer) {
            if (0 > m_ioServer->start()) {
                DEBUG_ERROR("start io server fail");
                return -1;
            }
        }

        return 0;
    }

    int RpcServiceImpl::stop() {
        {
            std::lock_guard<std::mutex> g(m_clientsMutex);
            for (auto& client : m_clients) {
                if (0 > client.second->stop()) {
                    DEBUG_ERROR("enable to stop client: %u", client.first);
                }
            }
            m_clients.clear();
        }

        if (m_ioServer) {
            if (0 > m_ioServer->stop()) {
                DEBUG_ERROR("stop io server fail");
            }
        }

        return 0;
    }

    int RpcServiceImpl::registerMethod(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func) {
        std::lock_guard<std::mutex> g(m_functionsMutex);

        if (m_functions.find(funcName) != m_functions.end()) {
            DEBUG_ERROR("func: %s exit already", funcName.c_str());
            return -1;
        }

        DEBUG_INFO("func: %s is registered", funcName.c_str());
        m_functions[funcName] = func;

        return 0;
    }

    int RpcServiceImpl::registerMethod(const std::string& funcName, ServiceInterface::ServiceFunctionType&& func) {
        std::lock_guard<std::mutex> g(m_functionsMutex);

        if (m_functions.find(funcName) != m_functions.end()) {
            DEBUG_ERROR("func: %s exit already", funcName.c_str());
            return -1;
        }

        DEBUG_INFO("func: %s is registered", funcName.c_str());
        m_functions[funcName] = std::forward<ServiceInterface::ServiceFunctionType>(func);

        return 0;
    }

    int RpcServiceImpl::unregisterMethod(const std::string& funcName) {
        std::lock_guard<std::mutex> g(m_functionsMutex);

        if (m_functions.find(funcName) == m_functions.end()) {
            DEBUG_ERROR("func: %s not add yet", funcName.c_str());
            return -1;
        }

        DEBUG_INFO("func: %s is removed", funcName.c_str());
        m_functions.erase(funcName);
        return 0;
    }

    int RpcServiceImpl::registerEvent(const std::string& eventName) {
        std::lock_guard<std::mutex> g(m_eventMutex);

        if (m_events.find(eventName) != m_events.end()) {
            DEBUG_ERROR("event: %s exit already", eventName.c_str());
            return -1;
        }

        DEBUG_INFO("event: %s is registered", eventName.c_str());
        m_events[eventName] = {};
        return 0;
    }

    int RpcServiceImpl::unregisterEvent(const std::string& eventName) {
        std::lock_guard<std::mutex> g(m_eventMutex);

        if (m_events.find(eventName) == m_events.end()) {
            DEBUG_ERROR("event: %s not add yet", eventName.c_str());
            return -1;
        }

        DEBUG_INFO("event: %s is removed", eventName.c_str());
        m_events.erase(eventName);
        return 0;
    }

    int RpcServiceImpl::postEvent(const std::string& eventName) {
        std::vector<uint32_t> clients;
        {
            std::lock_guard<std::mutex> gEvents(m_eventMutex);
            if (m_events.find(eventName) == m_events.end()) {
                DEBUG_ERROR("unknown event: %s", eventName.c_str());
                return -1;
            }

            clients = m_events[eventName];
        }

        serialer::MessageMeta meta = {MessageType::NOTIFY, eventName, getNewMessageId(), true};
        std::string metaStr = serialer::serialMessageData(meta);
        std::string eventStr = serialer::generateRpcStr(metaStr, "");
        std::lock_guard<std::mutex> gClients(m_clientsMutex);
        for (uint32_t client : clients) {
            auto iter = m_clients.find(client);
            if (iter != m_clients.end()) {
                iter->second->sendData(eventStr);
            }
        }

        return 0;
    }

    int RpcServiceImpl::clientRegisterToEvent(const std::string& eventName, uint32_t clientId) {
        std::lock_guard<std::mutex> g(m_eventMutex);

        if (m_events.find(eventName) == m_events.end()) {
            DEBUG_ERROR("event: %s not add yet", eventName.c_str());
            return -1;
        }

        DEBUG_INFO("client: %u register to event: %s", clientId, eventName.c_str());
        m_events[eventName].push_back(clientId);
        return 0;
    }

    int RpcServiceImpl::clientUnregisterToEvent(const std::string& eventName, uint32_t clientId) {
        std::lock_guard<std::mutex> g(m_eventMutex);

        if (m_events.find(eventName) == m_events.end()) {
            DEBUG_ERROR("event: %s not add yet", eventName.c_str());
            return -1;
        }

        DEBUG_INFO("client: %u unregister to event: %s", clientId, eventName.c_str());
        auto& regitserClients = m_events[eventName];
        regitserClients.erase(std::remove(regitserClients.begin(), regitserClients.end(), clientId));
        return 0;
    }

    void RpcServiceImpl::handleNewConnect(const std::shared_ptr<TcpIoConnection>& connect) {
        if (!connect) {
            return;
        }

        uint32_t clientId = getNewClientId();
        auto spClient = std::make_shared<RpcServiceClientEntry>(clientId, *this, connect);
        spClient->start();
        DEBUG_INFO("add client of id: %u", clientId);
        std::lock_guard<std::mutex> g(m_clientsMutex);
        m_clients[clientId] = std::move(spClient);
    }

    void RpcServiceImpl::handleClientClose(const uint32_t clientId) {
        ThreadPool::getInstance().commit([this, clientId](){onHandleClientClose(clientId);});
    }

    void RpcServiceImpl::handleIoServerEvent(const TcpIoServer::TcpIoServerEvent& event) {
        switch (event)
        {
            case TcpIoServer::TcpIoServerEvent::SERVER_IO_ERROR:
            {
                stop();
                break;
            }
            default:
                break;
        }
    }

    std::optional<std::string> RpcServiceImpl::handleCommand(const std::string& cmd, std::string_view data) {
        ServiceInterface::ServiceFunctionType func;

        {
            std::lock_guard<std::mutex> g(m_functionsMutex);
            if (m_functions.find(cmd) == m_functions.end()) {
                DEBUG_ERROR("unknown function: %s", cmd.c_str());
            }
            else {
                func = m_functions[cmd];
            }
        }

        if (func) {
            return func(data);
        }
        else {
            throw "invalid msg id";
        }
    }

    void RpcServiceImpl::onHandleClientClose(const uint32_t clientId) {
        {
            std::lock_guard<std::mutex> gEvent(m_eventMutex);
            for (auto& item : m_events) {
                auto& clients = item.second;
                clients.erase(std::remove(clients.begin(), clients.end(), clientId), clients.end());
            }
        }

        std::lock_guard<std::mutex> gClient(m_clientsMutex);
        if (m_clients.find(clientId) == m_clients.end()) {
            DEBUG_ERROR("unknown client id: %u", clientId);
            return;
        }
        DEBUG_INFO("erase client of id: %u", clientId);
        m_clients[clientId]->stop();
        m_clients.erase(clientId);
    }

    uint32_t RpcServiceImpl::getNewClientId()
    {
        static uint32_t index = 0;
        return ++index;
    }

    int RpcServiceImpl::getNewMessageId() {
        static int index = 0;
        return ++index;
    }
}