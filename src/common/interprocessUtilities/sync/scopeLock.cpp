#include "interprocessUtilities/sync/scopeLock.h"
#include "interprocessUtilities/sync/posixRobustMutex.h"

#include <cerrno>

namespace dawn
{
namespace interprocess
{

template <>
bool ScopeLock<PosixRobustMutex>::lock()
{
    int ret = mutex_.lock();

    if (ret == 0)
    {
        return true;
    }

    if (ret == EOWNERDEAD)
    {
        if (!mutex_.recover())
        {
            return false;
        }
        return true;
    }

    return true;
}

template <>
void ScopeLock<PosixRobustMutex>::unlock()
{
    mutex_.unlock();
}

}  // namespace interprocess
}  // namespace dawn
