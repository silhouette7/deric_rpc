#include "rpc_client_impl.h"
#include "deric_debug.h"
#include "io_module.h"

namespace deric
{
namespace rpc
{
    int RpcClientImpl::connect(const ClientConnectionConfig_s& _config) {
        std::shared_ptr<TcpIoConnection> io = std::make_shared<TcpIoConnection>(_config.ioBufferSize);

        io->setIoClient(shared_from_this());

        if (0 > io->start(_config.serviceIp, std::stoi(_config.servicePort))) {
            DEBUG_ERROR("unable to start the io connection to ip: %s, port: %s", _config.serviceIp.c_str(), _config.serviceIp.c_str());
            io.reset();
            return -1;
        }

        if (0 > IoModule::getInstance().addIoMember(io)) {
            DEBUG_ERROR("unable to add io member");
            io.reset();
            return -1;
        }

        setIoConnection(io);

        return 0;
    }

    int RpcClientImpl::disconnect() {
        if (!m_ioConnection) {
            DEBUG_INFO("the connection was release already");
            return 0;
        }

        if (0 > m_ioConnection->stop()) {
            DEBUG_ERROR("unable to stop the io");
            return -1;
        }

        m_ioConnection.reset();
        return 0;
    }

    int RpcClientImpl::sendMsg(const std::string& msg) {
        if (!m_ioConnection) {
            DEBUG_ERROR("no io connection");
            return -1;
        }

        return m_ioConnection->sendData(msg.data(), msg.length());
    }

    int RpcClientImpl::registerMessageCallback(const ClientMessageCallbackType& func) {
        m_msgCallback = func;
        return 0;
    }

    int RpcClientImpl::handleEvent(IoConnectionEvent_e event, void* data) {
        switch(event)
        {
            case TCP_IO_CONNECTION_EVENT_CLOSE:
            {
                m_ioConnection.reset();
                break;
            }
            default:
            {
                DEBUG_ERROR("receive unhandled io connection event: %d", event);
                break;
            }
        }

        return 0;
    }

    int RpcClientImpl::handleData(const char *pData, int len) {
        if (!pData || len <= 0) {
            DEBUG_ERROR("invalid params");
            return -1;
        }

        if (m_msgCallback) {
            m_msgCallback(std::string(pData, len));
            return 0;
        }
        else {
            DEBUG_ERROR("there is not msg handler");
        }

        return 0;
    }

    void RpcClientImpl::setIoConnection(std::shared_ptr<IoMember> ioConnection) {
        m_ioConnection = std::dynamic_pointer_cast<TcpIoConnection>(ioConnection);
    }

    int RpcClientImpl::startIoClient() {
        // No implementation
        return 0;
    }

    int RpcClientImpl::stopIoClient() {
        // No implememtation
        return 0;
    }
}
}