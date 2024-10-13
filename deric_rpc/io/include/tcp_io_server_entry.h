#ifndef _TCP_IO_SERVER_ENTRY_H_
#define _TCP_IO_SERVER_ENTRY_H_

#include "io_interface.h"

#include <functional>

namespace deric
{
class TcpIoServerEntry : public IoMember
{
public:
    using TcpIoServerEntryConnectCallbackType = std::function<void(int)>;

    using TcpIoServerEntryErrorCallbackType = std::function<void(IoMemberError_e)>;

    TcpIoServerEntry(int socketFd);

    ~TcpIoServerEntry();

    int getFd() override;

    void onReadAvailable() override;

    void onWriteAvailable() override;

    void onControlAvailable() override;

    void onIoError(IoMemberError_e error) override;

    int sendData(std::string_view data) override;

    void setConnectCallback(const TcpIoServerEntryConnectCallbackType& callback);

    void setConnectCallback(TcpIoServerEntryConnectCallbackType&& callback);

    void setErrorCallback(const TcpIoServerEntryErrorCallbackType& callback);

    void setErrorCallback(TcpIoServerEntryErrorCallbackType&& callback);

private:
    int m_socketFd;
    TcpIoServerEntryConnectCallbackType m_connectCallback;
    TcpIoServerEntryErrorCallbackType m_errorCallback;
};
}

#endif