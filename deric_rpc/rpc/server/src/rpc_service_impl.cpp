#include "deric_debug.h"
#include "message_public.h"
#include "io_module.h"
#include "rpc_service_client_entry.h"
#include "rpc_service_impl.h"

namespace deric
{
namespace rpc
{
    RpcServiceImpl::RpcServiceImpl(const std::string& serviceName) :
        m_serviceName(serviceName),
        m_state(COMPONENT_STATE_CREATED),
        m_serialer(),
        m_ioServer(),
        m_clients(),
        m_functions(),
        m_instanceMutex(),
        m_clientsMutex(),
        m_functionsMutex()
    {
        DEBUG_INFO("construct");
    }

    RpcServiceImpl::~RpcServiceImpl() {
        DEBUG_INFO("deconstruct");
    }

    int RpcServiceImpl::init(const ServiceConfig_s &serviceConfig) {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_instanceMutex);

        do {
            if (m_state != COMPONENT_STATE_CREATED) {
                DEBUG_ERROR("invalid state: %d", m_state);
                res = -1;
                break;
            }

            m_serialer = serviceConfig.serialer;

            m_ioServer = std::make_shared<TcpIoServer>();
            if (!m_ioServer) {
                DEBUG_ERROR("create tcp io server fail");
                res = -1;
                break;
            }

            TcpIoServerConfig_s config;
            config.ip = serviceConfig.serviceIp;
            config.port = serviceConfig.servicePort;
            config.maxConnectionNumber = serviceConfig.maxConnectionNumber;
            config.ioBufferSize = serviceConfig.ioBufferSize;
            config.serverClient = shared_from_this();
            res = m_ioServer->init(config);
            if(res < 0) {
                DEBUG_ERROR("init tcp io server fail");
                m_ioServer = nullptr;
                res = -1;
                break;
            }
        } while(0);

        if (res >= 0) {
            m_state = COMPONENT_STATE_INITIALIZED;
        }

        return res;
    }

    int RpcServiceImpl::deInit() {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_instanceMutex);

        do {
            if (m_state != COMPONENT_STATE_INITIALIZED) {
                DEBUG_ERROR("invalid state: %d", m_state);
                res = -1;
                break;
            }

            if (m_ioServer) {
                res = m_ioServer->deInit();
                if(res < 0) {
                    DEBUG_ERROR("deinit tcp io server fail");
                    res = -1;
                    break;
                }
                m_ioServer.reset();
            }

            if (m_serialer) {
                m_serialer.reset();
            }
        } while(0);

        if (res >= 0) {
            m_state = COMPONENT_STATE_CREATED;
        }

