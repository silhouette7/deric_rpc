#ifndef _DERIC_PROCESS_H_
#define _DERIC_PROCESS_H_

#include <mutex>

namespace deric
{
class DericProcess
{
public:
    static DericProcess& instance();

    int init();
    
    int deinit();
private:
    DericProcess();

    ~DericProcess() = default;

    DericProcess(const DericProcess&) = delete;

    DericProcess& operator=(const DericProcess&) = delete;

    bool m_init;
    std::mutex m_initMutex;
};
}
#endif