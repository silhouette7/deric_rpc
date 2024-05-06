#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "deric_debug.h"
#include "message_public.h"
#include "tcp_io_connection.h"

namespace deric
{
namespace rpc
{
    TcpIoConnection::TcpIoConnection(int bufferSize) :
        m_state(COMPONENT_STATE_CREATED),
        m_bufferSize(bufferSize),
        m_recvBuffer(nullptr),
        m_socketFd(-1),
        m_ioMode(TCP_IO_CONNECTION_MODE_SERVER),
        m_ioClient()
    {
        DEBUG_INFO("construct");
    }

    TcpIoConnection::~TcpIoConnection() {
        DEBUG_INFO("deconstruct");
    }

    int TcpIoConnection::getFd() {
        return m_socketFd;
    }

    int TcpIoConnection::onReadAvailable() {
        if (m_state != COMPONENT_STATE_STARTED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (m_socketFd < 0) {
            DEBUG_ERROR("invalid socket fd");
            return -1;
        }

        auto spClient = m_ioClient.lock();
        if (!spClient) {
            DEBUG_ERROR("no io client instance");
            return -1;
        }

        TcpHeader_s tcpHeader;
        ssize_t recvBytes = recv(m_socketFd, m_recvBuffer, TCP_HEADER_SIZE, MSG_WAITALL);
        if (recvBytes < 0)
        {
            DEBUG_ERROR("recv data fail, res: %ld", recvBytes);
        }
        else if (recvBytes == 0)
        {
            DEBUG_INFO("the connection is being closed");
            spClient->handleEvent(TCP_IO_CONNECTION_EVENT_CLOSE, &m_socketFd);
        }
        else
        {
            tcpHeader.deserialerFrom(m_recvBuffer, recvBytes);
            DEBUG_INFO("will receive data size: %u", tcpHeader.messageLen);

            if (tcpHeader.messageLen > m_bufferSize)
            {
                DEBUG_ERROR("tcp message length - %d excced the buffer size - %d", tcpHeader.messageLen, m_bufferSize);
            }
            else
            {
                recvBytes = recv(m_socketFd, m_recvBuffer, tcpHeader.messageLen, MSG_WAITALL);
                if (recvBytes < 0) {
                    DEBUG_ERROR("recv data fail, res: %ld", recvBytes);
                }
                else if (recvBytes == 0) {
                    DEBUG_INFO("the connection is being closed");
                    spClient->handleEvent(TCP_IO_CONNECTION_EVENT_CLOSE, &m_socketFd);
                }
                else {
                    DEBUG_INFO("recv data size: %ld", recvBytes);
                    (void)spClient->handleData(m_recvBuffer, recvBytes);
                }
            }
        }
    
        return recvBytes;
    }

    int TcpIoConnection::onWriteAvailable() {
        //Not implement
        return 0;
    }

    int TcpIoConnection::onControlAvailable() {
        //Not implement
        return 0;
    }

    int TcpIoConnection::sendData(const char* data, int len) {
        if ((m_state != COMPONENT_STATE_STARTED)
            || (m_socketFd < 0)) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (!data || len <= 0) {
            DEBUG_ERROR("invalid params");
            return -1;
        }

        char headerBuffer[TCP_HEADER_SIZE];
        TcpHeader_s tcpHeader;
        tcpHeader.messageLen = len;
        tcpHeader.serialerTo(headerBuffer, TCP_HEADER_SIZE);
        int res = send(m_socketFd, headerBuffer, TCP_HEADER_SIZE, 0);
        if (0 > res) {
            DEBUG_ERROR("unable to send tcp header to socket: %d", m_socketFd);
        }

        DEBUG_INFO("try to send data of size: %d to socket: %d", len, m_socketFd);
        res = send(m_socketFd, data, len, 0);
        if (0 > res) {
            DEBUG_ERROR("unable to send data of size: %d to socket: %d", len, m_socketFd);
        }
        DEBUG_INFO("send data of size: %d to socket: %d successfully", len, m_socketFd);

        return res;
    }

    IoMemberErrorAction TcpIoConnection::onIoError(IoMemberError_e error) {
        IoMemberErrorAction action = IO_MEMBER_ERROR_ACTION_NULL;
        switch(error)
        {
            case IO_MEMBER_ERROR_EPOLL_FAILED:
            {
                DEBUG_INFO("encounter epoll error, maybe the connection is being closed");
                auto spClient = m_ioClient.lock();
                if (spClient) {
                    spClient->handleEvent(TCP_IO_CONNECTION_EVENT_CLOSE, &m_socketFd);
                }
                break;
            }
            default:
            {
                DEBUG_ERROR("receive known error: %d", error);
                break;
            }
        }

        return action;
    }

    void TcpIoConnection::setIoClient(std::weak_ptr<IoConnectionClientInterface> client) {
        m_ioClient = client;
    }

    void TcpIoConnection::setIoFd(int socketFd) {
        m_socketFd = socketFd;
    }

    int TcpIoConnection::start() {
        if (m_state != COMPONENT_STATE_CREATED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (!m_ioClient.lock()) {
            DEBUG_ERROR("no io client instance");
            return -1;
        }

        if (m_socketFd < 0) {
            DEBUG_ERROR("invalid socket id");
            return -1;
        }

        m_recvBuffer = new char[m_bufferSize];
        m_state = COMPONENT_STATE_STARTED;
        return 0;
    }

    int TcpIoConnection::start(std::string Ip, int port) {
        if (m_state != COMPONENT_STATE_CREATED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (!m_ioClient.lock()) {
            DEBUG_ERROR("no io client instance");
            return -1;
        }

        if (Ip.length() < 0 || port < 0) {
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
        serverAdd.sin_addr.s_addr = inet_addr(Ip.c_str());
        if (0 > connect(m_socketFd, reinterpret_cast<struct sockaddr *>(&serverAdd), sizeof(serverAdd))) {
            DEBUG_ERROR("unable to connect to server ip: %s, port: %d", Ip.c_str(), port);
            close(m_socketFd);
            m_socketFd = -1;
            return -1;
        }

        m_recvBuffer = new char[m_bufferSize];
        m_ioMode = TCP_IO_CONNECTION_MODE_CLIENT;
        m_state = COMPONENT_STATE_STARTED;

        DEBUG_INFO("start and connect to service ip: %s, port: %d successfully", Ip.c_str(), port);
        return 0;
    }

    int TcpIoConnection::stop() {
        if (m_state != COMPONENT_STATE_STARTED) {
            DEBUG_ERROR("invalid state");
            return -1;
        }

        if (m_recvBuffer) {
            delete[] m_recvBuffer;
            m_recvBuffer = nullptr;
        }

        if (m_socketFd) {
            if (m_ioMode == TCP_IO_CONNECTION_MODE_CLIENT) {
                DEBUG_INFO("close socket fd: %d", m_socketFd);
                close(m_socketFd);
            }
            m_socketFd = -1;
        }

        m_state = COMPONENT_STATE_CREATED;

        return 0;
    }

    void TcpIoConnection::TcpHeader_s::deserialerFrom(const char* data, int len) {
        if (!data || len < TCP_HEADER_SIZE) {
            DEBUG_ERROR("invalid params");
        }

        magic = TCP_HEADER_MAGIC;
        messageLen = GET_UINT32_FROM_CHARS((uint8_t*)(data) + TCP_HEADER_OFFSET_MESSAGE_LEN);
    }

    void TcpIoConnection::TcpHeader_s::serialerTo(char* buffer, int size) {
        if (!buffer || size < TCP_HEADER_SIZE) {
            DEBUG_ERROR("invalid params");
        }

        buffer[0] = TCP_HEADER_MAGIC;
        SET_UINT32_TO_CHARS(messageLen, (uint8_t*)buffer + TCP_HEADER_OFFSET_MESSAGE_LEN);
    }
}
}