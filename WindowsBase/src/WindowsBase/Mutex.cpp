#include "WindowsBase/Mutex.h"

namespace Windows
{
namespace Threading
{

Mutex::Mutex()
{
    ::InitializeCriticalSection(&mMutex);
}

Mutex::~Mutex()
{
    ::DeleteCriticalSection(&mMutex);
}


void Mutex::Lock()
{
    ::EnterCriticalSection(&mMutex);
}


void Mutex::Unlock()
{
    ::LeaveCriticalSection(&mMutex);
}


} // namespace Threading
} // namespace Windows

