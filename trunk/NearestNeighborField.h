#pragma once

#include "Image.h"
#include "Convert.h"
#include "RGB.h"
#include "Random.h"
#include "TypeTraits.h"
#include "Point2D.h"
#include "IO.h"
#include "Profiler.h"

namespace IRL
{
    template<class PixelType>
    class PixelCache
    {
        typedef typename PixelType::DistanceType DistanceType;
    public:
    };

    template<class PixelType>
    class NearestNeighborField
    {
        typedef typename PixelType::DistanceType DistanceType;

    public:
        NearestNeighborField(const Image<PixelType>& source, const Image<PixelType>& target)
            : Source(source), Target(target), 
            OffsetField(target.Width(), target.Height()), _cache(target.Width(), target.Height())
        {
            _iteration = 0;
            _maxRadius = Source.Width();
            if (Source.Height() > _maxRadius)
                _maxRadius = Source.Height();
        }

        void RandomFill()
        {
            // TODO: make parallel?
            ASSERT(Source.IsValid());
            ASSERT(OffsetField.IsValid());

            Random rnd(0);

            const int32_t w = Source.Width();
            const int32_t h = Source.Height();

            for (int32_t y = 0; y < OffsetField.Height(); y++)
            {
                for (int32_t x = 0; x < OffsetField.Width(); x++)
                {
                    int32_t sx = rnd.Uniform<int32_t>(0, w);
                    int32_t sy = rnd.Uniform<int32_t>(0, h);

                    OffsetField(x, y).x = (uint16_t)(sx - x);
                    OffsetField(x, y).y = (uint16_t)(sy - y);
                }
            }

            memset(_cache.Data(), 0, sizeof(PixelCache<PixelType>) * _cache.Width() * _cache.Height());
            _searchRandom.Seed(0);
        }

        void Iteration()
        {
            _iteration++;
            if ((_iteration % 2) == 1)
                DirectScanOrder();
            else
                ReverseScanOrder();
        }

        void Save(const std::string& path)
        {
            Image<RGB8> vis;
            Convert(vis, OffsetField);
            SaveImage(vis, path);
        }

    private:
        void DirectScanOrder()
        {
            Tools::Profiler profiler("DirectScanOrder");
            for (int32_t x = 1; x < OffsetField.Width(); x++)
            {
                PropagateDownRight(Point32(x, 0));
                RandomSearch(Point32(x, 0));
            }

            for (int32_t y = 1; y < OffsetField.Height(); y++)
            {
                for (int32_t x = 0; x < OffsetField.Width(); x++)
                {
                    PropagateDownRight(Point32(x, y));
                    RandomSearch(Point32(x, y));
                }
            }
        }

        void ReverseScanOrder()
        {
            Tools::Profiler profiler("ReverseScanOrder");
            int32_t y = OffsetField.Height() - 1;
            for (int32_t x = OffsetField.Width() - 2; x >= 0; x--)
            {
                PropagateUpLeft(Point32(x, y));
                RandomSearch(Point32(x, y));
            }

            for (int32_t y = OffsetField.Height() - 2; y >= 0; y--)
            {
                for (int32_t x = OffsetField.Width() - 1; x >= 0; x--)
                {
                    PropagateUpLeft(Point32(x, y));
                    RandomSearch(Point32(x, y));
                }
            }
        }

        void PropagateDownRight(const Point32& target)
        {
            Point32 best   = target;
            Point32 source = target + f(target);
            DistanceType bestD = Distance(target, source);

            if (target.y != 0)
            {
                const Point32 pointToTest(target.x, target.y - 1);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance(target, source);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                    }
                }
            }

            if (target.x != 0)
            {
                const Point32 pointToTest(target.x - 1, target.y);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance(target, source);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                    }
                }
            }

            f(target) = f(best);
        }

        void PropagateUpLeft(const Point32& target)
        {
            Point32 best   = target;
            Point32 source = target + f(target);
            DistanceType bestD = Distance(target, source);

            if (target.y != Target.Height() - 1)
            {
                const Point32 pointToTest(target.x, target.y + 1);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance(target, source);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                    }
                }
            }

            if (target.x != Target.Width() - 1)
            {
                const Point32 pointToTest(target.x + 1, target.y);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance(target, source);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                    }
                }
            }

            f(target) = f(best);
        }

        void RandomSearch(const Point32& target)
        {
            const int32_t InvAlpha = 2;

            int32_t Rx = _searchRandom.Uniform<int32_t>(-65536, 65536);
            int32_t Ry = _searchRandom.Uniform<int32_t>(-65536, 65536);
            
            Point16 w((int16_t)(Rx * _maxRadius / 65536), (int16_t)(Ry * _maxRadius / 65536));
            Point16 offset = f(target);

            Point16 best(0, 0);
            DistanceType bestD = Distance(target, target + offset);

            while (true)
            {
                if (abs(w.x) < 1 && abs(w.y) < 1)
                    break;
                Point32 source = target + offset + w;
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance(target, source);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = w;
                    }
                }
                w.x /= InvAlpha;
                w.y /= InvAlpha;
            }

            f(target) = offset + best;
        }

        Point16& f(const Point32& p)
        {
            return OffsetField(p.x, p.y);
        }

        DistanceType Distance(const Point32& targetPatch, const Point32& sourcePatch)
        {
            const int PatchHalfSize = 3;

            DistanceType distance = 0;
            for (int y = -PatchHalfSize; y < PatchHalfSize; y++)
            {
                for (int x = -PatchHalfSize; x < PatchHalfSize; x++)
                {
                    distance += PixelType::Distance(
                        Source.PixelWithMirroring(sourcePatch.x, sourcePatch.y),
                        Target.PixelWithMirroring(targetPatch.x, targetPatch.y));
                }
            }
            return distance;
        }

    public:
        const Image<PixelType> Source; // B
        const Image<PixelType> Target; // A
        Image<Point16> OffsetField;

    private:
        int32_t _maxRadius;
        Image<PixelCache<PixelType> > _cache;
        Random _searchRandom;
        int _iteration;
    };
}