#include "rpc_service_entry.h"

#include "deric_debug.h"
#include "rpc_service_impl.h"

#include <cstdlib>
#include <functional>

namespace deric::rpc
{
    RpcServiceEntry::RpcServiceEntry(std::string_view serviceName) :
        m_serviceName(serviceName),
        m_serviceImpl(nullptr)
    {
        DEBUG_INFO("construct");
    }

    RpcServiceEntry::~RpcServiceEntry()
    {
        DEBUG_INFO("deconstruct");
    }

    int RpcServiceEntry::init(const ConfigInterface& config) {
        std::string ip, portStr, maxConnectionNumberStr;
        int port = 0, maxConnectionNumber = -1;
        if (0 > config.getValue("IP", ip)) {
            DEBUG_ERROR("no IP indicated in the config");
            return -1;
        }

        if (0 > config.getValue("Port", portStr)) {
            DEBUG_ERROR("no port indicated in the config");
            return -1;
        }
        try {
            port = std::stoi(portStr);
        }
        catch(...) {
            DEBUG_ERROR("invalid port param: %s", portStr.c_str());
            return -1;
        }

        if (0 == config.getValue("MaxConnectionNumber", maxConnectionNumberStr)) {
            try {
                maxConnectionNumber = std::stoi(maxConnectionNumberStr);
            }
            catch(...) {
                DEBUG_ERROR("invalid maxConnectionNumber param: %s", maxConnectionNumberStr.c_str());
            }
        }

        if (0 < maxConnectionNumber) {
            m_serviceImpl = std::make_unique<RpcServiceImpl>(m_serviceName, ip, port, maxConnectionNumber);
        }
        else {
            m_serviceImpl = std::make_unique<RpcServiceImpl>(m_serviceName, ip, port);
        }

        return 0;
    }

    int RpcServiceEntry::deInit() {
        m_serviceImpl.reset();

        return 0;
    }

    int RpcServiceEntry::start() {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->start();
    }

    int RpcServiceEntry::stop()  {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->stop();
    }

    int RpcServiceEntry::registerEvent(const std::string& eventName) {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->registerEvent(eventName);
    }

    int RpcServiceEntry::unregisterEvent(const std::string& eventName) {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->unregisterEvent(eventName);
    }

    int RpcServiceEntry::postEvent(const std::string& eventName) {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->postEvent(eventName);
    }

    int RpcServiceEntry::unregisterMethod(const std::string& funcName) {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->unregisterMethod(funcName);
    }

    int RpcServiceEntry::registerMethodImpl(const std::string& funcName, ServiceInterface::ServiceFunctionType&& func) {
        if (!m_serviceImpl) {
            DEBUG_ERROR("no service impl");
            return -1;
        }

        return m_serviceImpl->registerMethod(funcName, std::forward<ServiceInterface::ServiceFunctionType>(func));
    }
}