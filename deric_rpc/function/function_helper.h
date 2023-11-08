#ifndef _FUNCTION_HELPER_H_
#define _FUNCTION_HELPER_H_

#include <memory>
#include <functional>

#include "rpc_serialer.h"
#include "function_traits.h"

namespace deric
{
namespace rpc
{

class FunctionHelper
{
public:
    FunctionHelper() {
    }

    ~FunctionHelper() {
    }

    int setSerialer(std::shared_ptr<RpcSerialer> _serialer) {
        m_serialer = _serialer;
        return 0;
    }

    template<typename F>
    int evec(F func, const char* data, int len, std::string& resultString) {
        int res = 0;

        if (!m_serialer) {
            return -1;
        }

        using tuple_args_type = typename function_traits<F>::Tuple_Args_Type;
        using return_type = typename function_traits<F>::Return_Type;

        tuple_args_type args = m_serialer->getMessageData<tuple_args_type>(data, len);
        auto funcRes = callFunc(func, args, std::make_index_sequence<std::tuple_size<tuple_args_type>::value>{});
        m_serialer->serialMessageData(funcRes, resultString);

        return res;
    }

    template<typename F, typename O>
    int exec(F func, O* obj, const char* data, int len, std::string& resultString) {
        int res = 0;

        if (!m_serialer) {
            return -1;
        }

        auto funcRes = callMemberFunc(func, obj, data, len);
        m_serialer->serialMessageData(funcRes, resultString);
    
        return res;
    }

private:
    template<typename F, typename T, size_t ...N>
    decltype(auto) callFunc(F&& func, T&& args, std::index_sequence<N...>) {
        return func(std::get<N>(std::forward<T>(args))...);
    }

    template<typename R, typename ...Args, typename O>
    decltype(auto) callMemberFunc(R (O::*F)(Args...), O* obj, const char* data, int len) {
        using tuple_args_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
        tuple_args_type args = m_serialer->getMessageData<tuple_args_type>(data, len);

        auto f = [&](Args... _args)->R{
            return (obj->*F)(_args...);
        };

        return callFunc(f, args, std::make_index_sequence<std::tuple_size<tuple_args_type>{}>{});
    }

    std::shared_ptr<RpcSerialer> m_serialer;
};
}
}

#endif