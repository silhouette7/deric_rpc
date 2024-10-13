#ifndef _TCP_IO_CONNECTION_H_
#define _TCP_IO_CONNECTION_H_

#include "io_interface.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

namespace deric
{
class TcpIoConnection;

using TcpIoConnectCallbackType = std::function<void(const std::shared_ptr<TcpIoConnection>&)>;

using TcpIoCloseCallbackType = std::function<void(const std::shared_ptr<TcpIoConnection>&)>;

using TcpIoDataCallbackType = std::function<void(const std::shared_ptr<TcpIoConnection>&, const std::string&)>;

class TcpIoConnection : public IoMember,
                        public std::enable_shared_from_this<TcpIoConnection>
{
public:
    TcpIoConnection();

    TcpIoConnection(int socket);

    ~TcpIoConnection();

    int getFd() override;

    void onReadAvailable() override;

    void onWriteAvailable() override;

    void onControlAvailable() override;

    void onIoError(IoMemberError_e) override;

    int sendData(std::string_view data) override;

    int connect();

    int connect(std::string_view ip, int port);
    
    int shutdown();

    int destory();

    bool isConnected();

    void setConnectCallback(const TcpIoConnectCallbackType& callback);

    void setConnectCallback(TcpIoConnectCallbackType&& callback);

    void unsetConnectCallback();

    void setCloseCallback(const TcpIoCloseCallbackType& callback);

    void setCloseCallback(TcpIoCloseCallbackType&& callback);

    void unsetCloseCallback();

    void setIoDataCallback(const TcpIoDataCallbackType& callback);

    void setIoDataCallback(TcpIoDataCallbackType&& callback);

    void unsetIoDataCallback();

private:
    constexpr static uint8_t TCP_HEADER_MAGIC = 0x01;
    constexpr static size_t TCP_HEADER_OFFSET_MAGIC = 0;
    constexpr static size_t TCP_HEADER_OFFSET_MESSIGE_LEN = 0;
    constexpr static size_t TCP_HEADER_SIZE = 5;

    enum class ConnectState {
        DISCONNCTED,
        CONNECTED
    };

    typedef struct{
        uint8_t magic;
        uint32_t messageLen;

        void deserialerFrom(const char* data, int len);
        void serialerTo(char* buffer, int size);
    } TcpHeader_s;

    void onConnected();

    void onShutdown();

    void onClosed();

    void onDestoryed();

    void handleClose();

    inline static uint32_t GET_UINT32_FROM_CHARS(const uint8_t* data) {
        return static_cast<uint32_t>(data[0] << 24)
                | static_cast<uint32_t>(data[1] << 16)
                | static_cast<uint32_t>(data[2] << 8)
                | static_cast<uint32_t>(data[3]);
    }

    inline static void SET_UINT32_TO_CHARS(uint32_t num, uint8_t* data) {
        data[0] = static_cast<uint8_t>(num >> 24);
        data[1] = static_cast<uint8_t>(num >> 16);
        data[2] = static_cast<uint8_t>(num >> 8);
        data[3] = static_cast<uint8_t>(num);
    }

    ConnectState m_connectState;
    int m_socketFd;
    std::mutex m_mutex;
    std::string m_sendBuffer;
    std::string m_receiveBuffer;
    TcpIoConnectCallbackType m_connectCallback;
    TcpIoCloseCallbackType m_closeCallback;
    TcpIoDataCallbackType m_dataCallback;
    void* m_context;
};
}

#endif