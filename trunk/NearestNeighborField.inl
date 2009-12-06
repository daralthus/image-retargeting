#include "NearestNeighborField.h"
#include "Profiler.h"
#include "IO.h"

namespace IRL
{
    const int PatchSize = 7;                    // main parameter of the algorithm
    const int HalfPatchSize = PatchSize / 2;    // handy shortcut
    const int RandomSearchInvAlpha = 2;         // how much to cut each step during random search
    const int RandomSearchLimit = 3;            // how many pixels to examine during random search
    const int SuperPatchSize = 4 * PatchSize;   // how many pixels to process in one sequential step in parallel mode

    //////////////////////////////////////////////////////////////////////////
    // IterationTask implementation

    template<class PixelType>
    NNF<PixelType>::IterationTask::IterationTask() : 
    _queue(NULL), _owner(NULL), _iteration(0), _lock(NULL)
    { }

    template<class PixelType>
    void NNF<PixelType>::IterationTask::Initialize(NNF* owner, 
        Queue<SuperPatch>* queue, int iteration, Mutex* lock)
    {
        _owner = owner;
        _queue = queue;
        _iteration = iteration;
        _lock = lock;
    }

    template<class PixelType>
    void NNF<PixelType>::IterationTask::Run()
    {
        while (1)
        {
            SuperPatch* superPatch = _queue->Get();
            if (superPatch == NULL)
                break; // quit signal
            _owner->Iteration(superPatch->Left, superPatch->Top, superPatch->Right, superPatch->Bottom, _iteration);
            _lock->Lock();
            superPatch->Processed = true;
            if ((_iteration % 2) == 0) // direct scan order?
            {
                VisitRightPatch(superPatch->RightNeighbor);
                VisitBottomPatch(superPatch->BottomNeighbor);
                if (superPatch->RightNeighbor == NULL && superPatch->BottomNeighbor == NULL)
                    _queue->Add(NULL); // it was the last patch, send quit signal
            } else
            {
                VisitLeftPatch(superPatch->LeftNeighbor);
                VisitTopPatch(superPatch->TopNeighbor);
                if (superPatch->LeftNeighbor == NULL && superPatch->TopNeighbor == NULL)
                    _queue->Add(NULL); // it was the last patch, send quit signal
            }
            _lock->Unlock();
        }
    }

    template<class PixelType>
    inline void NNF<PixelType>::IterationTask::VisitRightPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->TopNeighbor != NULL && !patch->TopNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType>
    inline void NNF<PixelType>::IterationTask::VisitBottomPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->LeftNeighbor != NULL && !patch->LeftNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType>
    inline void NNF<PixelType>::IterationTask::VisitLeftPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->BottomNeighbor != NULL && !patch->BottomNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType>
    inline void NNF<PixelType>::IterationTask::VisitTopPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->RightNeighbor != NULL && !patch->RightNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // NNF implementation

    template<class PixelType>
    NNF<PixelType>::NNF(const Image<PixelType>& source, const Image<PixelType>& target) :
        Source(source), Target(target), OffsetField(target.Width(), target.Height()), _cache(target.Width(), target.Height())
    {
        _iteration = 0;
        _topLeftSuperPatch = NULL;
        _bottomRightSuperPatch = NULL;

        _sourceRect.Left = HalfPatchSize;
        _sourceRect.Right = Source.Width() - HalfPatchSize;
        _sourceRect.Top = HalfPatchSize;
        _sourceRect.Bottom = Source.Height() - HalfPatchSize;

        _targetRect.Left = HalfPatchSize;
        _targetRect.Right = Target.Width() - HalfPatchSize;
        _targetRect.Top = HalfPatchSize;
        _targetRect.Bottom = Target.Height() - HalfPatchSize;

        _sourceRect1px.Left = HalfPatchSize + 1;
        _sourceRect1px.Right = Source.Width() - HalfPatchSize - 1;
        _sourceRect1px.Top = HalfPatchSize + 1;
        _sourceRect1px.Bottom = Source.Height() - HalfPatchSize - 1;

        _targetRect1px.Left = HalfPatchSize + 1;
        _targetRect1px.Right = Target.Width() - HalfPatchSize - 1;
        _targetRect1px.Top = HalfPatchSize + 1;
        _targetRect1px.Bottom = Target.Height() - HalfPatchSize - 1;

        BuildSuperPatches();
    }

