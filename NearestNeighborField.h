#pragma once

#include "Image.h"
#include "Convert.h"
#include "RGB.h"
#include "Random.h"
#include "TypeTraits.h"
#include "Point2D.h"
#include "IO.h"
#include "Profiler.h"
#include "Rectangle.h"

namespace IRL
{
    const int PatchSize = 7;
    const int HalfPatchSize = PatchSize / 2;
    const int RandomSearchInvAlpha = 2;

    template<class PixelType>
    class PixelCache
    {
        typedef typename PixelType::DistanceType DistanceType;
    public:
        DistanceType Distance;
        bool HasDistance;
    };

    template<class PixelType>
    class NearestNeighborField
    {
        typedef typename PixelType::DistanceType DistanceType;
        typedef PixelCache<PixelType> CacheType;

    public:
        NearestNeighborField(const Image<PixelType>& source, const Image<PixelType>& target)
            : Source(source), Target(target), 
            OffsetField(target.Width(), target.Height()), _cache(target.Width(), target.Height())
        {
            _iteration = 0;
            _maxRadius = Source.Width();
            if (Source.Height() > _maxRadius)
                _maxRadius = Source.Height();

            _sourceRect.Left = HalfPatchSize;
            _sourceRect.Right = Source.Width() - HalfPatchSize;
            _sourceRect.Top = HalfPatchSize;
            _sourceRect.Bottom = Source.Height() - HalfPatchSize;

            _targetRect.Left = HalfPatchSize;
            _targetRect.Right = Target.Width() - HalfPatchSize;
            _targetRect.Top = HalfPatchSize;
            _targetRect.Bottom = Target.Height() - HalfPatchSize;
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
                    int32_t sx = rnd.Uniform<int32_t>(HalfPatchSize, w - HalfPatchSize);
                    int32_t sy = rnd.Uniform<int32_t>(HalfPatchSize, h - HalfPatchSize);

                    OffsetField(x, y).x = (uint16_t)(sx - x);
                    OffsetField(x, y).y = (uint16_t)(sy - y);
                }
            }

