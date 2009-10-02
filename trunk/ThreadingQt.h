#pragma once

// Adapters from Qt to custom threading classes

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QThreadStorage>

namespace IRL
{
    class Thread;
    class Mutex;
    class WaitCondition;

    class Thread : 
        private QThread
    {
    public:
        virtual void Run() = 0;
        void Start()
        {
            start();
        }
        void Join()
        {
            wait();
        }
    private:
        virtual void run()
        {
            Run();
        }
    };

    class Mutex :
        private QMutex
    {
        friend class WaitCondition;
    public:
        void Lock()
        {
            lock();
        }
        void Unlock()
        {
            unlock();
        }
    };

    class WaitCondition :
        private QWaitCondition
    {
    public:
        void Wait(Mutex& lock)
        {
            wait(&lock);
        }
        void WakeOne()
        {
            wakeOne();
        }
        void WakeAll()
        {
            wakeAll();
        }
    };

    template<class T>
    class ThreadLocal :
        private QThreadStorage<T*>
    {
    public:
        T Get() const
        {
            T* t = localData();
            if (t)
                return *t;
            else
                return NULL;
        }

        void Set(T t)
        {
            setLocalData(new T(t));
        }

        bool IsSet() const
        {
            return hasLocalData();
        }
    };
}