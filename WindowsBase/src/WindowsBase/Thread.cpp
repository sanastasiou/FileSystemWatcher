#include "WindowsBase/Thread.h"

namespace Windows
{
namespace Threading
{

Thread::Thread(Callback callback) :
    _threadHandle(NULL),
    _Callback(callback)
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
    if (_threadHandle != NULL)
    {
        //already started
        return false;
    }

    StateObject * aThreadProcParam = new StateObject{ this, data };

    _threadHandle = ::CreateThread(NULL, 0, ThreadProc, aThreadProcParam, 0, NULL);
    
    return (_threadHandle != NULL);
}

bool Thread::Stop(unsigned int* result /* = 0 */, unsigned int timeout /* = INFINITE */)
{
    if (_threadHandle == NULL)
        return false;

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
    }

    return 0;
}

::DWORD WINAPI Thread::ThreadProc(::LPVOID param)
{
    StateObject * tpp = reinterpret_cast<StateObject*>(param);
    ::DWORD result;

    if (tpp->thread->_Callback != NULL)
    {
        result = tpp->thread->_Callback(tpp->param);
    }
    else
    {
        result = tpp->thread->DefaultFunc(tpp->param);
    }

    delete tpp;

    return result;
}

} // namespace Threading
} // namespace Windows