    template<class PixelType>
    void NNF<PixelType>::BuildSuperPatches()
    {
        Tools::Profiler profiler("BuildSuperPatches");

        int w = Target.Width()  / SuperPatchSize + 1;
        int h = Target.Height() / SuperPatchSize + 1;
        _superPatches.reserve(w * h);
        int y = 0;
        while (y < Target.Height())
        {
            w = 0;
            int x = 0;
            int bottomLine = Minimum<int>(y + SuperPatchSize, Target.Height());
            while (x < Target.Width())
            {
                SuperPatch patch;
                patch.Left   = x;
                patch.Top    = y;
                patch.Right  = Minimum<int>(x + SuperPatchSize, Target.Width());
                patch.Bottom = bottomLine;
                _superPatches.push_back(patch);
                x += SuperPatchSize;
                w++;
            }
            y += SuperPatchSize;
        }
        _topLeftSuperPatch = &_superPatches.front();
        _bottomRightSuperPatch = &_superPatches.back();

        // build interdependencies
        for (unsigned int i = 0; i < _superPatches.size(); i++)
        {
            if (_superPatches[i].Left == 0)
                _superPatches[i].LeftNeighbor = NULL;
            if (_superPatches[i].Right != Target.Width())
            {
                _superPatches[i].RightNeighbor = &_superPatches[i+1];
                _superPatches[i+1].LeftNeighbor = &_superPatches[i];
            } else
                _superPatches[i].RightNeighbor = NULL;

            if (_superPatches[i].Top == 0)
                _superPatches[i].TopNeighbor = NULL;
            if (_superPatches[i].Bottom != Target.Height())
            {
                _superPatches[i].BottomNeighbor = &_superPatches[i+w];
                _superPatches[i+w].TopNeighbor = &_superPatches[i];
            } else
                _superPatches[i].BottomNeighbor = NULL;
        }
    }

    template<class PixelType>
    void NNF<PixelType>::RandomFill()
    {
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

        _searchRandom.Seed(0);
        _iteration = 0;
    }

    template<class PixelType>
    void NNF<PixelType>::SmoothFill()
    {
        ASSERT(Source.IsValid());
        ASSERT(OffsetField.IsValid());

        const int32_t w = Source.Width();
        const int32_t h = Source.Height();

        for (int32_t y = 0; y < OffsetField.Height(); y++)
        {
            for (int32_t x = 0; x < OffsetField.Width(); x++)
            {
                int32_t sx = x * w / OffsetField.Width();
                int32_t sy = y * h / OffsetField.Height();

                OffsetField(x, y).x = (uint16_t)(sx - x);
                OffsetField(x, y).y = (uint16_t)(sy - y);
            }
        }

        _searchRandom.Seed(0);
        _iteration = 0;
    }

    template<class PixelType>
    void NNF<PixelType>::Iteration(bool parallel = true)
    {
        Tools::Profiler profiler("Iteration");
        if (!parallel)
            Iteration(0, 0, Target.Width(), Target.Height(), _iteration);
        else
        {
            _superPatchQueue.Reinitialize();
            for (unsigned int i = 0; i < _superPatches.size(); i++)
            {
                _superPatches[i].AddedToQueue = false;
                _superPatches[i].Processed = false;
            }
            if ((_iteration % 2) == 0)
            {
                _topLeftSuperPatch->AddedToQueue = true;
                _superPatchQueue.Add(_topLeftSuperPatch); // direct scan order
            } else
            {
                _bottomRightSuperPatch->AddedToQueue = true;
                _superPatchQueue.Add(_bottomRightSuperPatch); // reverse scan order
            }
            Parallel::TaskGroup<IterationTask> workers;
            for (int i = 0; i < workers.Count(); i++)
                workers[i].Initialize(this, &_superPatchQueue, _iteration, &_lock);
            workers.SpawnAndSync();
        }
        _iteration++;
    }

    template<class PixelType>
    void NNF<PixelType>::Save(const std::string& path)
    {
        SaveImage(OffsetField, path);
    }

    template<class PixelType>
    void NNF<PixelType>::Iteration(int left, int top, int right, int bottom, int iteration)
    {
        if (iteration == 0)
            PrepareCache(left, top, right, bottom);
        if ((iteration % 2) == 0)
            DirectScanOrder(left, top, right, bottom);
        else
            ReverseScanOrder(left, top, right, bottom);
    }

    template<class PixelType>
    void NNF<PixelType>::PrepareCache(int left, int top, int right, int bottom)
    {
        for (int32_t y = top; y < bottom; y++)
        {
            for (int32_t x = left; x < right; x++)
            {
                const Point32 p(x, y);
                _cache.Pixel(x, y) = Distance<false>(p, p + f(p));
            }
        }
    }

