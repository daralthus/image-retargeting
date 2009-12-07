#include "NearestNeighborField.h"
#include "Profiler.h"
#include "IO.h"

namespace IRL
{
    const int PatchSize = 7;                    // main parameter of the algorithm
    const int HalfPatchSize = PatchSize / 2;    // handy shortcut
    const int RandomSearchInvAlpha = 2;         // how much to cut each step during random search
    const int RandomSearchLimit = 3;            // how many pixels to examine during random search
    const int SuperPatchSize = 8 * PatchSize;   // how many pixels to process in one sequential step in parallel mode

    //////////////////////////////////////////////////////////////////////////
    // IterationTask implementation

    template<class PixelType, bool UseSourceMask>
    NNF<PixelType, UseSourceMask>::IterationTask::IterationTask() : 
    _queue(NULL), _owner(NULL), _iteration(0), _lock(NULL)
    { }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::IterationTask::Initialize(NNF* owner, 
        Queue<SuperPatch>* queue, int iteration, Mutex* lock)
    {
        _owner = owner;
        _queue = queue;
        _iteration = iteration;
        _lock = lock;
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::IterationTask::Run()
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

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::IterationTask::VisitRightPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->TopNeighbor != NULL && !patch->TopNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::IterationTask::VisitBottomPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->LeftNeighbor != NULL && !patch->LeftNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::IterationTask::VisitLeftPatch(SuperPatch* patch)
    {
        if (patch != NULL && !patch->AddedToQueue)
        {
            if (patch->BottomNeighbor != NULL && !patch->BottomNeighbor->Processed)
                return;
            patch->AddedToQueue = true;
            _queue->Add(patch);
        }
    }

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::IterationTask::VisitTopPatch(SuperPatch* patch)
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

    template<class PixelType, bool UseSourceMask>
    NNF<PixelType, UseSourceMask>::NNF()
    {
        InitialOffsetField = SmoothField;
        _iteration = 0;
        _topLeftSuperPatch = NULL;
        _bottomRightSuperPatch = NULL;
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::Initialize()
    {
        ASSERT(Source.IsValid());
        ASSERT(Target.IsValid());

        if (UseSourceMask)
        {
            ASSERT(SourceMask.IsValid());
            ASSERT((Source.Width() == SourceMask.Width() && Source.Height() == SourceMask.Height()));
        } 

        if (InitialOffsetField != PredefinedField)
            OffsetField = Image<Point16>(Target.Width(), Target.Height());
        else
        {
            ASSERT(OffsetField.IsValid());
            ASSERT(OffsetField.Width() == Target.Width());
            ASSERT(OffsetField.Height() == Target.Height());
        }

        _cache = Image<DistanceType>(Target.Width(), Target.Height());

        _sourceRect.Left = HalfPatchSize;
        _sourceRect.Right = Source.Width() - HalfPatchSize;
        _sourceRect.Top = HalfPatchSize;
        _sourceRect.Bottom = Source.Height() - HalfPatchSize;

        _targetRect.Left = HalfPatchSize;
        _targetRect.Right = Target.Width() - HalfPatchSize;
        _targetRect.Top = HalfPatchSize;
        _targetRect.Bottom = Target.Height() - HalfPatchSize;

        BuildSuperPatches();

        switch (InitialOffsetField)
        {
            case RandomField:
                RandomFill();
            break;
            case SmoothField:
                SmoothFill();
            break;
            case PredefinedField:
            break;
            default:
                ASSERT(false);
        }

        _random.Seed(rand()); // TODO: control randomness (to reproduce result)
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::BuildSuperPatches()
    {
        Tools::Profiler profiler("BuildSuperPatches");

        int w = (_targetRect.Right - _targetRect.Left) / SuperPatchSize + 1;
        int h = (_targetRect.Bottom - _targetRect.Top) / SuperPatchSize + 1;
        _superPatches.reserve(w * h);
        int y = _targetRect.Top;
        while (y < _targetRect.Bottom)
        {
            w = 0;
            int x = _targetRect.Left;
            int bottomLine = Minimum<int>(y + SuperPatchSize, _targetRect.Bottom);
            while (x < _targetRect.Right)
            {
                SuperPatch patch;
                patch.Left   = x;
                patch.Top    = y;
                patch.Right  = Minimum<int>(x + SuperPatchSize, _targetRect.Right);
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
            if (_superPatches[i].Left == _targetRect.Left)
                _superPatches[i].LeftNeighbor = NULL;
            if (_superPatches[i].Right != _targetRect.Right)
            {
                _superPatches[i].RightNeighbor = &_superPatches[i+1];
                _superPatches[i+1].LeftNeighbor = &_superPatches[i];
            } else
                _superPatches[i].RightNeighbor = NULL;

            if (_superPatches[i].Top == _targetRect.Top)
                _superPatches[i].TopNeighbor = NULL;
            if (_superPatches[i].Bottom != _targetRect.Bottom)
            {
                _superPatches[i].BottomNeighbor = &_superPatches[i+w];
                _superPatches[i+w].TopNeighbor = &_superPatches[i];
            } else
                _superPatches[i].BottomNeighbor = NULL;
        }
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::RandomFill()
    {
        ASSERT(Source.IsValid());
        ASSERT(OffsetField.IsValid());

        for (int32_t y = _targetRect.Top; y < _targetRect.Bottom; y++)
        {
            for (int32_t x = _targetRect.Left; x < _targetRect.Right; x++)
            {
                int32_t sx = _random.Uniform<int32_t>(_sourceRect.Left, _sourceRect.Right);
                int32_t sy = _random.Uniform<int32_t>(_sourceRect.Top,  _sourceRect.Bottom);

                CheckThatNotMasked(sx, sy);

                OffsetField(x, y).x = (uint16_t)(sx - x);
                OffsetField(x, y).y = (uint16_t)(sy - y);
            }
        }
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::SmoothFill()
    {
        ASSERT(Source.IsValid());
        ASSERT(OffsetField.IsValid());

        int32_t w = Source.Width();
        int32_t h = Source.Height();

        for (int32_t y = _targetRect.Top; y < _targetRect.Bottom; y++)
        {
            for (int32_t x = _targetRect.Left; x < _targetRect.Right; x++)
            {
                int32_t sx = x * w / OffsetField.Width();
                int32_t sy = y * h / OffsetField.Height();
                
                CheckThatNotMasked(sx, sy);

                OffsetField(x, y).x = (uint16_t)(sx - x);
                OffsetField(x, y).y = (uint16_t)(sy - y);
            }
        }
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::CheckThatNotMasked(int32_t& sx, int32_t& sy)
    {
        if (!UseSourceMask)
            return;
        int i = 0;
        while (SourceMask(sx, sy).IsMasked() && i < 3)
        {
            sx = _random.Uniform<int32_t>(_sourceRect.Left, _sourceRect.Right);
            sy = _random.Uniform<int32_t>(_sourceRect.Top,  _sourceRect.Bottom);
            i++;
        }
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::Iteration(bool parallel = true)
    {
        if (_iteration == 0)
            Initialize();

        Tools::Profiler profiler("Iteration");
        if (!parallel)
            Iteration(_targetRect.Left, _targetRect.Top, _targetRect.Right, _targetRect.Bottom, _iteration);
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

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::Save(const std::string& path)
    {
        SaveImage(OffsetField, path);
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::Iteration(int left, int top, int right, int bottom, int iteration)
    {
        if (iteration == 0)
            PrepareCache(left, top, right, bottom);
        if ((iteration % 2) == 0)
            DirectScanOrder(left, top, right, bottom);
        else
            ReverseScanOrder(left, top, right, bottom);
    }

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::PrepareCache(int left, int top, int right, int bottom)
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

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::DirectScanOrder(int left, int top, int right, int bottom)
    {
        // Top left point is special - nowhere to propagate from,
        // so do only random search on it
        if (left == _targetRect.Left && top == _targetRect.Top)
            RandomSearch(Point32(left, top));

        int startX = left;
        if (startX == _targetRect.Left) startX++;

        int startY = top;
        if (startY == _targetRect.Top) startY++;

        // Top most row is also special - only propagation from left occurs
        if (top == _targetRect.Top)
        {
            for (int32_t px = startX; px < right; px++)
            {
                Propagate<-1, true, false>(Point32(px, top));
                RandomSearch(Point32(px, top));
            }
        }

        // Left most column is also an exception
        if (left == _targetRect.Left)
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

    template<class PixelType, bool UseSourceMask>
    void NNF<PixelType, UseSourceMask>::ReverseScanOrder(int left, int top, int right, int bottom)
    {
        // Bottom right point is special - nowhere to propagate from,
        // so do only random search on it
        if (right == _targetRect.Right && bottom == _targetRect.Bottom)
            RandomSearch(Point32(right - 1, bottom - 1));

        int startX = right - 1;
        if (startX == _targetRect.Right - 1) startX--;
        int startY = bottom - 1;
        if (startY == _targetRect.Bottom - 1) startY--;

        // Bottom most row is also special - only propagation from right occurs
        if (bottom == _targetRect.Bottom)
        {
            for (int32_t px = startX; px >= left; px--)
            {
                Propagate<+1, true, false>(Point32(px, bottom - 1));
                RandomSearch(Point32(px, bottom - 1));
            }
        }

        // Right most column is also an exception
        if (right == _targetRect.Right)
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

    template<class PixelType, bool UseSourceMask>
    template<int Direction, bool LeftAvailable, bool UpAvailable>
    void NNF<PixelType, UseSourceMask>::Propagate(const Point32& target)
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
            if (source != newSource && _sourceRect.Contains(newSource))
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
            if (source != newSource && _sourceRect.Contains(newSource))
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

    template<class PixelType, bool UseSourceMask>
    inline typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::MoveDistanceByDx(const Point32& target, int dx)
    {
        DistanceType distance = _cache.Pixel(target.x, target.y);
        Point32 source = target + f(target);
        if (dx == -1)
        {
            // move right
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance -= PixelDistance(
                    source.x - HalfPatchSize, source.y + y,
                    target.x - HalfPatchSize, target.y + y);
            }
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance += PixelDistance(
                    source.x + HalfPatchSize + 1, source.y + y,
                    target.x + HalfPatchSize + 1, target.y + y);
            }
            return distance;
        }
        if (dx ==  1)
        {
            // move left
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance -= PixelDistance(
                    source.x + HalfPatchSize, source.y + y,
                    target.x + HalfPatchSize, target.y + y);
            }
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance += PixelDistance(
                    source.x - HalfPatchSize - 1, source.y + y,
                    target.x - HalfPatchSize - 1, target.y + y);
            }
            return distance;
        }
        ASSERT(false);
        return 0;
    }

    template<class PixelType, bool UseSourceMask>
    inline typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::MoveDistanceByDy(const Point32& target, int dy)
    {
        DistanceType distance = _cache.Pixel(target.x, target.y);
        Point32 source = target + f(target);
        if (dy == -1)
        {
            // move up
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance -= PixelDistance(
                    source.x + x, source.y - HalfPatchSize,
                    target.x + x, target.y - HalfPatchSize);
            }
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelDistance(
                    source.x + x, source.y + HalfPatchSize + 1,
                    target.x + x, target.y + HalfPatchSize + 1);
            }
            return distance;
        }
        if (dy ==  1)
        {
            // move down
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance -= PixelDistance(
                    source.x + x, source.y + HalfPatchSize,
                    target.x + x, target.y + HalfPatchSize);
            }
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelDistance(
                    source.x + x, source.y - HalfPatchSize - 1,
                    target.x + x, target.y - HalfPatchSize - 1);
            }
            return distance;
        }
        ASSERT(false);
        return 0;
    }

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::RandomSearch(const Point32& target)
    {
        Point16 offset = f(target);
        DistanceType bestD = _cache.Pixel(target.x, target.y);
        Point32 best(0, 0);
        bool changed = false;
        if (bestD == 0)
            return;

        Point32 min_w = target + offset;

        // uniform random sample
        int32_t Rx = _random.Uniform<int32_t>(_sourceRect.Left, _sourceRect.Right);
        int32_t Ry = _random.Uniform<int32_t>(_sourceRect.Top, _sourceRect.Bottom);
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

    template<class PixelType, bool UseSourceMask>
    template<bool EarlyTermination>
    typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::Distance(const Point32& targetPatch, const Point32& sourcePatch, DistanceType known = 0)
    {
        ASSERT(_sourceRect.Contains(sourcePatch));
        ASSERT(_targetRect.Contains(targetPatch));

        DistanceType distance = 0;
        for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
        {
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelDistance(
                    sourcePatch.x + x, sourcePatch.y + y,
                    targetPatch.x + x, targetPatch.y + y);
                if (EarlyTermination) 
                {
                    if (distance > known)
                        return distance;
                }
            }
        }
        return distance;
    }

    template<class PixelType, bool UseSourceMask>
    typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::PixelDistance(int sx, int sy, int tx, int ty)
    {
        if (UseSourceMask)
        {
            if (SourceMask(sx, sy).IsMasked())
                return PixelType::DistanceUpperBound() * PatchSize * PatchSize; // return > maximum possible distance to eliminate that pixel
        }

        return PixelType::Distance(Source(sx, sy), Target(tx, ty));
    }
}