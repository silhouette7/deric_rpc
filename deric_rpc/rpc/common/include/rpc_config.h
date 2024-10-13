#ifndef _RPC_CONFIG_H_
#define _RPC_CONFIG_H_

#include "config_interface.h"

#include <string>
#include <unordered_map>

namespace deric::rpc
{
class RpcConfig : public ConfigInterface
{
public:
    RpcConfig() = default;
    
    virtual ~RpcConfig() = default;

    virtual int init(const std::string& configFile) override;

    virtual int getValue(const std::string& key, std::string& val) const override;

private:
    void parseLine(const std::string& buf);

    std::unordered_map<std::string, std::string> m_configItemMap;
};

}

#endif