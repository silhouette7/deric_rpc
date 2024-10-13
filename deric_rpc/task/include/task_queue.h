#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include "function_helper.h"
#include "nocopyable.h"

#include <functional>
#include <future>
#include <queue>
#include <memory>
#include <mutex>
#include <tuple>

namespace deric
{
class TaskQueue : public Nocopyable
{
public:
    using TaskType = std::function<void()>;

    template<typename F, typename ...Args>
    auto addTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));

        auto func = [f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]()mutable{
            return std::apply([&f](auto&& ...arg){return f(std::forward<decltype(arg)>(arg)...);}, std::move(args));
        };
        std::packaged_task<return_type()> task(std::move(func));
        auto res = task.get_future();
        m_queue.emplace(functionhelper::make_copyable_function(std::move(task)));

        return res;
    }

    TaskType getTask() {
        if (empty()) {
            return TaskType();
        }
        auto task = std::move(m_queue.front());
        m_queue.pop();
        return task;
    }

    bool empty() {
        return m_queue.empty();
    }

    std::size_t size() {
        return m_queue.size();
    }

    void clear() {
        m_queue = decltype(m_queue){};
    }

    TaskQueue() = default;

    ~TaskQueue() = default;

private:
    std::queue<TaskType> m_queue;
};
}

#endif