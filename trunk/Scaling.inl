#include "Scaling.h"
#include "Parallel.h"
#include "Profiler.h"

namespace IRL
{
    namespace Internal
    {
        template<class PixelType>
        class ScaleDownTask :
            public Parallel::Runnable
        {
        public:
            struct State
            {
                const Image<PixelType>* Src;
                Image<PixelType>* Dst;
            };
            State S;
            int StartPos;
            int StopPos;
        public:
            void Set(int startPos, int stopPos, State s)
            {
                S = s;
                StartPos = startPos;
                StopPos = stopPos;
            }
        };

        template<class Kernel, class PixelType>
        class HScaleDownTask :
            public ScaleDownTask<PixelType>
        {
        public:
            virtual void Run()
            {
                for (int y = StartPos; y < StopPos; y++)
                    ProcessLine(y);
            }

            inline void ProcessLine(int y)
            {
                const int EdgeSize = Kernel::HalfSize() / 2;
                const int Width = S.Dst->Width();
                int x = 0;
                for (; x < EdgeSize; x++)
                    S.Dst->Pixel(x, y) = ProcessLeftEdge(x, y);
                for (; x < Width - EdgeSize; x++)
                    S.Dst->Pixel(x, y) = ProcessMidlePart(x, y);
                for (; x < Width; x++)
                    S.Dst->Pixel(x, y) = ProcessRightEdge(x, y);
            }

            inline const PixelType ProcessLeftEdge(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(abs(2*x + m), 2*y);
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }

            inline const PixelType ProcessMidlePart(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(2*x + m, 2*y);
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }

            inline const PixelType ProcessRightEdge(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                const int maxX = S.Src->Width() - 1;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(maxX - abs(maxX - (2*x + m)), 2 * y);
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }
        };

        template<class Kernel, class PixelType>
        class VScaleDownTask :
            public ScaleDownTask<PixelType>
        {
        public:
            virtual void Run()
            {
                for (int x = StartPos; x < StopPos; x++)
                    ProcessLine(x);
            }

            inline void ProcessLine(int x)
            {
                const int EdgeSize = Kernel::HalfSize() / 2;
                const int Height = S.Dst->Height();
                int y = 0;
                for (; y < EdgeSize; y++)
                    S.Dst->Pixel(x, y) = ProcessUpEdge(x, y);
                for (; y < Height - EdgeSize; y++)
                    S.Dst->Pixel(x, y) = ProcessMidlePart(x, y);
                for (; y < Height; y++)
                    S.Dst->Pixel(x, y) = ProcessDownEdge(x, y);
            }

            inline const PixelType ProcessUpEdge(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(2*x, abs(2*y + m));
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }

            inline const PixelType ProcessMidlePart(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(2*x, 2*y + m);
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }

            inline const PixelType ProcessDownEdge(int x, int y)
            {
                Accumulator<PixelType, typename Kernel::CoefficientType> accum;
                const int maxY = S.Src->Height() - 1;
                for (int m = -Kernel::HalfSize(); m <= Kernel::HalfSize(); m++)
                {
                    const PixelType* src = &S.Src->Pixel(2*x, maxY - abs(maxY - (2*y + m)));
                    accum.Append(*src, Kernel::Value(m));
                }
                return accum.GetSum(Kernel::Sum());
            }
        };

        class Kernel1D5Tap
        {
        public:
            typedef int CoefficientType;

            static const int HalfSize()     { return 2; }
            static const int Sum()          { return 16; }
            static const int Value(int i)
            { 
                static int LookUp[5] = { 1, 4, 6, 4, 1 }; 
                return LookUp[i + HalfSize()];
            }
        };

        template<class PixelType>
        class ScaleUpTask :
            public Parallel::Runnable
        {
        public:
            struct State
            {
                const Image<PixelType>* Src;
                Image<PixelType>* Dst;
            };
            State S;
            int StartPos;
            int StopPos;
        public:
            void Set(int startPos, int stopPos, State s)
            {
                S = s;
                StartPos = startPos;
                StopPos = stopPos;
            }

            virtual void Run()
            {
                for (int y = StartPos; y < StopPos; y++)
                    ProcessLine(y);
            }

            inline void ProcessLine(int y)
            {
                int sy = y / 2;
                for (int x = 0; x < S.Dst->Width(); x++)
                {
                    int sx = x / 2;
                    S.Dst->Pixel(x, y) = S.Src->Pixel(sx, sy);
                }
            }
        };
    }

    template<class PixelType>
    const Image<PixelType> ScaleDown(const Image<PixelType>& src)
    {
        using namespace Internal;

        typedef Kernel1D5Tap Kernel;

        Tools::Profiler profiler("ScaleDown");
        int w = src.Width();
        int h = src.Height();

        Image<PixelType> res(w / 2, h / 2);
        typename ScaleDownTask<PixelType>::State state;
        state.Src = &src;
        state.Dst = &res;

        // horizontal filter
        Parallel::ParallelFor<
            HScaleDownTask<Kernel, PixelType>,
            typename ScaleDownTask<PixelType>::State
        > htasks(0, res.Height(), state);
        htasks.SpawnAndSync();

        // vertical filter
        Parallel::ParallelFor<
            VScaleDownTask<Kernel, PixelType>,
            typename ScaleDownTask<PixelType>::State
        > vtasks(0, res.Width(), state);
        vtasks.SpawnAndSync();

        return res;
    }

    template<class PixelType>
    const Image<PixelType> ScaleUp(const Image<PixelType>& src)
    {
        using namespace Internal;

        Tools::Profiler profiler("ScaleUp");
        int w = src.Width();
        int h = src.Height();

        Image<PixelType> res(w * 2, h * 2);
        typename ScaleUpTask<PixelType>::State state;
        state.Src = &src;
        state.Dst = &res;

        Parallel::ParallelFor<
            ScaleUpTask<PixelType>,
            typename ScaleUpTask<PixelType>::State
        > tasks(0, res.Height(), state);
        tasks.SpawnAndSync();

        return res;
    }
}