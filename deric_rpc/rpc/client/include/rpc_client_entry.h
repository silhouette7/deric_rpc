#ifndef _RPC_CLIENT_ENTRY_H_
#define _RPC_CLIENT_ENTRY_H_

#include "rpc_serialer.h"
#include "config_interface.h"
#include "client_interface.h"
#include "message_public.h"
#include <string>
#include <tuple>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <map>

namespace deric
{
namespace rpc
{
class RpcClientEntry : public std::enable_shared_from_this<RpcClientEntry> 
{
public:
    int init(const ConfigInterface& config);

    int deinit();

    int connect(const std::string& serviceName);

    int disconnect();

    template<typename... Args>
    int invoke(const std::string& func, Args&&... args) {
        if (!m_serialer || !m_clientImpl) {
            return -1;
        }

        std::string serialStr;
        int msgId = _getMessageId();
        if (0 > m_serialer->serialMessageMeta(MESSAGE_TYPE_COMMAND, func, msgId, serialStr)) {
            return -1;
        }
        auto as = std::make_tuple(std::forward<Args>(args)...);
        if (0 > m_serialer->serialMessageData(as, serialStr)) {
            return -1;
        }

        return _invoke(serialStr);
    }

    template<typename R, typename... Args>
    typename std::enable_if<!std::is_void<R>::value, int>::type invokeWithResultSync(const std::string& func, R& returnValue, Args&&... args) {
        if (!m_serialer || !m_clientImpl) {
            return -1;
        }

        std::string serialStr;
        int msgId = _getMessageId();
        if (0 > m_serialer->serialMessageMeta(MESSAGE_TYPE_COMMAND, func, msgId, serialStr)) {
            return -1;
        }
        auto as = std::make_tuple(std::forward<Args>(args)...);
        if (0 > m_serialer->serialMessageData(as, serialStr)) {
            return -1;
        }

        if (0 > _invoke(serialStr)) {
            return -1;
        }

        std::string resultStr;
        if (0 > _waitForResult(msgId, resultStr)) {
            return -1;
        }

        returnValue = m_serialer->getMessageData<R>(resultStr.data(), resultStr.length());

        return 0;
    }
 
    template<typename... Args, typename O>
    int invokeWithResultAsync(const std::string& func, Args&&... args, O& observer) {
        // Not implement
        return 0;
    }

    void serviceMessageCallback(const std::string& msg);

    static const int DEFAULT_RPC_CLIENT_BUFFER_SIZE = 2048;

private:
    typedef struct {
        std::string msg;
        std::condition_variable cv;
    } ClientWaitingItem;

    int _invoke(const std::string& invokeStr);

    int _waitForResult(int msgId, std::string& resultStr);

    int _addResultWaitingItem(int msgId);

    int _removeResultWaitingItem(int msgId);

    int _getMessageId();

    std::unique_ptr<RpcSerialer> m_serialer;
    std::shared_ptr<ClientInterface> m_clientImpl;
    std::string m_serviceBook;
    std::mutex m_waitingLock;
    std::map<int, ClientWaitingItem> m_waitingItems;
};
}
}
#endif