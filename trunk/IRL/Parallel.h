#pragma once

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

        //////////////////////////////////////////////////////////////////////////

        // Initialized the lib
        extern void Initialize(unsigned int workers);

        // Maximum allowed workers count in Spawn
        extern unsigned int GetWorkersCount();

        // Invokes targets in parallel. Count should be <= GetWorkersCount().
        // All workers should not be busy, i.e. Sync should be called.
        extern void Spawn(Runnable** targets, unsigned int count);

        // Wait till all workers finish their tasks.
        extern void Sync();

        //////////////////////////////////////////////////////////////////////////

        // Little helper
        template<class T>
        class TaskGroup
        {
        public:
            TaskGroup();
            TaskGroup(unsigned int size);
            void Resize(unsigned int size);
            int Count();
            T& operator[](int i);
            void SpawnAndSync();
        private:
            std::vector<T> _vec;
        };

        //////////////////////////////////////////////////////////////////////////

        template<class Task, class State, class Index = int32_t>
        class ParallelFor : public TaskGroup<Task>
        {
            // Task should have Set(Index start, Index stop, State state) method.

            // disable copy methods
            ParallelFor();
            ParallelFor(const ParallelFor&);
            void operator=(const ParallelFor&);
        public:
            ParallelFor(Index min, Index max, State state);
        };
    }
}

#include "Parallel.inl"