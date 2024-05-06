#ifndef _TCP_IO_CONNECTION_H_
#define _TCP_IO_CONNECTION_H_

#include <memory>

#include "io_connection_client_interface.h"
#include "io_interface.h"
#include "component_public.h"

namespace deric
{
namespace rpc
{
class TcpIoConnection : public IoMember
{
public:
    TcpIoConnection(int bufferSize);

    ~TcpIoConnection();

    int getFd() override;

    int onReadAvailable() override;

    int onWriteAvailable() override;

    int onControlAvailable() override;

    int sendData(const char* data, int len) override;

    IoMemberErrorAction onIoError(IoMemberError_e) override;

    void setIoClient(std::weak_ptr<IoConnectionClientInterface> client);

    void setIoFd(int socketFd);

    int start();

    int start(std::string Ip, int port);
    
    int stop();

private:
    typedef struct{
        uint8_t magic;
        uint32_t messageLen;

        void deserialerFrom(const char* data, int len);
        void serialerTo(char* buffer, int size);
    } TcpHeader_s;

    static const uint8_t TCP_HEADER_MAGIC = 0x01;

    typedef enum{
        TCP_HEADER_OFFSET_MAGIC = 0,
        TCP_HEADER_OFFSET_MESSAGE_LEN = 1,
        TCP_HEADER_SIZE = 5
    } TcpHeaderOffset_e;

    typedef enum {
        TCP_IO_CONNECTION_MODE_SERVER = 0,
        TCP_IO_CONNECTION_MODE_CLIENT
    } TcpIoConnectionMode_e;

    ComponentState_e m_state;
    int m_bufferSize;
    char *m_recvBuffer;
    int m_socketFd;
    TcpIoConnectionMode_e m_ioMode;
    std::weak_ptr<IoConnectionClientInterface> m_ioClient;
};
}
}

#endif