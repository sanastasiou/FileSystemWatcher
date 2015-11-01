#ifndef WINDOWS_THREADING_THREAD_H__
#define WINDOWS_THREADING_THREAD_H__

#if defined(WINDOWS_THREADING_THREAD_DLL_EXPORTS)
#define WINDOWS_THREADING_THREAD_API __declspec (dllexport)
#else
#define WINDOWS_THREADING_THREAD_API __declspec (dllimport)
#endif

#include "Windows.h"
#include "WindowsBase/Base.h"

namespace Windows
{
namespace Threading
{

class WINDOWS_THREADING_THREAD_API Thread
{
public:
    typedef unsigned int (*Callback)(void*); //!< Callback typedef.

    Thread(Callback callback = 0);

    virtual ~Thread();

    bool Start(void* data);

    bool Stop(unsigned int* result = 0, unsigned int const timeout = INFINITE);

protected:
    //! Main function of the thread, if _Callback is null
    virtual unsigned int DefaultFunc(void * data);

private:
    ::HANDLE _threadHandle;
    Callback _Callback;

    //! Static helper function for the operating system that calls the thread main function of the Thread instance.
    static ::DWORD WINAPI ThreadProc(::LPVOID param);

    struct StateObject
    {
        Thread* thread;
        void* param;
    };
};


} // namespace Threading
} // namespace Windows


#endif // #ifndef WINDOWS_THREADING_THREAD_H__
