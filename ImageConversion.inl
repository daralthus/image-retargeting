#include "ImageConversion.h"
#include "Parallel.h"
#include "Profiler.h"
#include "ColorConversion.h"

namespace IRL
{
    namespace Internal
    {
        template<class ToPixelType, class FromPixelType>
        class ConvertTask :
            public Parallel::Runnable
        {
        public:
            struct State
            {
                const FromPixelType* From;
                ToPixelType* To;
            };

        private:
            const FromPixelType* _start;
            const FromPixelType* _end;
            State _state;
        public:
            void Set(const FromPixelType* start, const FromPixelType* end, const State& state)
            {
                _start = start;
                _end = end;
                _state = state;
            }

            virtual void Run()
            {
                const FromPixelType* fromPtr = _start;
                ToPixelType* toPtr = _state.To + (_start - _state.From);
                while (fromPtr < _end)
                {
                    Convert(*toPtr, *fromPtr);
                    ++fromPtr;
                    ++toPtr;
                }
            }
        };
    }

    template<class ToPixelType, class FromPixelType>
    void Convert(Image<ToPixelType>& to, const Image<FromPixelType>& from)
    {
        Tools::Profiler profiler("ConvertImage");

        using namespace Internal;

        ASSERT(from.IsValid());

        to = Image<ToPixelType>(from.Width(), from.Height());

        const FromPixelType* fromStart = from.Data();
        const FromPixelType* fromEnd   = from.Data() + from.Width() * from.Height();

        typename ConvertTask<ToPixelType, FromPixelType>::State state;
        state.From = fromStart;
        state.To = to.Data();

        Parallel::ParallelFor
            <
            ConvertTask<ToPixelType, FromPixelType>, 
            typename ConvertTask<ToPixelType, FromPixelType>::State,
            const FromPixelType*
            > tasks(fromStart, fromEnd, state);

        tasks.SpawnAndSync();
    }
}