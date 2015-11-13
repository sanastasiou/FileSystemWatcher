
#pragma managed(push, off)

#include "NativeFileSystemWatcher/ConqurrentQueue.h"
#include <queue>
#include <utility>
#include <condition_variable>

namespace Windows
{
namespace File
{
    ConqurrentQueue::ConqurrentQueue() :
        _queue(static_cast<void*>(new std::queue<value_type>())),
        _mutex(static_cast<void*>(new std::mutex())),
        _conditionVariable(static_cast<void*>(new std::condition_variable()))
    {
    }


    ConqurrentQueue::~ConqurrentQueue()
    {
        ::delete static_cast<std::queue<value_type>*>(_queue); _queue                          = nullptr;
        ::delete static_cast<std::mutex*>(_mutex); _mutex                                      = nullptr;
        ::delete static_cast<std::condition_variable*>(_conditionVariable); _conditionVariable = nullptr;
    }

    void ConqurrentQueue::ClearSync()
    {
        auto aQueue = static_cast<std::queue<value_type>*>(_queue);
        while (!aQueue->empty())
        {
            aQueue->pop();
        }
    }

    void ConqurrentQueue::push(value_type const& data)
    {
        auto aQueue = static_cast<std::queue<value_type>*>(_queue);
        auto aMutex = static_cast<std::mutex*>(_mutex);
        auto aConditionVariable = static_cast<std::condition_variable*>(_conditionVariable);
        std::unique_lock<std::mutex> lock(*aMutex);
        aQueue->push(data);
        lock.unlock();
        aConditionVariable->notify_one();
    }

    bool ConqurrentQueue::empty() const
    {
        auto aQueue = static_cast<std::queue<value_type>*>(_queue);
        auto aMutex = static_cast<std::mutex*>(_mutex);
        std::unique_lock<std::mutex> lock(*aMutex);
        return aQueue->empty();
    }

    bool ConqurrentQueue::try_pop(value_type& popped_value)
    {
        auto aQueue = static_cast<std::queue<value_type>*>(_queue);
        auto aMutex = static_cast<std::mutex*>(_mutex);
        std::unique_lock<std::mutex> lock(*aMutex);
        if (aQueue->empty())
        {
            return false;
        }

        popped_value = aQueue->front();
        aQueue->pop();
        return true;
    }

    void ConqurrentQueue::wait_and_pop(value_type& popped_value)
    {
        auto aQueue = static_cast<std::queue<value_type>*>(_queue);
        auto aMutex = static_cast<std::mutex*>(_mutex);
        auto aConditionVariable = static_cast<std::condition_variable*>(_conditionVariable);
        std::unique_lock<std::mutex> lock(*aMutex);
        while (aQueue->empty())
        {
            aConditionVariable->wait(lock);
        }

        popped_value = aQueue->front();
        aQueue->pop();
    }
} // namespace File
} // namespace Windows

#pragma managed(pop)