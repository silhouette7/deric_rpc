#include "tcp_io_connection.h"

#include "deric_debug.h"
#include "io_module.h"
#include "thread_pool.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

namespace deric
{
    TcpIoConnection::TcpIoConnection() :
        m_connectState(ConnectState::DISCONNCTED),
        m_socketFd(-1),
        m_mutex(),
        m_sendBuffer(),
        m_receiveBuffer(),
        m_connectCallback(),
        m_closeCallback(),
        m_dataCallback(),
        m_context(nullptr)
    {
        DEBUG_INFO("construct");
    }

    TcpIoConnection::TcpIoConnection(int socket) : TcpIoConnection() {
        m_socketFd = socket;
    }

    TcpIoConnection::~TcpIoConnection() {
        DEBUG_INFO("deconstruct");
        if (m_socketFd > 0) {
            close(m_socketFd);
        }
    }

    int TcpIoConnection::getFd() {
        return m_socketFd;
    }

    void TcpIoConnection::onReadAvailable() {
        std::lock_guard<std::mutex> g(m_mutex);
        if (m_socketFd < 0 || m_connectState != ConnectState::CONNECTED) {
            DEBUG_ERROR("invalid state");
            return;
        }

        m_receiveBuffer.resize(TCP_HEADER_SIZE);
        TcpHeader_s tcpHeader;
        ssize_t recvBytes = recv(m_socketFd, m_receiveBuffer.data(), TCP_HEADER_SIZE, MSG_WAITALL);
        if (recvBytes < 0)
        {
            DEBUG_ERROR("recv data fail, res: %ld", recvBytes);
        }
        else if (recvBytes == 0)
        {
            DEBUG_INFO("the connection is being closed");
            handleClose();
        }
        else
        {
            tcpHeader.deserialerFrom(m_receiveBuffer.data(), recvBytes);
            DEBUG_INFO("will receive data size: %u", tcpHeader.messageLen);
            m_receiveBuffer.clear();
            m_receiveBuffer.resize(tcpHeader.messageLen);
            recvBytes = recv(m_socketFd, m_receiveBuffer.data(), tcpHeader.messageLen, MSG_WAITALL);
            if (recvBytes < 0) {
                DEBUG_ERROR("recv data fail, res: %ld", recvBytes);
            }
            else if (recvBytes == 0) {
                DEBUG_INFO("the connection is being closed");
                handleClose();
            }
            else {
                DEBUG_INFO("recv data size: %ld", recvBytes);
                if (m_dataCallback) {
                    std::string receiveData;
                    receiveData.swap(m_receiveBuffer);
                    ThreadPool::getInstance().commit(m_dataCallback, shared_from_this(), std::move(receiveData));
                }
            }
        }
        m_receiveBuffer.clear();
    }

    void TcpIoConnection::onWriteAvailable() {
        //Not implement
    }

    void TcpIoConnection::onControlAvailable() {
        //Not implement
    }

    void TcpIoConnection::onIoError(IoMemberError_e error) {
        switch(error)
        {
            case IO_MEMBER_ERROR_POLL_FAILED:
            {
                DEBUG_INFO("encounter epoll error, maybe the connection is being closed");
                std::lock_guard<std::mutex> g(m_mutex);
                handleClose();
                break;
            }
            default:
            {
                DEBUG_ERROR("receive known error: %d", error);
                break;
            }
        }
    }

    int TcpIoConnection::sendData(std::string_view data) {
        if (data.empty()) {
            DEBUG_ERROR("invalid params");
            return -1;
        }

        std::lock_guard<std::mutex> g(m_mutex);
        if (m_socketFd < 0 || m_connectState != ConnectState::CONNECTED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }
    
        size_t len = data.length();
        m_sendBuffer.resize(TCP_HEADER_SIZE + len);
        TcpHeader_s tcpHeader;
        tcpHeader.messageLen = data.size();
        tcpHeader.serialerTo(m_sendBuffer.data(), TCP_HEADER_SIZE);
        memcpy(m_sendBuffer.data() + TCP_HEADER_SIZE, data.data(), len);
        DEBUG_INFO("try to send data of size: %lu to socket: %d", len, m_socketFd);
        int res = send(m_socketFd, m_sendBuffer.data(), TCP_HEADER_SIZE + len, 0);
        if (0 > res) {
            DEBUG_ERROR("unable to send data to socket: %d", m_socketFd);
        }

        DEBUG_INFO("send data of size: %lu to socket: %d successfully", len, m_socketFd);
        return res;
    }

    int TcpIoConnection::connect() {
        std::lock_guard<std::mutex> g(m_mutex);
        if (m_socketFd < 0) {
            DEBUG_ERROR("invalid socket id");
            return -1;
        }
    
        m_connectState = ConnectState::CONNECTED;
        ThreadPool::getInstance().commit([sp = shared_from_this()](){sp->onConnected();});
        return 0;
    }

