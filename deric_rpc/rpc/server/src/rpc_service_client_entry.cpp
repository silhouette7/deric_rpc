#include "deric_debug.h"
#include "rpc_service_client_entry.h"
#include "io_module.h"

namespace deric
{
namespace rpc
{
    RpcServiceClientEntry::RpcServiceClientEntry(uint32_t clientId, std::weak_ptr<RpcServiceImpl> serviceImpl) :
        m_clientId(clientId),
        m_serviceImpl(serviceImpl),
        m_ioConnection()
    {
        DEBUG_INFO("construct");
    }

    RpcServiceClientEntry::~RpcServiceClientEntry() {
        DEBUG_INFO("deconstruct");
    }

    int RpcServiceClientEntry::handleEvent(IoConnectionEvent_e event, void* data) {
        int res = 0;
        DEBUG_INFO("receive event: %d", event);
        switch(event)
        {
            case TCP_IO_CONNECTION_EVENT_CLOSE:
            {
                if (0 > stop()) {
                    DEBUG_ERROR("unable to stop");
                }

                auto spServiceImpl = m_serviceImpl.lock();
                if (spServiceImpl) {
                    spServiceImpl->removeIoConnectionClient(m_clientId);
                }
                else {
                    DEBUG_ERROR("no service impl instance");
                    res = -1;
                }
            }    
        }
        return res;
    }

    int RpcServiceClientEntry::handleData(const char *pData, int len) {
        int res = -1;
        std::string resultString;
        int resultLen = 0;
        auto spServiceImpl = m_serviceImpl.lock();

        if (!spServiceImpl) {
            DEBUG_ERROR("no service instance");
            return res;
        }

        res = spServiceImpl->handleData(pData, len, resultString);

        resultLen = resultString.size();
        if (resultLen) {
            res = sendData(resultString);
            if (0 > res) {
                DEBUG_ERROR("unable to send data, length: %d, res: %d", resultLen, res);
            }
        }

        return res;
    }

    void RpcServiceClientEntry::setIoConnection(std::shared_ptr<IoMember> ioConnection) {
        m_ioConnection = std::dynamic_pointer_cast<TcpIoConnection>(ioConnection);
        if (!m_ioConnection) {
            DEBUG_ERROR("invalid params");
        }
    }

    int RpcServiceClientEntry::startIoClient() {
        return start();
    }

    int RpcServiceClientEntry::stopIoClient() {
        return stop();
    }
 
    int RpcServiceClientEntry::startClientEntry() {
        return start();
    }

    int RpcServiceClientEntry::stopClientEntry() {
        return stop();
    }

    uint32_t RpcServiceClientEntry::getClientId() {
        return m_clientId;
    }

    int RpcServiceClientEntry::sendData(const std::string& data) {
        if (!m_ioConnection) {
            DEBUG_ERROR("no io connection instance");
            return -1;
        }

        return m_ioConnection->sendData(data.c_str(), data.length());
    }

    int RpcServiceClientEntry::start() {
        if (!m_ioConnection) {
            DEBUG_ERROR("unable to start since there is no connection");
            return -1;
        }

        if (!m_serviceImpl.lock()) {
            DEBUG_ERROR("unable to start since there is no service instance");
            return -1;
        }

        if (0 > m_ioConnection->start()) {
            DEBUG_ERROR("unable to start the io connection");
            return -1;
        }

        IoModule::getInstance().addIoMember(m_ioConnection);
        return 0;
    }

    int RpcServiceClientEntry::stop() {
        if (m_ioConnection) {
            IoModule::getInstance().removeIoMember(m_ioConnection);

            if (0 > m_ioConnection->stop()) {
                DEBUG_ERROR("unable to stop the io connection");
            }
        }

        return 0;
    }

}
}