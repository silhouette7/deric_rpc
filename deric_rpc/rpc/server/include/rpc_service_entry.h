#ifndef _RPC_SERVICE_ENTRY_H
#define _RPC_SERVICE_ENTRY_H

#include <string>
#include <memory>

#include "function_helper.h"
#include "config_interface.h"
#include "service_interface.h"

namespace deric
{
namespace rpc
{
class RpcServiceEntry
{
public:
    RpcServiceEntry(const std::string& serviceName);

    ~RpcServiceEntry();

    int init(const ConfigInterface& config);

    int deInit();

    int start();

    int stop();

    int unregisterMethod(const std::string& funcName);

    template<typename F>
    int registerMethod(const std::string& funcName, F func) {
        int res = 0;
        if (!m_funcHelper) {
            DEBUG_ERROR("no valid function helper");
            res = -1;
        }
        else {
            res = registerMethodImpl(funcName, std::bind(&FunctionHelper::evec<F>,
                                                         m_funcHelper,
                                                         func,
                                                         std::placeholders::_1,
                                                         std::placeholders::_2,
                                                         std::placeholders::_3));
        }
        return res;
    }

    template<typename F, typename O>
    int registerMethod(const std::string& funcName, F func, O* obj) {
        int res = 0;
        if (!m_funcHelper) {
            DEBUG_ERROR("no valid function helper");
            res = -1;
        }
        else {
            res = registerMethodImpl(funcName, std::bind(&FunctionHelper::exec<F, O>,
                                                         m_funcHelper,
                                                         func,
                                                         obj,
                                                         std::placeholders::_1,
                                                         std::placeholders::_2,
                                                         std::placeholders::_3));
        }
        return res;
    }

    static const int DEFAULT_RPC_SERVICE_BUFFER_SIZE = 2048;

private:
    int registerMethodImpl(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func);

    std::string m_serviceName;
    std::shared_ptr<ServiceInterface> m_serviceImpl;
    std::shared_ptr<FunctionHelper> m_funcHelper;
};
}
}

#endif