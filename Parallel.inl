#include "Parallel.h"

namespace IRL
{
    namespace Parallel
    {
        template<class T>
        TaskGroup<T>::TaskGroup()
        {
            _vec.resize(GetWorkersCount());
        }
        template<class T>
        TaskGroup<T>::TaskGroup(unsigned int size)
        {
            _vec.resize(size);
        }
        template<class T>
        void TaskGroup<T>::Resize(unsigned int size)
        {
            _vec.resize(size);
        }
        template<class T>
        int TaskGroup<T>::Count()
        { 
            return (int)_vec.size();
        }
        template<class T>
        T& TaskGroup<T>::operator[](int i)
        {
            return _vec[i];
        }
        template<class T>
        void TaskGroup<T>::SpawnAndSync()
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

        //////////////////////////////////////////////////////////////////////////

        template<class Task, class State, class Index>
        ParallelFor<Task, State, Index>::ParallelFor(Index min, Index max, State state) : TaskGroup<Task>(0)
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
            Index pos = min;
            for (; i < Count() - 1; i++)
            {
                (*this)[i].Set(pos, pos + step, state);
                pos += step;
            }
            (*this)[i].Set(pos, max, state);
        }
    }
}