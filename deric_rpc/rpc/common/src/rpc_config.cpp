#include <fstream>

#include "deric_debug.h"
#include "rpc_config.h"

namespace deric::rpc
{
int RpcConfig::init(const std::string& configFile) {
    std::ifstream inFile(configFile);
    if (!inFile.good()) {
        DEBUG_ERROR("open config file %s failed", configFile.c_str());
        return -1;
    }

    for(std::string buf; std::getline(inFile, buf);) {
        DEBUG_INFO("parse line: %s", buf.c_str());
        parseLine(buf);
    }
    return 0;
}

int RpcConfig::getValue(const std::string& key, std::string& val) const {
    auto item = m_configItemMap.find(key);
    if (item == m_configItemMap.end()) {
        DEBUG_ERROR("cannot find value of %s", key.c_str());
        return -1;
    }
    else {
        val = item->second;
        return 0;
    }
}

void RpcConfig::parseLine(const std::string& buf) {
    std::size_t index = 0;

    if (buf.empty() || buf[index]=='#') {
        return;
    }

    std::size_t equalPos = buf.find('=');
    if (equalPos == std::string::npos) {
        DEBUG_ERROR("invalid buf: %s", buf.c_str());
        return;
    }

    for(index = equalPos - 1; index > 0 && buf[index] == ' '; --index) {}
    if (index == 0 && buf[index] == ' ') {
        DEBUG_ERROR("key is empty");
        return;
    }
    std::string key(buf.begin(), buf.begin() + index + 1);

    index = buf.find_first_not_of(' ', equalPos + 1);
    if (index == std::string::npos) {
        DEBUG_ERROR("value is empty");
        return;
    }
    std::string value(buf, index, buf.length() - index);

    m_configItemMap.emplace(std::make_pair(std::move(key), std::move(value)));
}
}