    int TcpIoConnection::connect(std::string_view ip, int port) {
        std::lock_guard<std::mutex> g(m_mutex);
        if (m_connectState != ConnectState::DISCONNCTED) {
            DEBUG_ERROR("already connect");
            return -1;
        }

        if (ip.length() < 0 || port < 0) {
            DEBUG_ERROR("invalid params");
            return -1;
        }

        m_socketFd = socket(PF_INET, SOCK_STREAM, 0);
        if (0 > m_socketFd) {
            DEBUG_ERROR("unable to create socket");
            return -1;
        }

        struct sockaddr_in serverAdd;
        memset(&serverAdd, 0, sizeof(serverAdd));
        serverAdd.sin_family = AF_INET;
        serverAdd.sin_port = htons(port);
        serverAdd.sin_addr.s_addr = inet_addr(ip.data());
        if (0 > ::connect(m_socketFd, reinterpret_cast<struct sockaddr *>(&serverAdd), sizeof(serverAdd))) {
            DEBUG_ERROR("unable to connect to server ip: %s, port: %d", ip.data(), port);
            close(m_socketFd);
            m_socketFd = -1;
            return -1;
        }
        
        m_connectState = ConnectState::CONNECTED;
        DEBUG_INFO("start and connect to service ip: %s, port: %d successfully", ip.data(), port);
        ThreadPool::getInstance().commit([sp = shared_from_this()](){sp->onConnected();});
        return 0;
    }

    int TcpIoConnection::shutdown() {
        std::lock_guard<std::mutex> g(m_mutex);
        if (m_socketFd && m_connectState == ConnectState::CONNECTED) {
            if (0 > ::shutdown(m_socketFd, SHUT_WR)) {
                DEBUG_ERROR("shutdown fail");
                return -1;
            }
            m_connectState = ConnectState::DISCONNCTED;
            ThreadPool::getInstance().commit([sp = shared_from_this()](){sp->onShutdown();});
        }
        return 0;
    }

    int TcpIoConnection::destory() {
        m_connectState = ConnectState::DISCONNCTED;
        ThreadPool::getInstance().commit([spThis = shared_from_this()](){spThis->onDestoryed();});
        return 0;
    }

    bool TcpIoConnection::isConnected() {
        std::lock_guard<std::mutex> g(m_mutex);
        return m_connectState == ConnectState::CONNECTED;
    }

    void TcpIoConnection::setConnectCallback(const TcpIoConnectCallbackType& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_connectCallback = callback;
    }

    void TcpIoConnection::setConnectCallback(TcpIoConnectCallbackType&& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_connectCallback = std::move(callback);
    }

    void TcpIoConnection::unsetConnectCallback() {
        std::lock_guard<std::mutex> g(m_mutex);
        auto empty = (decltype(m_connectCallback)){};
        m_connectCallback.swap(empty);
    }

    void TcpIoConnection::setCloseCallback(const TcpIoCloseCallbackType& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_closeCallback = callback;
    }

    void TcpIoConnection::setCloseCallback(TcpIoCloseCallbackType&& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_closeCallback = std::move(callback);
    }

    void TcpIoConnection::unsetCloseCallback() {
        std::lock_guard<std::mutex> g(m_mutex);
        auto empty = (decltype(m_closeCallback)){};
        m_closeCallback.swap(empty);
    }

    void TcpIoConnection::setIoDataCallback(const TcpIoDataCallbackType& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_dataCallback = callback;
    }

    void TcpIoConnection::setIoDataCallback(TcpIoDataCallbackType&& callback) {
        std::lock_guard<std::mutex> g(m_mutex);
        m_dataCallback = std::move(callback);
    }

    void TcpIoConnection::unsetIoDataCallback() {
        std::lock_guard<std::mutex> g(m_mutex);
        auto empty = (decltype(m_dataCallback)){};
        m_dataCallback.swap(empty);
    }

    void TcpIoConnection::onConnected() {
        if (m_connectCallback) {
            m_connectCallback(shared_from_this());
        }
    }

    void TcpIoConnection::onShutdown() {
        if (m_connectCallback) {
            m_connectCallback(shared_from_this());
        }
    }

    void TcpIoConnection::onClosed() {
        if (m_connectCallback) {
            m_connectCallback(shared_from_this());
        }

        if (m_closeCallback) {
            m_closeCallback(shared_from_this());
        }
        else {
            destory();
        }
    }

    void TcpIoConnection::onDestoryed() {
        DEBUG_INFO("call");
    }

    void TcpIoConnection::handleClose() {
        m_connectState = ConnectState::DISCONNCTED;
        IoModule::getInstance().removeIoMember(shared_from_this());
        ThreadPool::getInstance().commit([sp = shared_from_this()](){sp->onClosed();});
    }

    void TcpIoConnection::TcpHeader_s::deserialerFrom(const char* data, int len) {
        if (!data || len < TCP_HEADER_SIZE) {
            DEBUG_ERROR("invalid params");
            return;
        }

        magic = TCP_HEADER_MAGIC;
        messageLen = GET_UINT32_FROM_CHARS((uint8_t*)(data) + TCP_HEADER_OFFSET_MESSIGE_LEN);
    }

    void TcpIoConnection::TcpHeader_s::serialerTo(char* buffer, int size) {
        if (!buffer || size < TCP_HEADER_SIZE) {
            DEBUG_ERROR("invalid params");
            return;
        }

        buffer[TCP_HEADER_OFFSET_MAGIC] = TCP_HEADER_MAGIC;
        SET_UINT32_TO_CHARS(messageLen, (uint8_t*)buffer + TCP_HEADER_OFFSET_MESSIGE_LEN);
    }
}