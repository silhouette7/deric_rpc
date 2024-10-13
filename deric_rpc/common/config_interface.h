#ifndef _CONFIG_INTERFACE_H_
#define _CONFIG_INTERFACE_H_

#include <string>

namespace deric
{
class ConfigInterface
{
public:
    ConfigInterface() {}

    virtual ~ConfigInterface() {}

    virtual int init(const std::string& configFile) = 0;

    virtual int getValue(const std::string& key, std::string& val) const = 0;
};   
}

#endif