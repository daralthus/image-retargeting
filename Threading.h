#pragma once

#include "Config.h"

#ifdef IRL_USE_QT
#include "ThreadingQt.h"
#else
#error Implement threading.
#endif

namespace IRL
{
    class AutoMutex
    {
    public:
        AutoMutex(Mutex& mutex) : _mutex(mutex)
        {
            _mutex.Lock();
        }
        ~AutoMutex()
        {
            _mutex.Unlock();
        }
    private:
        Mutex& _mutex;
    };
}