#include "WindowsBase/Thread.h"

namespace Windows
{
namespace Threading
{

Thread::Thread(Callback callback) :
    _threadHandle(NULL),
    _callback(callback),
    _threadId(0U)
{
}

Thread::~Thread()
{
    Stop(NULL, 500U);

    if (_threadHandle != NULL)
    {
        ::CloseHandle(_threadHandle);
        _threadHandle = NULL;
    }
}

unsigned int Thread::DefaultFunc(void* /* data */)
{
    return 0;
}

bool Thread::Start(void* data)
{
    if (IsStarted())
    {
        //already started
        return false;
    }

    StateObject * aThreadProcParam = new StateObject( this, data );

    _threadHandle = ::CreateThread(NULL, 0, ThreadProc, aThreadProcParam, 0, &_threadId);
    
    return (_threadHandle != NULL);
}

bool Thread::IsStarted()const
{
    return (_threadHandle != NULL);
}

::DWORD Thread::GetThreadId()const
{
    return _threadId;
}

bool Thread::Stop(unsigned int* result /* = 0 */, unsigned int timeout /* = INFINITE */)
{
    if (_threadHandle == NULL)
    {
        return false;
    }

    bool aSuccess = true;
    ::DWORD waitResult = ::WaitForSingleObject(_threadHandle, timeout);
    if (waitResult == WAIT_OBJECT_0)
    {
        if (result)
        {
            ::DWORD threadResult = 0;
            if (::GetExitCodeThread(_threadHandle, &threadResult))
            {
                *result = threadResult;
            }
            else
            {
                aSuccess = false;
            }
        }
    }
    else
    {
        aSuccess = false;
    }

    if (waitResult != WAIT_TIMEOUT)
    {
        ::CloseHandle(_threadHandle);
        _threadHandle = NULL;
        _threadId = 0U;
    }

    return aSuccess;
}

::DWORD WINAPI Thread::ThreadProc(::LPVOID param)
{
    StateObject * aStateObject = reinterpret_cast<StateObject*>(param);
    ::DWORD result;

    if (aStateObject->_thread->_callback != NULL)
    {
        result = aStateObject->_thread->_callback(aStateObject->_param);
    }
    else
    {
        result = aStateObject->_thread->DefaultFunc(aStateObject->_param);
    }

    ::delete aStateObject;

    return result;
}

} // namespace Threading
} // namespace Windows

