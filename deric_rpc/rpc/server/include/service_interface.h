#ifndef _SERVICE_INTERFACE_H_
#define _SERVICE_INTERFACE_H_

#include <functional>
#include "rpc_serialer.h"

namespace deric
{
namespace rpc
{

typedef struct {
    std::string serviceIp;
    int servicePort;
    int maxConnectionNumber;
    std::shared_ptr<RpcSerialer> serialer;
} ServiceConfig_s;

class ServiceInterface
{
public:
    using ServiceFunctionType = std::function<int (const char*, int, std::string&)>;

    virtual ~ServiceInterface() {}

    virtual int init(const ServiceConfig_s& serviceConfig) = 0;

    virtual int deInit() = 0;

    virtual int start() = 0;

    virtual int stop() = 0;

    virtual int registerMethod(const std::string& funcName, const ServiceFunctionType& func) = 0;

    virtual int unregisterMethod(const std::string& funcName) = 0;

    virtual int postEvent(const std::string& eventName, const char* pData, int len) = 0;
};

}
}

#endif