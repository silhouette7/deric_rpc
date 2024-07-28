#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "deric_debug.h"
#include "tcp_io_server_entry.h"
#include "io_module.h"
#include "tcp_io_server.h"

namespace deric
{
namespace rpc
{
    TcpIoServer::TcpIoServer() :
        m_state(COMPONENT_STATE_CREATED),
        m_ip(),
        m_port(-1),
        m_maxConnectionNumber(0),
        m_socketFd(-1),
        m_client(),
        m_ioEntry()
    {
        DEBUG_INFO("construct");
    }

    TcpIoServer::~TcpIoServer() {
        DEBUG_INFO("deconstruct");
    }

    int TcpIoServer::init(const TcpIoServerConfig_s& config) {
        int res = 0;

        if (m_state == COMPONENT_STATE_CREATED) {
            m_ip = config.ip;
            m_port = config.port;
            m_maxConnectionNumber = config.maxConnectionNumber;
            m_client = config.serverClient;
            m_state = COMPONENT_STATE_INITIALIZED;

            res = 0;
        }
        else {
            DEBUG_ERROR("invalid state");
            res = -1;
        }

        return res;
    }

    int TcpIoServer::deInit() {
        int res = 0;

        if (m_state == COMPONENT_STATE_STARTED) {
            res = stop();
        }

        m_ip = "";
        m_port = -1;
        m_maxConnectionNumber = 0;
        m_client.reset();
        m_state = COMPONENT_STATE_CREATED;
        return res;
    }

    int TcpIoServer::start() {
        int res = 0;

        do {
            if (m_state != COMPONENT_STATE_INITIALIZED) {
                DEBUG_ERROR("invalid state");
                res = -1;
                break;
            }

            m_socketFd = socket(PF_INET, SOCK_STREAM, 0);
            if (0 > m_socketFd) {
                DEBUG_ERROR("create socket fail");
                res = -1;
                break;
            }

            struct sockaddr_in serverAdd;
            memset(&serverAdd, 0, sizeof(serverAdd));
            serverAdd.sin_family = AF_INET;
            serverAdd.sin_port = htons(m_port);
            serverAdd.sin_addr.s_addr = inet_addr(m_ip.c_str());
            res = bind(m_socketFd, reinterpret_cast<struct sockaddr*>(&serverAdd), sizeof(serverAdd));
            if (0 > res) {
                DEBUG_ERROR("bind server ip %s, port %d failed", m_ip.c_str(), m_port);
                close(m_socketFd);
                break;
            }
            
            res = listen(m_socketFd, m_maxConnectionNumber);
            if (0 > res) {
                DEBUG_ERROR("listen failed");
                close(m_socketFd);
                break;
            }
            
            m_ioEntry = std::make_shared<TcpIoServerEntry>(m_socketFd, shared_from_this());
            res = IoModule::getInstance().addIoMember(m_ioEntry);
            if (0 > res) {
                DEBUG_ERROR("add io member fail");
                m_ioEntry.reset();
                close(m_socketFd);
                break;
            }

        } while(0);

        if (res >= 0) {
            m_state = COMPONENT_STATE_STARTED;
        }

        DEBUG_INFO("io server start res: %d", res);
        return res;
    }

    int TcpIoServer::stop() {
        int res = 0;

        if (m_state != COMPONENT_STATE_STARTED) {
            DEBUG_ERROR("invalid state");
            res = -1;
        }
        else {
            if (m_ioEntry) {
                IoModule::getInstance().removeIoMember(m_ioEntry);
                m_ioEntry.reset();
            }
            close(m_socketFd);
            m_socketFd = -1;
            m_state = COMPONENT_STATE_INITIALIZED;
        }

        return res;
    }

    int TcpIoServer::getFd() {
        return m_socketFd;
    }

    int TcpIoServer::onConnectRequest() {
        int res = 0;

        do {
            if (m_state != COMPONENT_STATE_STARTED) {
                DEBUG_ERROR("invalid state");
                res = -1;
                break;
            }

            if (!m_client) {
                DEBUG_ERROR("error no client");
                res = -1;
                break;
            }

            int acceptSocket = accept(m_socketFd, NULL, NULL);
            if (0 > acceptSocket) {
                DEBUG_ERROR("accept socket fail");
                res = -1;
                break;
            }

            std::shared_ptr<IoConnectionClientInterface> spConnectionClient;
            m_client->createIoConnectionClient(spConnectionClient);
            if (!spConnectionClient) {
                DEBUG_ERROR("unable to create io connection client");
                res = -1;
                break;
            }

            std::shared_ptr<TcpIoConnection> spConnection = std::make_shared<TcpIoConnection>();
            spConnection->setIoFd(acceptSocket);
            spConnection->setIoClient(spConnectionClient);
            spConnectionClient->setIoConnection(spConnection);
            res = spConnectionClient->startIoClient();
            if (res < 0) {
                DEBUG_ERROR("unable to start io client");
                m_client->removeIoConnectionClient(spConnectionClient);
                close(acceptSocket);
                break;
            }
            DEBUG_INFO("add connection for socket fd: %d successfully", acceptSocket);
        } while(0);

        return res;
    }

    void TcpIoServer::onIoError() {
        (void)stop();
    }
}
}