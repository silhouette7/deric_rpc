#ifndef _TCP_IO_SERVER_ENTRY_H_
#define _TCP_IO_SERVER_ENTRY_H_

#include <memory>

#include "io_interface.h"
#include "tcp_io_server.h"

namespace deric
{
namespace rpc
{
class TcpIoServerEntry : public IoMember
{
public:
    TcpIoServerEntry(int socketFd, std::shared_ptr<TcpIoServer> serverImpl);

    ~TcpIoServerEntry();

    int getFd() override;

    int onReadAvailable() override;

    int onWriteAvailable() override;

    int onControlAvailable() override;

    int sendData(const char* data, int len) override;

    IoMemberErrorAction onIoError(IoMemberError_e error) override;

private:
    int m_socketFd;
    std::shared_ptr<TcpIoServer> m_serverImpl;
};
}
}

#endif