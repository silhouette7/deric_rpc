#include "tcp_io_server_entry.h"

#include "deric_debug.h"

#include <sys/socket.h>
#include <unistd.h>

namespace deric
{
    TcpIoServerEntry::TcpIoServerEntry(int socketFd) :
        m_socketFd(socketFd),
        m_connectCallback(),
        m_errorCallback()
    {
        DEBUG_INFO("construct");
    }

    TcpIoServerEntry::~TcpIoServerEntry() {
        DEBUG_INFO("deconstruct");
        if (m_socketFd) {
            close(m_socketFd);
        }
    }

    int TcpIoServerEntry::getFd() {
        return m_socketFd;
    }

    void TcpIoServerEntry::onReadAvailable() {
        if (m_connectCallback) {
            int acceptSocket = accept(m_socketFd, NULL, NULL);
            m_connectCallback(acceptSocket);
        }
    }

    void TcpIoServerEntry::onWriteAvailable() {
        //Not implement
    }

    void TcpIoServerEntry::onControlAvailable() {
        //Not implement
    }

    void TcpIoServerEntry::onIoError(IoMemberError_e error) {
        if (m_errorCallback) {
            m_errorCallback(error);
        }
    }

    int TcpIoServerEntry::sendData(std::string_view) {
        //Not implement
        return -1;
    }

    void TcpIoServerEntry::setConnectCallback(const TcpIoServerEntryConnectCallbackType& callback) {
        m_connectCallback = callback;
    }

    void TcpIoServerEntry::setConnectCallback(TcpIoServerEntryConnectCallbackType&& callback) {
        m_connectCallback = std::move(callback);
    }

    void TcpIoServerEntry::setErrorCallback(const TcpIoServerEntryErrorCallbackType& callback) {
        m_errorCallback = callback;
    }

    void TcpIoServerEntry::setErrorCallback(TcpIoServerEntryErrorCallbackType&& callback) {
        m_errorCallback = std::move(callback);
    }
}