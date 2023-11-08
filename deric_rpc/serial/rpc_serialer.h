#ifndef _RPC_SERIALER_H_
#define _RPC_SERIALER_H_

#include <sstream>
#include <msgpack.hpp>
#include "deric_debug.h"

namespace deric
{
namespace rpc
{

class RpcSerialer
{
public:
    RpcSerialer() {}

    virtual ~RpcSerialer() {}

    template<typename T>
    int serialMessageData(T&& t, std::string& outString) {
        std::stringstream ss;
        ss << outString;
        msgpack::pack(ss, std::forward<T>(t));
        outString = ss.str();
        return 0;
    }

    int serialMessageMeta(int msgType, const std::string& msgName, int msgId, std::string& outString) {
        std::stringstream ss;
        msgpack::pack(ss, msgType);
        msgpack::pack(ss, msgName);
        msgpack::pack(ss, msgId);
        outString = ss.str();
        return 0;
    }

    int getMessageType(const char* pData, int len) {
        std::size_t offset = 0;
        try {
            auto oh = msgpack::unpack(pData, len, offset);
            return oh.get().as<int>();
        }
        catch(...) {
            DEBUG_ERROR("get message type fail");
            return -1;
        }
    }

    std::string getMessageName(const char* pData, int len) {
        std::size_t offset = 0;
        try {
            auto oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            return oh.get().as<std::string>();
        }
        catch(...) {
            DEBUG_ERROR("get message type fail");
            return "";
        }
    }

    int getMessageId(const char* pData, int len) {
        std::size_t offset = 0;
        try {
            auto oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            return oh.get().as<int>();
        }
        catch(...) {
            DEBUG_ERROR("get message type fail");
            return -1;
        }
    }

    template<typename T>
    T getMessageData(const char* pData, int len) {
        std::size_t offset = 0;
        try {
            auto oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            oh = msgpack::unpack(pData, len, offset);
            return oh.get().as<T>();
        }
        catch(...) {
            DEBUG_ERROR("get message type fail");
            return T();
        }
    }
};

}
}

#endif