#ifndef _SERVICE_CLIENT_ENTRY_INTERFACE_H_
#define _SERVICE_CLIENT_ENTRY_INTERFACE_H_

namespace deric
{
namespace rpc
{
class ServiceClientEntryInterface
{
public:
    virtual int startClientEntry() = 0;

    virtual int stopClientEntry() = 0;

    virtual uint32_t getClientId() = 0;

    virtual int sendData(const std::string& data) = 0;
};
    
}
}

#endif