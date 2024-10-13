#include "thread_pool.h"

#include <atomic>
#include <iostream>

static int _index;
static std::mutex _mutex;

void hello() {
    _mutex.lock();
    std::cout << "hello " << ++_index << std::endl;
    _mutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

class Test
{
public:
    int add(int i, int j) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return i + j;
    }
};

int main()
{
    deric::ThreadPool::ThreadPoolConfig_s poolConfig;
    poolConfig.maxTaskNum = 10;
    poolConfig.threadNum = 3;

    deric::ThreadPool& pool = deric::ThreadPool::getInstance();
    pool.init(poolConfig);
    std::thread t([&pool, &poolConfig](){return pool.init(poolConfig);});
    t.join();

    Test test;
    auto res = pool.commit([&test](int i, int j){return test.add(i, j);}, 1, 2);

    for(int i = 0; i < 10; ++i) {
        pool.commit(hello);
    }

    std::cout << "get result" << std::endl;
    if (res.valid()) {
        std::cout << res.get() << std::endl;
    }

    pool.deinit();
    pool.init(poolConfig);
}