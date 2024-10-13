#ifndef _RPC_SERIALER_H_
#define _RPC_SERIALER_H_

#include "deric_debug.h"
#include "rpc_messages.h"

#include <msgpack.hpp>

#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <sstream>
#include <tuple>

namespace deric::rpc::serialer
{
typedef struct {
    MessageType msgType;
    std::string msgName;
    int msgId;
    bool result;
} MessageMeta;

typedef struct {
    std::string_view meta;
    std::string_view payload;
} SerialerStr;

template<typename T>
inline auto serialMessageData(T&& t) -> std::enable_if_t<!std::is_same_v<std::decay_t<T>, MessageMeta>, std::string>  {
    std::stringstream ss;
    msgpack::pack(ss, std::forward<T>(t));
    return ss.str();
}

template<typename T>
inline auto serialMessageData(T&& t) -> std::enable_if_t<std::is_same_v<std::decay_t<T>, MessageMeta>, std::string>  {
    std::stringstream ss;
    auto tuple_arg = std::make_tuple(static_cast<std::underlying_type_t<MessageType>>(t.msgType), t.msgName, t.msgId, t.result);
    msgpack::pack(ss, std::forward<decltype(tuple_arg)>(tuple_arg));
    return ss.str();
}

template<typename T>
inline auto getMessageData(std::string_view data) -> std::enable_if_t<!std::is_same_v<std::decay_t<T>, MessageMeta>, std::optional<T>> {
    try {
        auto oh = msgpack::unpack(data.data(), data.length());
        return oh.get().as<T>();
    }
    catch(...) {
        DEBUG_ERROR("get message type fail");
        return std::nullopt;
    }
}

template<typename T>
inline auto getMessageData(std::string_view data) -> std::enable_if_t<std::is_same_v<std::decay_t<T>, MessageMeta>, std::optional<T>> {
    try {
        auto oh = msgpack::unpack(data.data(), data.length());
        auto tuple_arg = oh.get().as<std::tuple<std::underlying_type_t<MessageType>, std::string, int, bool>>();
        return MessageMeta{static_cast<MessageType>(std::get<0>(tuple_arg)), std::get<1>(tuple_arg), std::get<2>(tuple_arg), std::get<3>(tuple_arg)};
    }
    catch(...) {
        DEBUG_ERROR("get message type fail");
        return std::nullopt;
    }
}

inline std::string generateRpcStr(std::string_view metaStr, std::string_view payload) {
    std::string res;
    std::size_t metaSize = metaStr.size(), payloadSize = payload.size(), offset = 0;

    res.resize(metaSize + payloadSize + 8);
    memcpy(res.data(), &metaSize, 4);
    offset += 4;
    memcpy(res.data() + offset, metaStr.data(), metaSize);
    offset += metaSize;
    memcpy(res.data() + offset, &payloadSize, 4);
    offset += 4;
    if (payloadSize > 0) {
        memcpy(res.data() + offset, payload.data(), payloadSize);
    }
    return res;
}

inline std::optional<SerialerStr> getSerialStrFromRpcStr(std::string_view data) {
    SerialerStr res;
    std::size_t metaSize = 0, payloadSize = 0, totalLen = 4;

    if (data.size() < totalLen) {
        DEBUG_ERROR("invalid data, expect size %lu, real size: %lu", data.size(), totalLen);
        return std::nullopt;
    }
    memcpy(&metaSize, data.data(), 4);

    totalLen += (metaSize + 4);
    if (data.size() < totalLen) {
        DEBUG_ERROR("invalid data, expect size %lu, real size: %lu", totalLen, data.size());
        return std::nullopt;
    }
    memcpy(&payloadSize, data.data() + 4 + metaSize, 4);

    totalLen += payloadSize;
    if (data.size() < totalLen) {
        DEBUG_ERROR("invalid data, expect size %lu, real size: %lu", 4 + metaSize + 4 + payloadSize, data.size());
        return std::nullopt;
    }

    res.meta = data.substr(4, metaSize);
    res.payload = data.substr(4 + metaSize + 4, payloadSize);

    return res;
}

}
#endif