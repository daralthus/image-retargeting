#pragma once

#include "Threading.h"

namespace IRL
{
    // Producer/consumer queue of T*
    template<class T>
    class Queue
    {
    public:
        void Add(T* value)
        {
            _lock.Lock();
            _queue.push_back(value);
            _lock.Unlock();
            if (value != NULL)
                _queueNotEmpty.WakeOne();
            else
                _queueNotEmpty.WakeAll();
        }

        T* Get()
        {
            AutoMutex autoMutex(_lock);

            while (_queue.empty())
                _queueNotEmpty.Wait(_lock);
            if (_queue.front() == NULL)
                return NULL; // do not pop NULL as it is marker of the end
            else
            {
                T* value = _queue.front();
                _queue.pop_front();
                return value;
            }
        }

        void Reinitialize()
        {
            AutoMutex autoMutex(_lock);
            _queue.clear();
        }

    private:
        Mutex _lock;
        std::list<T*> _queue;
        WaitCondition _queueNotEmpty; // signaled when _queue.empty() == false
    };
}