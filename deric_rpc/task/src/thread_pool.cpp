#include "thread_pool.h"

#include <algorithm>

namespace deric
{
static int threadPoolDeinit = false;
static std::once_flag initFlag;

int ThreadPool::init(const ThreadPoolConfig_s& config) {
    try {
        std::call_once(initFlag, [this, &config](){initImpl(config);});
    }
    catch(...) {
        return -1;
    }

    std::lock_guard<std::mutex> ul(m_queueMutex);
    if (!m_init) {
        return -1;
    }

    return 0;
}

int ThreadPool::deinit() {
    std::unique_lock<std::mutex> ul(m_queueMutex);
    if (!m_init) {
        return 0;
    }
    m_init = false;

    //wake up clients that waiting for commit action
    m_queueNoFull.notify_all();

    //make sure all tasks has been executed
    m_queueNoFull.wait(ul, [this](){return m_taskQueue.empty();});

    //wake up all waiting threads
    threadPoolDeinit = true;
    m_queueNoEmpty.notify_all();
    ul.unlock();

    std::for_each(m_threads.begin(), m_threads.end(), [](std::thread& t){t.join();});
    m_threads.clear();
    return 0;
}

ThreadPool& ThreadPool::getInstance() {
    static ThreadPool m_instance;
    return m_instance;
}

void ThreadPool::initImpl(const ThreadPoolConfig_s& config)
{
    m_threadNum = config.threadNum;
    m_maxTaskNum = config.maxTaskNum;

    m_threads.reserve(m_threadNum);
    for (int i = 0; i < m_threadNum; ++i) {
        m_threads.emplace_back(std::thread(&ThreadPool::workFunc, this));
    }

    std::lock_guard<std::mutex> g(m_queueMutex);
    m_init = true;
}

void ThreadPool::workFunc() {
    while(!threadPoolDeinit)
    {
        std::unique_lock<std::mutex> ul(m_queueMutex);
        m_queueNoEmpty.wait(ul, [this](){return !m_taskQueue.empty() || threadPoolDeinit;});
        if (!m_taskQueue.empty()) {
            auto task = m_taskQueue.getTask();
            if (m_taskQueue.size() < m_maxTaskNum) {
                m_queueNoFull.notify_one();
            }
            ul.unlock();
            task();
        }
    }
}

ThreadPool::ThreadPool() : 
    m_threadNum(0),
    m_maxTaskNum(0),
    m_init(false),
    m_taskQueue(),
    m_queueMutex(),
    m_queueNoEmpty(),
    m_queueNoFull(),
    m_threads()
{
}

ThreadPool::~ThreadPool() {
    if (m_init) {
        deinit();
    }
}
}