        return res;
    }

    int RpcServiceImpl::start() {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_instanceMutex);

        do {
            if (m_state != COMPONENT_STATE_INITIALIZED) {
                DEBUG_INFO("invalid state: %d", m_state);
                res = -1;
                break;
            }

            if (!m_ioServer) {
                DEBUG_ERROR("no tcp io server");
                res = -1;
                break;
            }

            res = m_ioServer->start();
            if (0 > res) {
                DEBUG_ERROR("start io server fail");
                break;
            }
        } while(0);

        if (res >= 0) {
            m_state = COMPONENT_STATE_STARTED;
        }

        return res;
    }

    int RpcServiceImpl::stop() {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_instanceMutex);

        do {
            if (m_state != COMPONENT_STATE_STARTED) {
                DEBUG_INFO("invalid state: %d", m_state);
                res = -1;
                break;
            }

            if (!m_ioServer) {
                DEBUG_ERROR("no io server");
                res = 0;
                break;
            }

            {
                std::lock_guard<std::mutex> g(m_clientsMutex);
                for (auto& client : m_clients) {
                    if (0 > client.second->stopClientEntry()) {
                        DEBUG_ERROR("enable to stop client: %u", client.first);
                    }
                }
                m_clients.clear();
            }

            res = m_ioServer->stop();
            if (0 > res) {
                DEBUG_ERROR("stop io server fail");
                break;
            }
        } while(0);

        m_state = COMPONENT_STATE_INITIALIZED;

        return res;
    }

    int RpcServiceImpl::registerMethod(const std::string& funcName,
                                       const ServiceInterface::ServiceFunctionType& func)
    {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_functionsMutex);

        if (m_functions.find(funcName) != m_functions.end()) {
            DEBUG_ERROR("func: %s exit already", funcName.c_str());
        }
        else {
            DEBUG_INFO("func: %s is registered", funcName.c_str());
            m_functions[funcName] = func;
            res = 0;
        }

        return res;
    }

    int RpcServiceImpl::unregisterMethod(const std::string& funcName) {
        int res = -1;
        std::lock_guard<std::mutex> guard(m_functionsMutex);

        if (m_functions.find(funcName) == m_functions.end()) {
            DEBUG_ERROR("func: %s not already", funcName.c_str());
        }
        else {
            DEBUG_INFO("func: %s is removed", funcName.c_str());
            m_functions.erase(funcName);
            res = 0;
        }
        return res;
    }

    int RpcServiceImpl::postEvent(const std::string& eventName, const char* pData, int len) {
        // Not implementation
        return 0;
    }

    void RpcServiceImpl::createIoConnectionClient(std::shared_ptr<IoConnectionClientInterface>& connectionClient) {
        std::lock_guard<std::mutex> g(m_clientsMutex);
        uint32_t clientId = getNewClientId();
        auto spClient = std::make_shared<RpcServiceClientEntry>(clientId, shared_from_this());
        m_clients[clientId] = spClient;
        connectionClient = spClient;
        DEBUG_INFO("add client of id: %u", clientId);
    }

    void RpcServiceImpl::removeIoConnectionClient(std::shared_ptr<IoConnectionClientInterface> connectionClient) {
        std::shared_ptr<RpcServiceClientEntry> spClient = std::dynamic_pointer_cast<RpcServiceClientEntry>(connectionClient);
        if (!spClient) {
            DEBUG_ERROR("invalid param");
            return;
        }
        removeIoConnectionClient(spClient->getClientId());
    }

    void RpcServiceImpl::removeIoConnectionClient(uint32_t clientId) {
        std::lock_guard<std::mutex> g(m_clientsMutex);
        if (m_clients.find(clientId) == m_clients.end()) {
            DEBUG_ERROR("cannot found client of id: %u", clientId);
        }

        m_clients.erase(clientId);

        DEBUG_INFO("remove client of id: %u", clientId);
    }

    int RpcServiceImpl::handleData(const char* pData, int len, std::string& resultString) {
        int res = -1;

        if (!pData) {
            DEBUG_ERROR("invalid param");
            return res;
        }

        std::shared_ptr<RpcSerialer> pSerialer;
        {
            std::lock_guard<std::mutex> guard(m_instanceMutex);
            if (m_state != COMPONENT_STATE_STARTED) {
                DEBUG_ERROR("invalid state");
                return res;
            }

            if (!m_serialer) {
                DEBUG_ERROR("no serialer");
                return res;
            }

            pSerialer = m_serialer;
        }

        MessageType_e msgType = static_cast<MessageType_e>(pSerialer->getMessageType(pData, len));
        int msgId = pSerialer->getMessageId(pData, len);
        std::string msgName = pSerialer->getMessageName(pData, len);

        DEBUG_INFO("receive message: type - %d, msgId - %d, msgName: %s", msgType, msgId, msgName.c_str());
        switch(msgType)
        {
            case MESSAGE_TYPE_COMMAND:
            {
                pSerialer->serialMessageMeta(MESSAGE_TYPE_REPLAY, msgName, msgId, resultString);
                res = handleCommand(msgName, pData, len, resultString);
                break;
            }
            case MESSAGE_TYPE_REGISTER:
            {
                res = handleRegister(msgName, pData, len, resultString);
                break;
            }
            default:
                DEBUG_ERROR("unhandle message type: %d", msgType);
                break;
        }

        if (res < 0) {
            DEBUG_ERROR("handle data fail");
        }
        return res;
    }

    int RpcServiceImpl::handleCommand(const std::string& funcName, const char* pData, int len, std::string& resultString)
    {
        int res = 0;
        std::function<int (const char*, int, std::string&)> func;

        do {
            {
                std::lock_guard<std::mutex> guard(m_functionsMutex);
                if (m_functions.find(funcName) == m_functions.end()) {
                    DEBUG_ERROR("unknown function: %s", funcName.c_str());
                    res = -1;
                    break;
                }
                else {
                    func = m_functions[funcName];
                }
            }

            res = func(pData, len, resultString);
            if (res < 0) {
                DEBUG_ERROR("handle command: %s fail", funcName.c_str());
            }
        } while(0);

        return res;
    }

    int RpcServiceImpl::handleRegister(const std::string& eventName, const char* pData, int len, std::string& resultString)
    {
        //TODO: implementation
        return 0;
    }

    uint32_t RpcServiceImpl::getNewClientId()
    {
        static uint32_t index = 0;
        return ++index;
    }
}
}