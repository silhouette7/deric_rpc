#include "rpc_client_impl.h"
#include "deric_debug.h"
#include "io_module.h"

namespace deric
{
namespace rpc
{
    RpcClientImpl::~RpcClientImpl() {
        if (m_ioConnection) {
            m_ioConnection->unsetIoDataCallback();
            m_ioConnection->unsetConnectCallback();
            m_ioConnection->unsetCloseCallback();
        }
    }

    int RpcClientImpl::connect(std::string_view ip, std::string_view port) {
        std::shared_ptr<TcpIoConnection> io = std::make_shared<TcpIoConnection>();

        io->setIoDataCallback([this](const std::shared_ptr<TcpIoConnection>& connect, const std::string& data){handleData(connect, data);});
        io->setConnectCallback([this](const std::shared_ptr<TcpIoConnection>& connect){handleConnect(connect);});
        io->setCloseCallback([this](const std::shared_ptr<TcpIoConnection>& connect){handleClose(connect);});

        if (0 > io->connect(ip, std::stoi(std::string(port)))) {
            DEBUG_ERROR("unable to start the io connection to ip: %s, port: %s", ip.data(), port.data());
            io.reset();
            return -1;
        }

        IoModule::getInstance().addIoMember(io);
        m_ioConnection = std::move(io);

        return 0;
    }

    int RpcClientImpl::disconnect() {
        if (!m_ioConnection) {
            DEBUG_INFO("the connection was release already");
            return 0;
        }

        if (0 > m_ioConnection->shutdown()) {
            DEBUG_ERROR("unable to stop the io");
            return -1;
        }
        return 0;
    }

    int RpcClientImpl::sendMsg(std::string_view msg) {
        if (!m_ioConnection) {
            DEBUG_ERROR("no io connection");
            return -1;
        }

        return m_ioConnection->sendData(msg);
    }

    int RpcClientImpl::registerMessageCallback(const ClientMessageCallbackType& func) {
        m_msgCallback = func;
        return 0;
    }

    int RpcClientImpl::registerMessageCallback(ClientMessageCallbackType&& func) {
        m_msgCallback = std::move(func);
        return 0;
    }

    void RpcClientImpl::handleConnect(const std::shared_ptr<TcpIoConnection>& connect) {
        if (!connect) {
            return;
        }

        if (connect->isConnected()) {
            DEBUG_INFO("io connect");
        }
        else {
            DEBUG_INFO("io disconnect");
        }
    }

    void RpcClientImpl::handleClose(const std::shared_ptr<TcpIoConnection>& connect) {
        (void)connect;
        if (m_ioConnection) {
            DEBUG_INFO("io close");
            m_ioConnection->destory();
            m_ioConnection.reset();
        }
    }

    void RpcClientImpl::handleData(const std::shared_ptr<TcpIoConnection>& connect, const std::string& data) {
        (void)connect;

        if (m_msgCallback) {
            m_msgCallback(data);
        }
        else {
            DEBUG_ERROR("there is not msg handler");
        }
    }
}
}