#ifndef _RPC_MESSAGES_H_
#define _RPC_MESSAGES_H_

namespace deric::rpc
{
enum class MessageType{
    INVALID,
    COMMAND,
    REPLAY,
    NOTIFY,
    REGISTER,
    UNREGISTER
};
}

#endif