            memset(_cache.Data(), 0, sizeof(CacheType) * _cache.Width() * _cache.Height());
            _searchRandom.Seed(0);
        }

        void Iteration()
        {
            // TODO: make parallel
            _iteration++;
            Iteration(0, 0, Target.Width(), Target.Height(), (_iteration % 2) == 1);
        }

        void Save(const std::string& path)
        {
            SaveImage(OffsetField, path);
        }

    private:
        void Iteration(int left, int top, int right, int bottom, bool directScanOrder)
        {
            if (directScanOrder)
                DirectScanOrder(left, top, right, bottom);
            else
                ReverseScanOrder(left, top, right, bottom);
        }

        void DirectScanOrder(int left, int top, int right, int bottom)
        {
            Tools::Profiler profiler("DirectScanOrder");

            // Top left point is special - nowhere to propagate from,
            // so do only random search on it
            if (left == 0 && top == 0)
                RandomSearch(Point32(left, top));

            int startX = left == 0 ? 1 : left;
            int startY = top == 0 ? 1 : top;

            // Top most row is also special - only propagation from left occurs
            if (top == 0)
            {
                for (int32_t px = startX; px < right; px++)
                {
                    Propagate<-1, true, false>(Point32(px, top));
                    RandomSearch(Point32(px, top));
                }
            }

            // Left most column is also an exception
            if (left == 0)
            {
                for (int32_t py = startY; py < bottom; py++)
                {
                    Propagate<-1, false, true>(Point32(left, py));
                    RandomSearch(Point32(left, py));
                }
            }

            // process the rest
            for (int32_t py = startY; py < bottom; py++)
            {
                for (int32_t px = startX; px < right; px++)
                {
                    Propagate<-1, true, true>(Point32(px, py));
                    RandomSearch(Point32(px, py));
                }
            }
        }

        void ReverseScanOrder(int left, int top, int right, int bottom)
        {
            Tools::Profiler profiler("DirectScanOrder");

            // Bottom right point is special - nowhere to propagate from,
            // so do only random search on it
            if (right == Target.Width() && bottom == Target.Height())
                RandomSearch(Point32(right - 1, bottom - 1));

            int startX = (right == Target.Width()) ? right - 2 : right - 1;
            int startY = (bottom == Target.Height()) ? bottom - 2 : bottom - 1;

            // Bottom most row is also special - only propagation from right occurs
            if (bottom == Target.Height())
            {
                for (int32_t px = startX; px >= left; px--)
                {
                    Propagate<+1, true, false>(Point32(px, bottom - 1));
                    RandomSearch(Point32(px, bottom - 1));
                }
            }

            // Right most column is also an exception
            if (right == Target.Width())
            {
                for (int32_t py = startY; py >= top; py--)
                {
                    Propagate<+1, false, true>(Point32(right - 1, py));
                    RandomSearch(Point32(right - 1, py));
                }
            }

            // process the rest
            for (int32_t py = startY; py >= top; py--)
            {
                for (int32_t px = startX; px >= left; px--)
                {
                    Propagate<+1, true, true>(Point32(px, py));
                    RandomSearch(Point32(px, py));
                }
            }
        }

        template<int Direction, bool LeftAvailable, bool UpAvailable>
        void Propagate(const Point32& target)
        {
            // Direction - -1 for direct scan order, +1 for reverse
            // LeftAvailable == true if caller gaurantees that CheckX<Direction>(target.x) == true
            // UpAvailable == true if caller gaurantees that CheckY<Direction>(target.y) == true

            bool changed   = false;
            Point32 best   = target;
            Point32 source = target + f(target);
            DistanceType bestD = LoadDistance(target);

            if (LeftAvailable || CheckX<Direction>(target.x))
            {
                const Point32 pointToTest(target.x + Direction, target.y);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance<true>(target, source, bestD);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                        changed = true;
                    }
                }
            }

            if (UpAvailable || CheckY<Direction>(target.y))
            {
                const Point32 pointToTest(target.x, target.y + Direction);
                source = target + f(pointToTest);
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance<true>(target, source, bestD);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = pointToTest;
                        changed = true;
                    }
                }
            }

            if (changed)
            {
                f(target) = f(best);
                StoreDistance(target, bestD);
            }
        }

        template<int Direction> bool CheckX(int x); // no implementation here
        template<> inline bool CheckX<-1>(int x) { return x > 0; }
        template<> inline bool CheckX<+1>(int x) { return x < Target.Width() - 1; }

        template<int Direction> bool CheckY(int x);
        template<> inline bool CheckY<-1>(int y) { return y > 0; }
        template<> inline bool CheckY<+1>(int y) { return y < Target.Height() - 1; }

        void RandomSearch(const Point32& target)
        {
            // search vector
            int32_t Rx = _searchRandom.Uniform<int32_t>(-65536, 65536);
            int32_t Ry = _searchRandom.Uniform<int32_t>(-65536, 65536);
            
            // normalized search 
            Point16 w((int16_t)(Rx * _maxRadius / 65536), (int16_t)(Ry * _maxRadius / 65536));
            Point16 offset = f(target);

            Point16 best(0, 0);
            DistanceType bestD = LoadDistance(target);
            bool changed = false;

            while (true)
            {
                if (abs(w.x) < 1 && abs(w.y) < 1)
                    break;
                Point32 source = target + offset + w;
                if (Source.IsValidPixelCoordinates(source.x, source.y))
                {
                    DistanceType distance = Distance<true>(target, source, bestD);
                    if (distance < bestD)
                    {
                        bestD = distance;
                        best = w;
                        changed = true;
                    }
                }
                w.x /= RandomSearchInvAlpha;
                w.y /= RandomSearchInvAlpha;
            }

            if (changed)
            {
                f(target) = offset + best;
                StoreDistance(target, bestD);
            }
        }

        template<bool EarlyTermination>
        DistanceType Distance(const Point32& targetPatch, const Point32& sourcePatch, DistanceType known = 0)
        {
            bool s = !_sourceRect.Contains(sourcePatch);
            bool t = !_targetRect.Contains(targetPatch);
            if ( s &&  t) return DistanceImpl<EarlyTermination, true,  true>(targetPatch, sourcePatch, known);
            if ( s && !t) return DistanceImpl<EarlyTermination, true,  false>(targetPatch, sourcePatch, known);
            if (!s &&  t) return DistanceImpl<EarlyTermination, false, true>(targetPatch, sourcePatch, known);
            if (!s && !t) return DistanceImpl<EarlyTermination, false, false>(targetPatch, sourcePatch, known);
            return 0;
        }

        template<bool EarlyTermination, bool SourceMirroring, bool TargetMirroring>
        DistanceType DistanceImpl(const Point32& targetPatch, const Point32& sourcePatch, DistanceType known)
        {
            DistanceType distance = 0;
            for (int y = -HalfPatchSize; y < HalfPatchSize; y++)
            {
                for (int x = -HalfPatchSize; x < HalfPatchSize; x++)
                {
                    distance += PixelType::Distance(
                            GetPixel<SourceMirroring>(Source, sourcePatch),
                            GetPixel<TargetMirroring>(Target, targetPatch));
                }
                // place it in the outer loop to reduce conditions check count
                if (EarlyTermination) 
                {
                    if (distance > known)
                        return distance;
                }
            }
            return distance;
        }

        template<bool Mirroring>
        const PixelType& GetPixel(const Image<PixelType>& from, const Point32& pos) const
        {
            return from.Pixel(pos.x, pos.y);
        }
        template<>
        const PixelType& GetPixel<true>(const Image<PixelType>& from, const Point32& pos) const
        {
            return from.PixelWithMirroring(pos.x, pos.y);
        }

        // handy shortcut
        inline Point16& f(const Point32& p)
        {
            return OffsetField(p.x, p.y);
        }

        // some caching to reduce computations
        inline void StoreDistance(const Point32& p, DistanceType distance)
        {
            CacheType& cache = _cache.Pixel(p.x, p.y);
            cache.Distance = distance;
        }

        inline DistanceType LoadDistance(const Point32& p)
        {
            CacheType& cache = _cache.Pixel(p.x, p.y);
            if (!cache.HasDistance)
            {
                cache.HasDistance = true;
                cache.Distance = Distance<false>(p, p + f(p));
            }
            return cache.Distance;
        }

    public:
        const Image<PixelType> Source; // B
        const Image<PixelType> Target; // A
        Image<Point16> OffsetField;

    private:
        int32_t _maxRadius;
        Image<CacheType> _cache;
        Random _searchRandom;
        int _iteration;

        Rectangle<int32_t> _sourceRect; // rectangle with allowed source patch centers
        Rectangle<int32_t> _targetRect; // rectangle with allowed target patch centers
    };
}