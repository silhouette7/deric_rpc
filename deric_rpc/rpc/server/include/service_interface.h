#ifndef _SERVICE_INTERFACE_H_
#define _SERVICE_INTERFACE_H_

#include "rpc_serialer.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace deric::rpc
{
class ServiceInterface
{
public:
    using ServiceFunctionType = std::function<std::optional<std::string>(std::string_view)>;

    virtual ~ServiceInterface() {}

    virtual int start() = 0;

    virtual int stop() = 0;

    virtual int registerMethod(const std::string& funcName, const ServiceFunctionType& func) = 0;

    virtual int registerMethod(const std::string& funcName, ServiceFunctionType&& func) = 0;

    virtual int unregisterMethod(const std::string& funcName) = 0;

    virtual int registerEvent(const std::string& funcName) = 0;

    virtual int unregisterEvent(const std::string& funcName) = 0;

    virtual int postEvent(const std::string& eventName) = 0;
};
}

#endif