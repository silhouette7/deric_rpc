#include "rpc_client_entry.h"
#include "rpc_client_impl.h"
#include "rpc_config.h"
#include "message_public.h"
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

        std::string s_bufferSize;
        if (0 > config.getValue("BufferSize", s_bufferSize)) {
            DEBUG_ERROR("unable to get buffersize, using the default value");
            m_bufferSize = DEFAULT_RPC_CLIENT_BUFFER_SIZE;
        }
        else {
            m_bufferSize = std::stoi(s_bufferSize);
        }

        m_clientImpl = std::make_shared<RpcClientImpl>();
        std::weak_ptr<RpcClientEntry> wp = shared_from_this();
        auto callback = [wp](const std::string& str)->void{auto sp = wp.lock();
                                                           if(sp) sp->serviceMessageCallback(str);};
        if (0 > m_clientImpl->registerMessageCallback(std::move(callback))) {
            DEBUG_ERROR("unable to register message callback");
            m_clientImpl.reset();
            return -1;
        }
    
        m_serialer = std::make_unique<RpcSerialer>();

        return 0;
    }

    int RpcClientEntry::deinit() {
        if (m_serialer) {
            m_serialer.reset();
        }

        if (m_clientImpl) {
            m_clientImpl.reset();
        }

        m_serviceBook.clear();

        std::lock_guard<std::mutex> lg(m_waitingLock);
        m_waitingItems.clear();
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

        ClientConnectionConfig_s config;
        config.ioBufferSize = m_bufferSize;
        config.serviceIp = std::move(serviceIp);
        config.servicePort = std::move(servicePort);

        if (0 > m_clientImpl->connect(config)) {
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

    void RpcClientEntry::serviceMessageCallback(const std::string& msg) {
        if (!m_serialer) {
            DEBUG_ERROR("not init yet");
            return;
        }

        int msgType = m_serialer->getMessageType(msg.data(), msg.length());
        DEBUG_INFO("receive msg len: %lu, type: %d", msg.length(), msgType);

        switch (msgType)
        {
            case MESSAGE_TYPE_REPLAY:
            {
                int msgId = m_serialer->getMessageId(msg.data(), msg.length());
                std::lock_guard<std::mutex> lg(m_waitingLock);
                DEBUG_INFO("receive msgId: %d", msgId);
                if (m_waitingItems.find(msgId) == m_waitingItems.end()) {
                    DEBUG_ERROR("not found msgId: %d", msgId);
                    break;;
                }
                auto& item = m_waitingItems[msgId];
                item.msg = msg;
                item.cv.notify_one();
                break;
            }
            default:
            {
                DEBUG_ERROR("receive unknown msg type: %d", msgType);
                break;
            }
        }
    }

    int RpcClientEntry::_invoke(const std::string& invokeStr) {
        if (!m_clientImpl) {
            DEBUG_ERROR("no init yet");
            return -1;
        }

        return m_clientImpl->sendMsg(invokeStr);
    }

    int RpcClientEntry::_waitForResult(int msgId, std::string& resultStr) {
        if (0 > _addResultWaitingItem(msgId)) {
            DEBUG_ERROR("unable add waiting item");
            return -1;
        }

        std::unique_lock<std::mutex> ul(m_waitingLock);
        auto& item = m_waitingItems[msgId];
        item.cv.wait(ul);
        resultStr = std::move(item.msg);

        ul.unlock();
        if (0 > _removeResultWaitingItem(msgId)) {
            DEBUG_ERROR("unable to remove waiting item");
        }

        return 0;
    }

    int RpcClientEntry::_addResultWaitingItem(int msgId) {
        std::lock_guard<std::mutex> lg(m_waitingLock);
        if (m_waitingItems.find(msgId) != m_waitingItems.end()) {
            DEBUG_ERROR("already add item for msgId: %d yet", msgId);
            return -1;
        }

        m_waitingItems[msgId];

        return 0;
    }

    int RpcClientEntry::_removeResultWaitingItem(int msgId) {
        std::lock_guard<std::mutex> lg(m_waitingLock);
        if (m_waitingItems.find(msgId) == m_waitingItems.end()) {
            DEBUG_ERROR("not add item for msgId: %d yet", msgId);
            return -1;
        }

        m_waitingItems.erase(msgId);
        return 0;
    }

    int RpcClientEntry::_getMessageId() {
        static int index = 0;
        return ++index;
    }
}
}