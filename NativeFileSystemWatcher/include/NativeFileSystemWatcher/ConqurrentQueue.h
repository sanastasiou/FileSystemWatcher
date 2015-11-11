#ifndef WINDOWS_FILE_CONQURRENT_QUEUE_H__
#define WINDOWS_FILE_CONQURRENT_QUEUE_H__

#include <queue>
#pragma unmanaged
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#pragma managed

namespace Windows
{
namespace File
{
    /*
    * Class based on excellent example found here : https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
    * Allow n-n consumers/producers to use a single queue.
    */
    template<typename Data>
        class ConqurrentQueue
        {
        private:
            std::queue<Data> _queue;
            mutable boost::mutex _mutex;
            boost::condition_variable _conditionVariable;
        public:
            typedef typename std::queue<Data>::value_type value_type;

            void ClearSync()
            {
                while (!_queue.empty())
                {
                    _queue.pop();
                }
            }

            void push(Data const& data)
            {
                boost::mutex::scoped_lock lock(_mutex);
                _queue.push(data);
                lock.unlock();
                _conditionVariable.notify_one();
            }

            bool empty() const
            {
                boost::mutex::scoped_lock lock(_mutex);
                return _queue.empty();
            }

            bool try_pop(Data& popped_value)
            {
                boost::mutex::scoped_lock lock(_mutex);
                if (_queue.empty())
                {
                    return false;
                }

                popped_value = _queue.front();
                _queue.pop();
                return true;
            }

            void wait_and_pop(Data& popped_value)
            {
                boost::mutex::scoped_lock lock(_mutex);
                while (_queue.empty())
                {
                    _conditionVariable.wait(lock);
                }

                popped_value = _queue.front();
                _queue.pop();
            }

        }; // class ConqurrentQueue
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_CONQURRENT_QUEUE_H__