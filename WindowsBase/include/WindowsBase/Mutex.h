#ifndef THREAD_MUTEX_H__
#define THREAD_MUTEX_H__

#if defined(WINDOWS_THREADING_MUTEX_DLL_EXPORTS)
#define WINDOWS_THREADING_MUTEX_API __declspec (dllexport)
#else
#define WINDOWS_THREADING_MUTEX_API __declspec (dllimport)
#endif

#include "Windows.h"

namespace Windows
{
namespace Threading
{

class WINDOWS_THREADING_MUTEX_API Mutex
{
public:

    Mutex();

    virtual ~Mutex();

    void Lock();

    void Unlock();

private:
    ::CRITICAL_SECTION mMutex;
};


} // namespace Threading
} // namespace Windows


#endif // #ifndef THREAD_MUTEX_H__
