#include "deric_process.h"
#include "deric_process_entry.h"

namespace deric
{
namespace rpc
{
int DericProcessEntry::init() {
    return DericProcess::instance().init();
}

int DericProcessEntry::deinit() {
    return DericProcess::instance().deinit();
}
}
}