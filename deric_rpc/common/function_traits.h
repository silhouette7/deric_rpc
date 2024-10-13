#ifndef _FUNCTION_TRAITS_H_
#define _FUNCTION_TRAITS_H_

#include <functional>
#include <tuple>
#include <type_traits>

namespace deric
{
    template<typename T>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;
        using tuple_args_type = std::tuple<std::decay_t<Args>...>;
    };

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {};

    template<typename R, typename... Args>
    struct function_traits<std::function<R(Args...)>> : function_traits<R(Args...)> {};

    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> : function_traits<R(Args...)> {};
}
#endif