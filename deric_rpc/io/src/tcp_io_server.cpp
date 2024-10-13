#include "tcp_io_server.h"

#include "deric_debug.h"
#include "io_module.h"
#include "tcp_io_server_entry.h"
#include "thread_pool.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

namespace deric
{
    TcpIoServer::TcpIoServer(std::string_view ip, int port) :
        m_isStarted(false),
        m_ip(ip),
        m_port(port),
        m_maxConnectionNumber(DEFAULT_MAX_CONNECTION_NUMBER),
        m_ioEntry(),
        m_connectCallback(),
        m_eventCallback(),
        m_connectionsLock(),
        m_connections()
    {
        DEBUG_INFO("construct");
    }

    TcpIoServer::TcpIoServer(std::string_view ip, int port, int maxConnectionNumber) : TcpIoServer(ip, port) {
        m_maxConnectionNumber = maxConnectionNumber;
    }

    TcpIoServer::~TcpIoServer() {
        DEBUG_INFO("deconstruct");
        stop();
    }

    int TcpIoServer::start() {
        if (m_isStarted) {
            DEBUG_ERROR("already started");
            return -1;
        }

        int socketFd = socket(PF_INET, SOCK_STREAM, 0);
        if (0 > socketFd) {
            DEBUG_ERROR("create socket fail");
            return -1;
        }

        struct sockaddr_in serverAdd;
        memset(&serverAdd, 0, sizeof(serverAdd));
        serverAdd.sin_family = AF_INET;
        serverAdd.sin_port = htons(m_port);
        serverAdd.sin_addr.s_addr = inet_addr(m_ip.c_str());
        if (0 > bind(socketFd, reinterpret_cast<struct sockaddr*>(&serverAdd), sizeof(serverAdd))) {
            DEBUG_ERROR("bind server ip %s, port %d failed", m_ip.c_str(), m_port);
            close(socketFd);
            return -1;
        }
        
        if (0 > listen(socketFd, m_maxConnectionNumber)) {
            DEBUG_ERROR("listen failed");
            close(socketFd);
            return -1;
        }
        
        m_ioEntry = std::make_shared<TcpIoServerEntry>(socketFd);
        m_ioEntry->setConnectCallback([this](int acceptSocket){return onConnectRequest(acceptSocket);});
        m_ioEntry->setErrorCallback([this](IoMemberError_e error){return onIoError(error);});
        if (0 > IoModule::getInstance().addIoMember(m_ioEntry)) {
            DEBUG_ERROR("add io member fail");
            m_ioEntry.reset();
            return -1;
        }

        m_isStarted = true;
        DEBUG_INFO("io server start successfullt, ip %s, port %d", m_ip.c_str(), m_port);
        return 0;
    }

    int TcpIoServer::stop() {
        m_isStarted = false;

        {
            std::lock_guard<std::mutex> g(m_connectionsLock);
            for (auto& item : m_connections) {
                item.second->destory();
                IoModule::getInstance().removeIoMember(item.second);
            }
            m_connections.clear();
        }

        if (m_ioEntry) {
            IoModule::getInstance().removeIoMember(m_ioEntry);
            m_ioEntry.reset();
        }

        return 0;
    }

    void TcpIoServer::setNewConnectCallback(const TcpIoServerNewConnectCallbackType& callback) {
        m_connectCallback = callback;
    }

    void TcpIoServer::setNewConnectCallback(TcpIoServerNewConnectCallbackType&& callback) {
        m_connectCallback = std::move(callback);
    }

    void TcpIoServer::setEventCallback(const TcpIoServerEventCallbackType& callback) {
        m_eventCallback = callback;
    }

    void TcpIoServer::setEventCallback(TcpIoServerEventCallbackType&& callback) {
        m_eventCallback = std::move(callback);
    }

    void TcpIoServer::onConnectRequest(int acceptSocket) {
        if (0 > acceptSocket) {
            DEBUG_ERROR("accept socket fail");
            return;
        }

        if (!m_isStarted) {
            DEBUG_ERROR("invalid state");
            close(acceptSocket);
            return;
        }

        std::shared_ptr<TcpIoConnection> spConnection;
        {
            std::lock_guard<std::mutex> g(m_connectionsLock);
            if (m_connections.size() >= m_maxConnectionNumber) {
                DEBUG_ERROR("too many connections");
                close(acceptSocket);
                return;
            }
            spConnection = std::make_shared<TcpIoConnection>(acceptSocket);
            m_connections[acceptSocket] = spConnection;
        }

        spConnection->setCloseCallback([this](const std::shared_ptr<TcpIoConnection>& conn){return onIoClose(conn);});
        if (m_connectCallback) {
            m_connectCallback(spConnection);
        }
        else {
            spConnection->connect();
        }
        DEBUG_INFO("add connection for socket fd: %d successfully", acceptSocket);
    }

    void TcpIoServer::onIoError(IoMemberError_e error) {
        DEBUG_ERROR("encounter io error: %d", error);
        if (m_eventCallback) {
            m_eventCallback(TcpIoServerEvent::SERVER_IO_ERROR);
        }
    }

    void TcpIoServer::onIoClose(const std::shared_ptr<TcpIoConnection>& connect) {
        if (!connect) {
            DEBUG_ERROR("invalid params");
        }
        int socketId = connect->getFd();
        DEBUG_INFO("tcp io: %d close", socketId);
        connect->unsetCloseCallback();
        connect->destory();
        {
            std::lock_guard<std::mutex> g(m_connectionsLock);
            m_connections.erase(socketId);
        }
    }
}