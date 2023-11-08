#ifndef _DERIC_PROCESS_ENTRY_H_
#define _DERIC_PROCESS_ENTRY_H_

namespace deric
{
namespace rpc
{
class DericProcessEntry
{
public:
    int init();

    int deinit();
};
}
}
#endif