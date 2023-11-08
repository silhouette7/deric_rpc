#ifndef _MESSAGE_PUBLIC_H_
#define _MESSAGE_PUBLIC_H_

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
}
}

#endif