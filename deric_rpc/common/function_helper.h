#ifndef _FUNCTION_HELPER_
#define _FUNCTION_HELPER_

#include <memory>
#include <type_traits>

namespace deric::functionhelper
{
template<typename F>
auto make_copyable_function(F&& f) {
    using function_type = std::decay_t<F>;

    std::shared_ptr<function_type> spFunc = std::make_shared<function_type>(std::forward<F>(f));
    return [spFunc](auto&&... args){return (*spFunc)(std::forward<decltype(args)>(args)...);};
}
}
#endif