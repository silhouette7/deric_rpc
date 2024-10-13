#include "rpc_service_client_entry.h"

#include "deric_debug.h"
#include "io_module.h"
#include "rpc_messages.h"
#include "rpc_serialer.h"
#include "rpc_service_impl.h"
#include "thread_pool.h"

#include <optional>

namespace deric::rpc
{
    RpcServiceClientEntry::RpcServiceClientEntry(uint32_t clientId,
                                                 RpcServiceImpl& serviceImpl,
                                                 const std::shared_ptr<TcpIoConnection>& connect) :
        m_clientId(clientId),
        m_serviceImpl(serviceImpl),
        m_ioConnection(connect)
    {
        DEBUG_INFO("construct");
    }

    RpcServiceClientEntry::~RpcServiceClientEntry() {
        DEBUG_INFO("deconstruct");
        if (m_ioConnection) {
            m_ioConnection->unsetIoDataCallback();
            m_ioConnection->unsetConnectCallback();
        }
    }

    int RpcServiceClientEntry::start() {
        if (!m_ioConnection) {
            DEBUG_ERROR("unable to start since there is no connection");
            return -1;
        }

        std::weak_ptr<RpcServiceClientEntry> wp = shared_from_this();
        m_ioConnection->setConnectCallback([wp](const std::shared_ptr<TcpIoConnection>& conn){
                                           auto sp = wp.lock(); if (sp) sp->handleConnect(conn);});
        m_ioConnection->setIoDataCallback([wp](const std::shared_ptr<TcpIoConnection>& conn, const std::string& data){
                                           auto sp = wp.lock(); if (sp) sp->handleData(conn, data);});

        if (0 > m_ioConnection->connect()) {
            DEBUG_ERROR("unable to connect the io connection");
            return -1;
        }

        IoModule::getInstance().addIoMember(m_ioConnection);
        return 0;
    }

    int RpcServiceClientEntry::stop() {
        if (!m_ioConnection) {
            DEBUG_ERROR("no io connection instance");
            return -1;
        }

        if (m_ioConnection->isConnected()) {
            if (0 > m_ioConnection->shutdown()) {
                DEBUG_ERROR("unable to shutdown the io connection");
                return -1;
            }
        }
        return 0;
    }

    uint32_t RpcServiceClientEntry::getClientId() {
        return m_clientId;
    }

    void RpcServiceClientEntry::handleData(const std::shared_ptr<TcpIoConnection>& connect, const std::string& data) {
        (void)connect;

        std::optional<serialer::SerialerStr> serialStr = serialer::getSerialStrFromRpcStr(data);
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

        DEBUG_INFO("receive message: type - %d, msgId - %d, msgName: %s", static_cast<int>(msgType), msgId, msgName.c_str());
        switch(msgType)
        {
            case MessageType::COMMAND:
            {
                bool isExec = false;
                std::optional<std::string> res;
                try {
                    res = m_serviceImpl.handleCommand(msgName, payloadStr);
                    isExec = true;
                }
                catch(...) {
                    DEBUG_ERROR("handle command error");
                }
                serialer::MessageMeta replyMeta = {MessageType::REPLAY, msgName, msgId, isExec};
                std::string replayMetaStr = serialer::serialMessageData(std::move(replyMeta));
                std::string rpcStr = serialer::generateRpcStr(replayMetaStr, res ? res.value() : "");
                if (0 > sendData(rpcStr)) {
                    DEBUG_ERROR("unable to return result");
                }
                break;
            }
            case MessageType::REGISTER:
            {
                m_serviceImpl.clientRegisterToEvent(msgName, m_clientId);
                break;
            }
            case MessageType::UNREGISTER:
            {
                m_serviceImpl.clientUnregisterToEvent(msgName, m_clientId);
                break;
            }
            default:
                DEBUG_ERROR("unhandle message type: %d", static_cast<int>(msgType));
                break;
        }
    }

    void RpcServiceClientEntry::handleConnect(const std::shared_ptr<TcpIoConnection>& connect) {
        if (!connect) {
            return;
        }

        if (connect->isConnected()) {
            DEBUG_INFO("io connect");
        }
        else {
            m_serviceImpl.handleClientClose(m_clientId);
        }
    }

    int RpcServiceClientEntry::sendData(const std::string& data) {
        if (!m_ioConnection) {
            DEBUG_ERROR("no io connection instance");
            return -1;
        }
        return m_ioConnection->sendData(data);
    }
}