    template<class PixelType>
    void NNF<PixelType>::DirectScanOrder(int left, int top, int right, int bottom)
    {
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

    template<class PixelType>
    void NNF<PixelType>::ReverseScanOrder(int left, int top, int right, int bottom)
    {
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

    template<class PixelType>
    template<int Direction, bool LeftAvailable, bool UpAvailable>
    void NNF<PixelType>::Propagate(const Point32& target)
    {
        // Direction - -1 for direct scan order, +1 for reverse
        // LeftAvailable == true if caller guarantees that CheckX<Direction>(target.x) == true
        // UpAvailable == true if caller guarantees that CheckY<Direction>(target.y) == true

        bool changed   = false;
        Point32 best   = target;
        Point32 source = target + f(target);
        DistanceType bestD = _cache.Pixel(target.x, target.y);
        if (bestD == 0)
            return;

        if (LeftAvailable || CheckX<Direction>(target.x))
        {
            const Point32 pointToTest(target.x + Direction, target.y);
            const Point32 newSource = target + f(pointToTest);
            if (source != newSource && Source.IsValidPixelCoordinates(newSource.x, newSource.y))
            {
                DistanceType distance = MoveDistanceByDx(pointToTest, Direction);
                source = newSource;
                if (distance < bestD)
                {
                    bestD = distance;
                    best = pointToTest;
                    changed = true;
                }
            }
        }

        if (UpAvailable || CheckY<Direction>(target.y) && bestD != 0)
        {
            const Point32 pointToTest(target.x, target.y + Direction);
            const Point32 newSource = target + f(pointToTest);
            if (source != newSource && Source.IsValidPixelCoordinates(newSource.x, newSource.y))
            {
                DistanceType distance = MoveDistanceByDy(pointToTest, Direction);
                source = newSource;
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
            _cache.Pixel(target.x, target.y) = bestD;
        }
    }

    template<class PixelType>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceByDx(const Point32& target, int dx)
    {
        DistanceType distance = _cache.Pixel(target.x, target.y);
        Point32 source = target + f(target);
        bool s = !_sourceRect1px.Contains(source);
        bool t = !_targetRect1px.Contains(target);
        if ( s &&  t) return MoveDistanceByDxImpl<true, true>(dx, target, source, distance);
        if ( s && !t) return MoveDistanceByDxImpl<true, false>(dx, target, source, distance);
        if (!s &&  t) return MoveDistanceByDxImpl<false, true>(dx, target, source, distance);
        if (!s && !t) return MoveDistanceByDxImpl<false, false>(dx, target, source, distance);
        ASSERT(false);
        return 0;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceByDxImpl(int dx, const Point32& target, const Point32& source, DistanceType distance)
    {
        if (dx == -1) return MoveDistanceRight<SourceMirroring, TargetMirroring>(target, source, distance);
        if (dx ==  1) return MoveDistanceLeft<SourceMirroring, TargetMirroring>(target, source, distance);
        ASSERT(false);
        return 0;
    }

    template<class PixelType>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceByDy(const Point32& target, int dy)
    {
        DistanceType distance = _cache.Pixel(target.x, target.y);
        Point32 source = target + f(target);
        bool s = !_sourceRect1px.Contains(source);
        bool t = !_targetRect1px.Contains(target);
        if ( s &&  t) return MoveDistanceByDyImpl<true, true>(dy, target, source, distance);
        if ( s && !t) return MoveDistanceByDyImpl<true, false>(dy, target, source, distance);
        if (!s &&  t) return MoveDistanceByDyImpl<false, true>(dy, target, source, distance);
        if (!s && !t) return MoveDistanceByDyImpl<false, false>(dy, target, source, distance);
        ASSERT(false);
        return 0;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceByDyImpl(int dy, const Point32& target, const Point32& source, DistanceType distance)
    {
        if (dy == -1) return MoveDistanceUp<SourceMirroring, TargetMirroring>(target, source, distance);
        if (dy ==  1) return MoveDistanceDown<SourceMirroring, TargetMirroring>(target, source, distance);
        ASSERT(false);
        return 0;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceRight(const Point32& target, const Point32& source, DistanceType distance)
    {
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            distance -= PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x - HalfPatchSize, source.y + y),
                Target.GetPixel<TargetMirroring>(target.x - HalfPatchSize, target.y + y));
        }
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            distance += PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + HalfPatchSize + 1, source.y + y),
                Target.GetPixel<TargetMirroring>(target.x + HalfPatchSize + 1, target.y + y));
        }
        return distance;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceLeft(const Point32& target, const Point32& source, DistanceType distance)
    {
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            distance -= PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + HalfPatchSize, source.y + y),
                Target.GetPixel<TargetMirroring>(target.x + HalfPatchSize, target.y + y));
        }
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            distance += PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x - HalfPatchSize - 1, source.y + y),
                Target.GetPixel<TargetMirroring>(target.x - HalfPatchSize - 1, target.y + y));
        }
        return distance;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceUp(const Point32& target, const Point32& source, DistanceType distance)
    {
        for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
        {
            distance -= PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + x, source.y - HalfPatchSize),
                Target.GetPixel<TargetMirroring>(target.x + x, target.y - HalfPatchSize));
        }
        for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
        {
            distance += PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + x, source.y + HalfPatchSize + 1),
                Target.GetPixel<TargetMirroring>(target.x + x, target.y + HalfPatchSize + 1));
        }
        return distance;
    }

    template<class PixelType>
    template<bool SourceMirroring, bool TargetMirroring>
    inline typename NNF<PixelType>::DistanceType NNF<PixelType>::MoveDistanceDown(const Point32& target, const Point32& source, DistanceType distance)
    {
        for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
        {
            distance -= PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + x, source.y + HalfPatchSize),
                Target.GetPixel<TargetMirroring>(target.x + x, target.y + HalfPatchSize));
        }
        for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
        {
            distance += PixelType::Distance(
                Source.GetPixel<SourceMirroring>(source.x + x, source.y - HalfPatchSize - 1),
                Target.GetPixel<TargetMirroring>(target.x + x, target.y - HalfPatchSize - 1));
        }
        return distance;
    }

    template<class PixelType>
    inline void NNF<PixelType>::RandomSearch(const Point32& target)
    {
        Point16 offset = f(target);
        DistanceType bestD = _cache.Pixel(target.x, target.y);
        Point32 best(0, 0);
        bool changed = false;
        if (bestD == 0)
            return;

        Point32 min_w = target + offset;

        // uniform random sample
        int32_t Rx = _searchRandom.Uniform<int32_t>(0, Source.Width());
        int32_t Ry = _searchRandom.Uniform<int32_t>(0, Source.Height());
        Point32 w(Rx - min_w.x, Ry - min_w.y);

        int i = 0;
        while (i < RandomSearchLimit)
        {
            if (abs(w.x) < 1 && abs(w.y) < 1)
                break;
            Point32 source = min_w + w;
            DistanceType distance = Distance<true>(target, source, bestD);
            if (distance < bestD)
            {
                bestD = distance;
                best = w;
                changed = true;
                if (bestD == 0)
                    break;
            }
            w.x /= RandomSearchInvAlpha;
            w.y /= RandomSearchInvAlpha;
            i++;
        }

        if (changed)
        {
            f(target) = offset + Point16((int16_t)best.x, (int16_t)best.y);
            _cache.Pixel(target.x, target.y) = bestD;
        }
    }

    template<class PixelType>
    template<bool EarlyTermination>
    typename NNF<PixelType>::DistanceType NNF<PixelType>::Distance(const Point32& targetPatch, const Point32& sourcePatch, DistanceType known = 0)
    {
        bool s = !_sourceRect.Contains(sourcePatch);
        bool t = !_targetRect.Contains(targetPatch);
        if ( s &&  t) return DistanceImpl<EarlyTermination, true, true>(targetPatch, sourcePatch, known);
        if ( s && !t) return DistanceImpl<EarlyTermination, true, false>(targetPatch, sourcePatch, known);
        if (!s &&  t) return DistanceImpl<EarlyTermination, false, true>(targetPatch, sourcePatch, known);
        if (!s && !t) return DistanceImpl<EarlyTermination, false, false>(targetPatch, sourcePatch, known);
        return 0;
    }

    template<class PixelType>
    template<bool EarlyTermination, bool SourceMirroring, bool TargetMirroring>
    typename NNF<PixelType>::DistanceType NNF<PixelType>::DistanceImpl(const Point32& targetPatch, const Point32& sourcePatch, DistanceType known)
    {
        DistanceType distance = 0;
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelType::Distance(
                    Source.GetPixel<SourceMirroring>(sourcePatch.x + x, sourcePatch.y + y),
                    Target.GetPixel<TargetMirroring>(targetPatch.x + x, targetPatch.y + y));
                if (EarlyTermination) 
                {
                    if (distance > known)
                        return distance;
                }
            }
        }
        return distance;
    }
}