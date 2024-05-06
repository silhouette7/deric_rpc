#include <cstdlib>
#include <functional>

#include "deric_debug.h"
#include "rpc_serialer.h"
#include "rpc_service_impl.h"
#include "rpc_service_entry.h"

namespace deric
{
namespace rpc
{
    RpcServiceEntry::RpcServiceEntry(const std::string& serviceName) :
        m_serviceName(serviceName),
        m_serviceImpl(std::make_shared<RpcServiceImpl>(m_serviceName)),
        m_funcHelper(std::make_shared<FunctionHelper>())
    {
        DEBUG_INFO("construct");
    }

    RpcServiceEntry::~RpcServiceEntry()
    {
        DEBUG_INFO("deconstruct");
    }

    int RpcServiceEntry::init(const ConfigInterface& config) {
        int res = -1;
        do {
            if (!m_serviceImpl) {
                DEBUG_ERROR("no service impl");
                res = -1;
                break;
            }

            std::string s_bufferSize;
            int bufferSize = DEFAULT_RPC_SERVICE_BUFFER_SIZE;
            if (0 > config.getValue("BufferSize", s_bufferSize)) {
                DEBUG_INFO("no buffer size inficated in the config, using the default value");
            }
            else {
                bufferSize = std::stoi(s_bufferSize);
            }
            
            std::shared_ptr<RpcSerialer> serialer = std::make_shared<RpcSerialer>();

            res = m_funcHelper->setSerialer(serialer);
            if (0 > res) {
                DEBUG_ERROR("set serialer fail with res: %d", res);
                break;
            }

            ServiceConfig_s serviceConfig;
            std::string ip, port;
            if (0 > config.getValue("IP", ip)) {
                DEBUG_ERROR("no IP indicated in the config");
                res = -1;
                break;
            }
            if (0 > config.getValue("Port", port)) {
                DEBUG_ERROR("no port indicated in the config");
                res = -1;
                break;
            }
            serviceConfig.serviceIp = std::move(ip);
            serviceConfig.servicePort = std::stoi(port);
            serviceConfig.ioBufferSize = bufferSize;
            serviceConfig.serialer = serialer;
            res = m_serviceImpl->init(serviceConfig);
            if (0 > res) {
                DEBUG_ERROR("service init fail with res: %d", res);
                break;
            }
        } while(0);
        return res;
    }

    int RpcServiceEntry::deInit() {
        int res = -1;
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            res = -1;
        }
        else {
            res = m_serviceImpl->deInit();
        }
        return res;
    }

    int RpcServiceEntry::start() {
        int res = -1;
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            res = -1;
        }
        else {
            res = m_serviceImpl->start();
        }
        return res;
    }

    int RpcServiceEntry::stop()  {
        int res = -1;
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            res = -1;
        }
        else {
            res = m_serviceImpl->stop();
        }
        return res;
    }

    int RpcServiceEntry::registerMethodImpl(const std::string& funcName, const ServiceInterface::ServiceFunctionType& func) {
        int res = -1;
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            res = -1;
        }
        else {
            res = m_serviceImpl->registerMethod(funcName, func);
        }
        return res;
    }

    int RpcServiceEntry::unregisterMethod(const std::string& funcName) {
        int res = -1;
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            res = -1;
        }
        else {
            res = m_serviceImpl->unregisterMethod(funcName);
        }
        return res;
    }
}
}