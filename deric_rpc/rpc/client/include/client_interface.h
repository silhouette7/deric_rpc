#ifndef _CLIENT_INTERFACE_H_
#define _CLIENT_INTERFACE_H_

#include <string>
#include <functional>

namespace deric
{
namespace rpc
{

typedef struct {
    std::string serviceIp;
    std::string servicePort;
    int ioBufferSize;
} ClientConnectionConfig_s;

class ClientInterface
{
public:
    using ClientMessageCallbackType = std::function<void(const std::string&)>;

    virtual ~ClientInterface() {}

    virtual int connect(const ClientConnectionConfig_s& _config) = 0;

    virtual int disconnect() = 0;

    virtual int sendMsg(const std::string& msg) = 0;

    virtual int registerMessageCallback(const ClientMessageCallbackType& func) = 0;
};
}
}

#endif