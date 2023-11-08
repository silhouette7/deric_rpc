#include "deric_debug.h"
#include "tcp_io_server_entry.h"

namespace deric
{
namespace rpc
{
    TcpIoServerEntry::TcpIoServerEntry(int socketFd, std::shared_ptr<TcpIoServer> serverImpl) :
        m_socketFd(socketFd),
        m_serverImpl(serverImpl)
    {
        DEBUG_INFO("construct");
    }

    TcpIoServerEntry::~TcpIoServerEntry() {
        DEBUG_INFO("deconstruct");
    }

    int TcpIoServerEntry::getFd() {
        return m_socketFd;
    }

    int TcpIoServerEntry::onReadAvailable() {
        if (m_serverImpl) {
            return m_serverImpl->onConnectRequest();
        }
        else {
            DEBUG_ERROR("no server instance");
            return -1;
        }
    }

    int TcpIoServerEntry::onWriteAvailable() {
        //Not implement
        return 0;
    }

    int TcpIoServerEntry::onControlAvailable() {
        //Not implement
        return 0;
    }

    int TcpIoServerEntry::sendData(const char* data, int len) {
        //Not implement
        return 0;
    }

    IoMemberErrorAction TcpIoServerEntry::onIoError(IoMemberError_e error) {
        DEBUG_ERROR("encounter io error: %d", error);
        if (m_serverImpl) {
            m_serverImpl->onIoError();
        }
        return IO_MEMBER_ERROR_ACTION_NULL;
    }
}
}