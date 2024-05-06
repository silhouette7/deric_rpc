#ifndef _MESSAGE_PUBLIC_H_
#define _MESSAGE_PUBLIC_H_

#include <cstdint>

namespace deric
{
namespace rpc
{
typedef enum {
    MESSAGE_TYPE_COMMAND,
    MESSAGE_TYPE_REPLAY,
    MESSAGE_TYPE_NOTIFY,
    MESSAGE_TYPE_REGISTER
} MessageType_e;

inline uint32_t GET_UINT32_FROM_CHARS(const uint8_t* data) {
    return static_cast<uint32_t>(data[0] << 24)
            | static_cast<uint32_t>(data[1] << 16)
            | static_cast<uint32_t>(data[2] << 8)
            | static_cast<uint32_t>(data[3]);
}

inline void SET_UINT32_TO_CHARS(uint32_t num, uint8_t* data) {
    data[0] = static_cast<uint8_t>(num >> 24);
    data[1] = static_cast<uint8_t>(num >> 16);
    data[2] = static_cast<uint8_t>(num >> 8);
    data[3] = static_cast<uint8_t>(num);
}
}
}

#endif