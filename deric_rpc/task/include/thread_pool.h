#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "nocopyable.h"
#include "task_queue.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace deric
{
class ThreadPool : public Nocopyable
{
public:
    static constexpr int THREAD_POOL_DEFAULT_THREAD_NUM = 3;
    static constexpr int THREAD_POOL_MAX_TASK_NUM = 100;

    typedef struct {
        int threadNum = THREAD_POOL_DEFAULT_THREAD_NUM;
        int maxTaskNum = THREAD_POOL_MAX_TASK_NUM;
    } ThreadPoolConfig_s;

    int init(const ThreadPoolConfig_s& config);

    int deinit();

    template<typename F, typename ...Args>
    auto commit(F&& f, Args&& ...args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));

        std::unique_lock<std::mutex> ul(m_queueMutex);
        m_queueNoFull.wait(ul, [this](){return (m_taskQueue.size() < m_maxTaskNum) || !m_init;});
        if (!m_init) {
            return std::future<return_type>();
        }
        auto res = m_taskQueue.addTask(std::forward<F>(f), std::forward<Args>(args)...);
        m_queueNoEmpty.notify_one();
        return res;
    }

    static ThreadPool& getInstance();

private:
    void initImpl(const ThreadPoolConfig_s& config); 

    void workFunc();

    ThreadPool();

    ~ThreadPool();

    int m_threadNum;
    int m_maxTaskNum;
    bool m_init;
    TaskQueue m_taskQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueNoEmpty;
    std::condition_variable m_queueNoFull;
    std::vector<std::thread> m_threads;
};
}

#endif