#pragma once

#include <vector>

namespace IRL
{
    namespace Parallel
    {
        class Runnable
        {
        public:
            virtual ~Runnable() {}
            virtual void Run() = 0;
        };

        // Initialized the lib
        extern void Initialize(unsigned int workers);

        // Maximum allowed workers count in Spawn
        extern unsigned int GetWorkersCount();

        // Invokes targets in parallel. Count should be <= GetWorkersCount().
        // All workers should not be busy, i.e. Sync should be called.
        extern void Spawn(Runnable** targets, unsigned int count);

        // Wait till all workers finish their tasks.
        extern void Sync();

        // Little helper
        template<class T>
        class TaskList
        {
        public:
            TaskList()
            {
                _vec.resize(GetWorkersCount());
            }
            int Count()
            { 
                return (int)_vec.size();
            }
            T& operator[](int i)
            {
                return _vec[i];
            }
            void SpawnAndSync()
            {
                if (!_vec.empty())
                {
                    std::vector<Runnable*> targets(_vec.size());
                    targets.resize(_vec.size());
                    for (unsigned int i = 0; i < _vec.size(); i++)
                        targets[i] = &_vec[i];
                    Spawn(&targets[0], targets.size());
                    Sync();
                }
            }
        private:
            std::vector<T> _vec;
        };
    }
}