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
        class TaskGroup
        {
        public:
            TaskGroup()
            {
                _vec.resize(GetWorkersCount());
            }
            TaskGroup(unsigned int size)
            {
                _vec.resize(size);
            }
            void Resize(unsigned int size)
            {
                _vec.resize(size);
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

        template<class Task, class State>
        class ParallelFor :
            public TaskGroup<Task>
        {
            // Task should have Set(Index start, Index stop, State state) method.

            ParallelFor();
            ParallelFor(const ParallelFor&);
            void operator=(const ParallelFor&);
        public:
            template<class IndexMin, class IndexMax>
            ParallelFor(IndexMin min, IndexMax max, State state) :
              TaskGroup<Task>(0)
            {
                ASSERT(max >= min);
                if (max == min)
                    return;
                unsigned int range = max - min;
                if (range <= GetWorkersCount())
                    Resize(range);
                else
                    Resize(GetWorkersCount());
                int step = (max - min) / Count();
                int i = 0;
                IndexMin pos = min;
                for (; i < Count() - 1; i++)
                {
                    (*this)[i].Set(pos, pos + step, state);
                    pos += step;
                }
                (*this)[i].Set(pos, max, state);
            }
        };
    }
}