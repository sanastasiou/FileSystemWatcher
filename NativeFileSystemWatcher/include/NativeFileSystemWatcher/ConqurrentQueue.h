#ifndef WINDOWS_FILE_CONQURRENT_QUEUE_H__
#define WINDOWS_FILE_CONQURRENT_QUEUE_H__

#if defined(WINDOWS_FILE_CONQURRENT_QUEUE_DLL_EXPORTS)
#define WINDOWS_FILE_CONQURRENT_QUEUE_API __declspec (dllexport)
#else
#define WINDOWS_FILE_CONQURRENT_QUEUE_API __declspec (dllimport)
#endif

#pragma managed(push, off)

#include "Windows.h"
#include <utility>
#include <string>

namespace Windows
{
namespace File
{
    /*
    * Class based on excellent example found here : https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
    * Allow n-n consumers/producers to use a single queue.
    */
    class WINDOWS_FILE_CONQURRENT_QUEUE_API ConqurrentQueue
    {
    private:
        void * _queue;             //std::queue<std::pair<std::wstring, ::DWORD> > _queue;
        mutable void * _mutex;     // std::mutex _mutex;
        void * _conditionVariable; // std::condition_variable;
    public:
        ConqurrentQueue();

        virtual ~ConqurrentQueue();

        typedef std::pair<std::wstring, ::DWORD> value_type;

        void ClearSync();

        void push(value_type const& data);

        bool empty() const;

        bool try_pop(value_type& popped_value);

        void wait_and_pop(value_type& popped_value);

    }; // class ConqurrentQueue
} // namespace File
} // namespace Windows

#pragma managed(pop)

#endif //#ifndef WINDOWS_FILE_CONQURRENT_QUEUE_H__