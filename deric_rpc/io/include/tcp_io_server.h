#ifndef _TCP_IO_SERVER_H_
#define _TCP_IO_SERVER_H_

#include "tcp_io_connection.h"
#include "tcp_io_server_entry.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace deric
{
class TcpIoServer
{
public:
    constexpr static int DEFAULT_MAX_CONNECTION_NUMBER = 100;

    enum class TcpIoServerEvent {
        SERVER_IO_ERROR
    };

    using TcpIoServerNewConnectCallbackType = std::function<void(const std::shared_ptr<TcpIoConnection>&)>;

    using TcpIoServerEventCallbackType = std::function<void(const TcpIoServerEvent&)>;

    TcpIoServer(std::string_view ip, int port);

    TcpIoServer(std::string_view ip, int port, int maxConnectionNumber);

    ~TcpIoServer();

    int start();

    int stop();

    void setNewConnectCallback(const TcpIoServerNewConnectCallbackType& callback);

    void setNewConnectCallback(TcpIoServerNewConnectCallbackType&& callback);

    void setEventCallback(const TcpIoServerEventCallbackType& callback);

    void setEventCallback(TcpIoServerEventCallbackType&& callback);

    void onConnectRequest(int acceptSocket);

    void onIoError(IoMemberError_e error);

    void onIoClose(const std::shared_ptr<TcpIoConnection>& connect);

public:
    bool m_isStarted;
    std::string m_ip;
    int m_port;
    int m_maxConnectionNumber;
    std::shared_ptr<TcpIoServerEntry> m_ioEntry;
    TcpIoServerNewConnectCallbackType m_connectCallback;
    TcpIoServerEventCallbackType m_eventCallback;
    std::mutex m_connectionsLock;
    std::unordered_map<int, std::shared_ptr<TcpIoConnection>> m_connections;
};
}

#endif