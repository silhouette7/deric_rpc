#ifndef _RPC_SERVICE_ENTRY_H
#define _RPC_SERVICE_ENTRY_H

#include "config_interface.h"
#include "rpc_function_executor.h"
#include "service_interface.h"

#include <memory>
#include <optional>
#include <string_view>

namespace deric::rpc
{
class RpcServiceEntry
{
public:
    RpcServiceEntry(std::string_view serviceName);

    ~RpcServiceEntry();

    int init(const ConfigInterface& config);

    int deInit();

    int start();

    int stop();

    int registerEvent(const std::string& eventName);

    int unregisterEvent(const std::string& eventName);

    int postEvent(const std::string& eventName);

    int unregisterMethod(const std::string& funcName);

    template<typename F>
    int registerMethod(const std::string& funcName, F&& func) {
        return registerMethodImpl(funcName, [func = std::forward<F>(func)](std::string_view data)mutable->std::optional<std::string>
                                            {return functionexecutor::exec(std::forward<decltype(func)>(func), data);}
                                 );
    }

    template<typename F, typename O>
    int registerMethod(const std::string& funcName, F func, O* obj) {
        return registerMethodImpl(funcName, [func, obj](std::string_view data)mutable->std::optional<std::string>
                                            {return functionexecutor::exec(func, obj, data);}
                                 );
    }

private:
    int registerMethodImpl(const std::string& funcName, ServiceInterface::ServiceFunctionType&& func);

    std::string m_serviceName;
    std::unique_ptr<ServiceInterface> m_serviceImpl;
};
}

#endif