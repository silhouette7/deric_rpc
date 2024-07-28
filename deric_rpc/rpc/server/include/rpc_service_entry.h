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
            std::weak_ptr<FunctionHelper> wp = m_funcHelper;
            res = registerMethodImpl(funcName, [wp, func](const char* data, int len, std::string& resultStr)->int
                                                            { auto sp = wp.lock(); 
                                                              if (sp) return sp->exec<F>(func, data, len, resultStr);
                                                              return -1; });
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
            std::weak_ptr<FunctionHelper> wp = m_funcHelper;
            res = registerMethodImpl(funcName, [wp, func, obj](const char* data, int len, std::string& resultStr)->int
                                                                { auto sp = wp.lock(); 
                                                                  if (sp) return sp->exec<F, O>(func, obj, data, len, resultStr);
                                                                  return -1; });
        }
        return res;
    }

private:
    int registerMethodImpl(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func);

    std::string m_serviceName;
    std::shared_ptr<ServiceInterface> m_serviceImpl;
    std::shared_ptr<FunctionHelper> m_funcHelper;
};
}
}

#endif