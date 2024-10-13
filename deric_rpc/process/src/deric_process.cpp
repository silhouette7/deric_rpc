#include "deric_process.h"
#include "deric_debug.h"
#include "io_module.h"
#include "thread_pool.h"

namespace deric
{
DericProcess::DericProcess() :
    m_init(false),
    m_initMutex()
{
}

DericProcess& DericProcess::instance() {
    static DericProcess processInstance;
    return processInstance;
}

int DericProcess::init() {
    int res = 0;
    std::lock_guard<std::mutex> guard(m_initMutex);
    if (m_init) {
        DEBUG_INFO("already init");
        return res;
    }

    DEBUG_INFO("init process");

    res = ThreadPool::getInstance().init(ThreadPool::ThreadPoolConfig_s{});
    if (res < 0) {
        DEBUG_ERROR("init thread pool fail");
        return res;
    }

    IoModuleConfig_s ioModuleConfig = {1024};
    res = IoModule::getInstance().init(ioModuleConfig);
    if (res < 0) {
        DEBUG_ERROR("init io module fail");
        return res;
    }

    res = IoModule::getInstance().start();
    if (res < 0) {
        DEBUG_ERROR("start io module fail");
        IoModule::getInstance().deinit();
        return res;
    }

    DEBUG_INFO("process init successfully");
    m_init = true;
    return res;
}

int DericProcess::deinit()
{
    int res = 0;
    std::lock_guard<std::mutex> guard(m_initMutex);
    if (!m_init) {
        DEBUG_INFO("not init yet");
        return res;
    }

    res = IoModule::getInstance().stop();;
    if (res < 0) {
        DEBUG_ERROR("stop io module fail");
        return res;
    }

    res = IoModule::getInstance().deinit();
    if (res < 0) {
        DEBUG_ERROR("deinit io module fail");
        return res;
    }

    res = ThreadPool::getInstance().deinit();
    if (res < 0) {
        DEBUG_ERROR("deinit thread pool fail");
        return res;
    }

    DEBUG_INFO("process deinit successfully");
    m_init = false;
    return res;
}
}