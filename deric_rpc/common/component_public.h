#ifndef _COMPONENT_PUBLIC_H_
#define _COMPONENT_PUBLIC_H_

namespace deric
{
namespace rpc
{
typedef enum {
    COMPONENT_STATE_CREATED,
    COMPONENT_STATE_INITIALIZED,
    COMPONENT_STATE_STARTED
} ComponentState_e;
}
}

#endif