#include "Includes.h"
#include "Parallel.h"
#include "Threading.h"

namespace IRL
{
    namespace Parallel
    {
        class ThreadPool
        {
        public:
            ThreadPool()
            {
                _initialized = false;
                _allReady = false;
            }

            ~ThreadPool()
            {
                if (!_initialized)
                    return;
                ASSERT(_allReady);
                for (unsigned int i = 0; i < _workers.size(); i++)
                {
                    _workers[i]->Stop();
                    delete _workers[i];
                }
            }

            void Initialize(unsigned int workers)
            {
                ASSERT(!_initialized);
                ASSERT(workers > 0);
                _initialized = true;
                _workers.resize(workers - 1);
                for (unsigned int i = 0; i < _workers.size(); i++)
                {
                    _workers[i] = new WorkerThread();
                    _workers[i]->Start();
                }
                _allReady = true;
            }

            unsigned int GetWorkersCount()
            {
                ASSERT(_initialized);
                return _workers.size() + 1;
            }

            void Spawn(Runnable** targets, unsigned int count)
            {
                ASSERT(_allReady);
                if (count == 0)
                    return;
                _allReady = false;
                // run count - 1 in parallel
                unsigned int i = 0;
                if (count > 1)
                {
                    for (i = 0; i < count - 1; i++)
                    {
                        ASSERT(targets[i] != NULL);
                        _workers[i]->RunTask(targets[i]);
                    }
                }
                // and run remaining one right now
                ASSERT(targets[i] != NULL);
                targets[i]->Run();
            }

            void Sync()
            {
                if (_allReady)
                    return;
                for (unsigned int i = 0; i < _workers.size(); i++)
                    _workers[i]->Sync();
                _allReady = true;
            }

        private:
            class WorkerThread :
                public Thread
            {
            public:
                WorkerThread()
                {
                    _ready = true;
                }

                void Stop()
                {
                    _lock.Lock();
                    _commands.push_back(Command(Command::Quit));
                    _lock.Unlock();
                    _hasCommands.WakeOne();
                    Join();
                }

                void Sync()
                {
                    _lock.Lock();
                    while (!_ready)
                        _readyCondition.Wait(_lock);
                    _lock.Unlock();
                }

                void RunTask(Runnable* task)
                {
                    _lock.Lock();
                    ASSERT(_ready);
                    _ready = false;
                    _commands.push_back(Command(Command::Exec, task));
                    _lock.Unlock();
                    _hasCommands.WakeOne();
                }

            private:
                class Command
                {
                public:
                    enum Types
                    {
                        Unknown,
                        Quit,
                        Exec
                    };
                    
                    Command()
                    {
                        Type = Unknown;
                        Task = NULL;
                    }

                    Command(Types type, Runnable* task = NULL)
                    {
                        Type = type;
                        Task = task;
                    }

                public:
                    Types Type;
                    Runnable* Task;
                };

                virtual void Run()
                {
                    while (1)
                    {
                        Command cmd;
                        _lock.Lock();
                        while (_commands.empty())
                            _hasCommands.Wait(_lock);
                        cmd = _commands.front();
                        _commands.pop_front();
                        _lock.Unlock();

                        ASSERT(cmd.Type != Command::Unknown);
                        if (cmd.Type == Command::Quit)
                            break;
                        if (cmd.Type == Command::Exec)
                        {
                            cmd.Task->Run();
                            _lock.Lock();
                            _ready = true;
                            _readyCondition.WakeAll();
                            _lock.Unlock();
                        }
                    }
                }

            private:
                Mutex _lock;
                bool _ready;
                WaitCondition _readyCondition; // _ready == true
                std::list<Command> _commands;
                WaitCondition _hasCommands; // !_commands.empty()
            };

            std::vector<WorkerThread*> _workers;
            bool _initialized;
            bool _allReady;
        };

        ThreadPool g_ThreadPool;

        void Initialize(unsigned int workers)
        {
            g_ThreadPool.Initialize(workers);
        }

        unsigned int GetWorkersCount()
        {
            return g_ThreadPool.GetWorkersCount();
        }

        void Spawn(Runnable** targets, unsigned int count)
        {
            ASSERT(count <= GetWorkersCount());
            return g_ThreadPool.Spawn(targets, count);
        }

        void Sync()
        {
            g_ThreadPool.Sync();
        }
    }
}