#include "rpc_client_entry.h"
#include "rpc_client_impl.h"
#include "rpc_config.h"
#include "rpc_messages.h"
#include "deric_debug.h"

namespace deric
{
namespace rpc
{
    int RpcClientEntry::init(const ConfigInterface& config) {
        if (0 > config.getValue("ServiceBook", m_serviceBook)) {
            DEBUG_ERROR("undefined service book");
            return -1;
        }

        m_clientImpl = std::make_unique<RpcClientImpl>();
        auto callback = [this](const std::string& str){serviceMessageCallback(str);};
        if (0 > m_clientImpl->registerMessageCallback(std::move(callback))) {
            DEBUG_ERROR("unable to register message callback");
            m_clientImpl.reset();
            return -1;
        }
    
        return 0;
    }

    int RpcClientEntry::deinit() {
        if (m_clientImpl) {
            m_clientImpl.reset();
        }

        m_serviceBook.clear();
        m_clientImpl.reset();

        {
            std::lock_guard<std::mutex> g(m_replayLock);
            m_replyCallbacks.clear();
        }

        {
            std::lock_guard<std::mutex> g(m_eventLock);
            m_eventCallback.clear();
        }
        return 0;
    }

    int RpcClientEntry::connect(const std::string& serviceName) {
        if (!m_clientImpl) {
            DEBUG_ERROR("no init yet");
            return -1;
        }

        RpcConfig serviceBookConfig;
        if (0 > serviceBookConfig.init(m_serviceBook)) {
            DEBUG_ERROR("unable to open serivce book: %s", m_serviceBook.c_str());
            return -1;
        }

        std::string serviceIp, servicePort;
        if (0 > serviceBookConfig.getValue(serviceName + "_IP", serviceIp)) {
            DEBUG_ERROR("unable to get ip of service: %s", serviceName.c_str());
            return -1;
        }

        if (0 > serviceBookConfig.getValue(serviceName + "_Port", servicePort)) {
            DEBUG_ERROR("unable to get port of service: %s", serviceName.c_str());
            return -1;
        }

        DEBUG_INFO("connecting to serivce - %s, with ip - %s port - %s", serviceName.c_str(), serviceIp.c_str(), servicePort.c_str());

        if (0 > m_clientImpl->connect(serviceIp, servicePort)) {
            DEBUG_ERROR("unable to connect to service: %s", serviceName.c_str());
            return -1;
        }

        DEBUG_INFO("connect to service: %s successfully", serviceName.c_str());

        return 0;
    }

    int RpcClientEntry::disconnect() {
        if (!m_clientImpl) {
            DEBUG_ERROR("no init yet");
            return -1;
        }

        return m_clientImpl->disconnect();
    }

    void RpcClientEntry::subscribeToEvent(const std::string& eventName, const RpcClientEntryEventCallbackType& callback) {
        std::lock_guard<std::mutex> g(m_eventLock);
        m_eventCallback[eventName] = callback;
        _subscribe(eventName, true);
    }

    void RpcClientEntry::subscribeToEvent(const std::string& eventName, RpcClientEntryEventCallbackType&& callback) {
        std::lock_guard<std::mutex> g(m_eventLock);
        m_eventCallback[eventName] = std::move(callback);
        _subscribe(eventName, true);
    }

    void RpcClientEntry::unsubscribeToEvent(const std::string& eventName) {
        std::lock_guard<std::mutex> g(m_eventLock);
        m_eventCallback.erase(eventName);
        _subscribe(eventName, false);
    }

    void RpcClientEntry::serviceMessageCallback(const std::string& msg) {
        std::optional<serialer::SerialerStr> serialStr = serialer::getSerialStrFromRpcStr(msg);
        if (!serialStr) {
            DEBUG_ERROR("invalid params");
            return;
        }
        auto [metaStr, payloadStr] = serialStr.value();

        std::optional<serialer::MessageMeta> meta = serialer::getMessageData<serialer::MessageMeta>(metaStr);
        if (!meta) {
            DEBUG_ERROR("invalid params");
            return; 
        }
        auto [msgType, msgName, msgId, result] = meta.value();

        DEBUG_INFO("receive msg type: %d, msg name: %s, msg id: %d, result: %d", static_cast<int>(msgType), msgName.c_str(), msgId, result);

        switch (msgType)
        {
            case MessageType::REPLAY:
            {
                InvokeReplyCallbakType replayCallback;
                {
                    std::lock_guard<std::mutex> g(m_replayLock);
                    if (m_replyCallbacks.find(msgId) != m_replyCallbacks.end()) {                    
                        replayCallback = std::move(m_replyCallbacks[msgId]);
                        m_replyCallbacks.erase(msgId);
                    }
                }
                if (result == true && replayCallback) {
                    replayCallback(payloadStr);
                }
                else {
                    DEBUG_ERROR("unhandled msg: type - %d, name - %s, id - %d", static_cast<int>(msgType), msgName.c_str(), msgId);
                }
                break;
            }
            case MessageType::NOTIFY:
            {
                RpcClientEntryEventCallbackType eventCallback;
                {
                    std::lock_guard<std::mutex> g(m_eventLock);
                    auto it = m_eventCallback.find(msgName);
                    if (it != m_eventCallback.end()) {
                        eventCallback = it->second;
                    }
                }
                if (eventCallback) {
                    eventCallback();
                }
                else {
                    DEBUG_ERROR("unhandled event name - %s, id - %d", msgName.c_str(), msgId);
                }
                break;
            }
            default:
            {
                DEBUG_ERROR("receive unknown msg type: %d", static_cast<int>(msgType));
                break;
            }
        }
    }

    int RpcClientEntry::_subscribe(const std::string& name, bool subscribe) {
        if (!m_clientImpl) {
            DEBUG_ERROR("no init yet");
            return -1;
        }

        serialer::MessageMeta cmdMeta = {subscribe ? MessageType::REGISTER : MessageType::UNREGISTER, name, 0, true};
        std::string rpcMeta = serialer::serialMessageData(std::move(cmdMeta));
        std::string rpcStr = serialer::generateRpcStr(rpcMeta, "");
        return m_clientImpl->sendMsg(rpcStr);
    }

    int RpcClientEntry::_invoke(const std::string& invokeStr) {
        if (!m_clientImpl) {
            DEBUG_ERROR("no init yet");
            return -1;
        }

        return m_clientImpl->sendMsg(invokeStr);
    }

    int RpcClientEntry::_getMessageId() {
        static int index = 0;
        return ++index;
    }
}
}