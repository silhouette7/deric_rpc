#ifndef _CLIENT_INTERFACE_H_
#define _CLIENT_INTERFACE_H_

#include <functional>
#include <string>
#include <string_view>

namespace deric::rpc
{
class ClientInterface
{
public:
    using ClientMessageCallbackType = std::function<void(const std::string&)>;

    virtual ~ClientInterface() {}

    virtual int connect(std::string_view ip, std::string_view port) = 0;

    virtual int disconnect() = 0;

    virtual int sendMsg(std::string_view msg) = 0;

    virtual int registerMessageCallback(const ClientMessageCallbackType& func) = 0;

    virtual int registerMessageCallback(ClientMessageCallbackType&& func) = 0;
};
}

#endif