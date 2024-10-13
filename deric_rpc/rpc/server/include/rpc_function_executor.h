#ifndef _RPC_FUNCTION_EXECUTOR_H_
#define _RPC_FUNCTION_EXECUTOR_H_

#include "function_traits.h"
#include "rpc_serialer.h"

#include <functional>
#include <optional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace deric::rpc::functionexecutor
{
// global function
template<typename F>
std::optional<std::string> exec(F&& func, std::string_view data) {
    using tuple_args_type = typename function_traits<F>::tuple_args_type;
    using return_type = typename function_traits<F>::return_type;

    std::optional<tuple_args_type> op = serialer::getMessageData<tuple_args_type>(data);
    if (!op) {
        return std::nullopt;
    }
    auto args = op.value();

    if constexpr(std::is_void_v<return_type>) {
        std::apply(std::forward<F>(func), std::move(args));
        return std::nullopt;
    }
    else {
        auto funcRes = std::apply(std::forward<F>(func), std::move(args));
        std::string res;
        serialer::serialMessageData(std::move(funcRes), res);
        return res;
    }
}

// member funtion
template<typename R, typename ...Args, typename O>
std::optional<std::string> exec(R (O::*func)(Args...), O* obj, std::string_view data) {
    using tuple_args_type = std::tuple<std::decay_t<Args>...>;

    std::optional<tuple_args_type> op = serialer::getMessageData<tuple_args_type>(data);
    if (!op) {
        return std::nullopt;
    }
    auto args = op.value();

    if constexpr(std::is_void_v<R>) {
        std::apply([obj, func](Args... args)->R{return (obj->*func)(args...);}, std::move(args));
        return std::nullopt;
    }
    else {
        auto funcRes = std::apply([obj, func](Args... args)->R{return (obj->*func)(args...);}, std::move(args));
        return serialer::serialMessageData(funcRes);
    }
}
}

#endif