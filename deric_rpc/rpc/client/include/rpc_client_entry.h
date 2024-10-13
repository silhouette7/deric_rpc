#ifndef _RPC_CLIENT_ENTRY_H_
#define _RPC_CLIENT_ENTRY_H_

#include "client_interface.h"
#include "config_interface.h"
#include "function_helper.h"
#include "rpc_messages.h"
#include "rpc_serialer.h"

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <mutex>
#include <tuple>
#include <string>
#include <string_view>

namespace deric::rpc
{
class RpcClientEntry
{
public:
    using RpcClientEntryEventCallbackType = std::function<void()>;

    RpcClientEntry() = default;

    ~RpcClientEntry() = default;

    int init(const ConfigInterface& config);

    int deinit();

    int connect(const std::string& serviceName);

    int disconnect();

    void subscribeToEvent(const std::string& eventName, const RpcClientEntryEventCallbackType& callback);

    void subscribeToEvent(const std::string& eventName, RpcClientEntryEventCallbackType&& callback);

    void unsubscribeToEvent(const std::string& eventName);

    template <typename R>
    void invokeResCallback(std::promise<R>& p, std::string_view data) {
        if constexpr (std::is_void_v<R>) {
            (void)data;
            p.set_value();
        }
        else {
            std::optional<R> o = serialer::getMessageData<R>(data);
            if (o) {
                p.set_value(o.value());
            }
        }
    }

    template<typename R, typename... Args>
    std::future<R> invoke(std::string_view funcName, Args&&... args) {
        std::promise<R> p;
        auto res = p.get_future();
        auto replayCallback = [p = std::move(p), this](std::string_view data)mutable {invokeResCallback(p, data);};
        int msgId = _getMessageId();
        
        {
            std::lock_guard<std::mutex> g(m_replayLock);
            m_replyCallbacks[msgId] = functionhelper::make_copyable_function(std::move(replayCallback));
        }

        serialer::MessageMeta cmdMeta = {MessageType::COMMAND, std::string(funcName), msgId, true};
        std::string rpcMeta = serialer::serialMessageData(std::move(cmdMeta));
        std::string rpcData = serialer::serialMessageData(std::make_tuple(std::forward<Args>(args)...));
        std::string rpcStr = serialer::generateRpcStr(rpcMeta, rpcData);
        if (0 > _invoke(rpcStr)) {
            std::lock_guard<std::mutex> g(m_replayLock);
            m_replyCallbacks.erase(msgId);
        }
        return res;
    }

    void serviceMessageCallback(const std::string& msg);

private:
    static constexpr int DEFAULT_RPC_CLIENT_BUFFER_SIZE = 2048;

    using InvokeReplyCallbakType = std::function<void(std::string_view)>;

    int _subscribe(const std::string& name, bool subscribe);

    int _invoke(const std::string& invokeStr);

    int _getMessageId();

    std::unique_ptr<ClientInterface> m_clientImpl;
    std::string m_serviceBook;
    std::mutex m_replayLock;
    std::mutex m_eventLock;
    std::map<int, InvokeReplyCallbakType> m_replyCallbacks;
    std::map<std::string, RpcClientEntryEventCallbackType> m_eventCallback;
};
}
#endif