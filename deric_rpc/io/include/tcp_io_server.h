#ifndef _TCP_IO_SERVER_H_
#define _TCP_IO_SERVER_H_

#include <memory>
#include <map>

#include "component_public.h"
#include "io_server_client_interface.h"
#include "tcp_io_connection.h"

namespace deric
{
namespace rpc
{
typedef struct {
    std::string ip;
    int port;
    int maxConnectionNumber;
    std::shared_ptr<IoServerClientInterface> serverClient;
} TcpIoServerConfig_s;

class TcpIoServer : public std::enable_shared_from_this<TcpIoServer>
{
public:
    TcpIoServer();

    ~TcpIoServer();

    int init(const TcpIoServerConfig_s& config);

    int deInit();

    int start();

    int stop();

    int getFd();

    int onConnectRequest();

    void onIoError();
public:
    ComponentState_e m_state;
    std::string m_ip;
    int m_port;
    int m_maxConnectionNumber;
    int m_socketFd;
    std::shared_ptr<IoServerClientInterface> m_client;
    std::shared_ptr<IoMember> m_ioEntry;
};
}
}

#endif