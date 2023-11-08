#ifndef _FUNCTION_TRAITS_H_
#define _FUNCTION_TRAITS_H_

#include <tuple>

namespace deric
{
    template<typename T>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        using Return_Type = R;
        using Tuple_Args_Type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
    };

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)>
    {
        using Return_Type = R;
        using Tuple_Args_Type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
    };

    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)>
    {
        using Return_Type = R;
        using Tuple_Args_Type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
    };
}
#endif