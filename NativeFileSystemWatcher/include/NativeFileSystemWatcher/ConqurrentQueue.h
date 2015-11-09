#ifndef WINDOWS_FILE_CONQURRENT_QUEUE_H__
#define WINDOWS_FILE_CONQURRENT_QUEUE_H__

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace boost {
namespace detail {
namespace win32 {
    struct _SECURITY_ATTRIBUTES : public ::_SECURITY_ATTRIBUTES {};
} // namespace win32
} // namespace detail
} // namespace boost


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

            void clear()
            {
                boost::mutex::scoped_lock lock(_mutex);
                std::queue<Data> emptyQueue;
                std::swap(_queue, emptyQueue);
                lock.unlock();
                _conditionVariable.notify_all();
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