#include "NearestNeighborField.h"
#include "Profiler.h"
#include "IO.h"

namespace IRL
{
    const int RandomSearchInvAlpha = 2;         // how much to cut each step during random search
    const int RandomSearchLimit = 10;           // how many pixels to examine during random search
    const int SuperPatchSize = 2 * PatchSize;   // how many pixels to process in one sequential step in parallel mode

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
        SearchRadius = -1;
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

        ASSERT(Field.IsValid());
        ASSERT(Field.Width() == Target.Width());
        ASSERT(Field.Height() == Target.Height());

        D = DistanceField(Target.Width(), Target.Height());

        _sourceRect.Left = HalfPatchSize;
        _sourceRect.Right = Source.Width() - HalfPatchSize;
        _sourceRect.Top = HalfPatchSize;
        _sourceRect.Bottom = Source.Height() - HalfPatchSize;

        _targetRect.Left = HalfPatchSize;
        _targetRect.Right = Target.Width() - HalfPatchSize;
        _targetRect.Top = HalfPatchSize;
        _targetRect.Bottom = Target.Height() - HalfPatchSize;

        BuildSuperPatches();

        if (SearchRadius < 0)
            SearchRadius = Maximum(Source.Width(), Source.Height());
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
                D.Pixel(x, y).A = Distance<false>(p, p + f(p));
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
        DistanceType bestD = D.Pixel(target.x, target.y).A;
        if (bestD == 0)
            return;

        if (LeftAvailable || CheckX<Direction>(target.x))
        {
            const Point32 pointToTest(target.x + Direction, target.y);
            const Point32 newSource = target + f(pointToTest);
            if (source != newSource && _sourceRect.Contains(newSource))
            {
                DistanceType distance = MoveDistanceByDx<Direction>(pointToTest);
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
                DistanceType distance = MoveDistanceByDy<Direction>(pointToTest);
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
            D.Pixel(target.x, target.y).A = bestD;
        }
    }

    template<class PixelType, bool UseSourceMask>
    template<int Direction>
    typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::MoveDistanceByDx(const Point32& target)
    {
        DistanceType distance = D.Pixel(target.x, target.y).A;
        Point32 source = target + f(target);
        if (Direction == -1)
        {
            // move right
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance += PixelDistance(source.x + HalfPatchSize + 1, source.y + y,
                                          target.x + HalfPatchSize + 1, target.y + y);
            }
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance -= PixelDistance(source.x - HalfPatchSize, source.y + y,
                                          target.x - HalfPatchSize, target.y + y);
            }
            return distance;
        }
        if (Direction ==  1)
        {
            // move left
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance += PixelDistance(source.x - HalfPatchSize - 1, source.y + y,
                                          target.x - HalfPatchSize - 1, target.y + y);
            }
            for (int y = -HalfPatchSize; y <= HalfPatchSize; y++)
            {
                distance -= PixelDistance(source.x + HalfPatchSize, source.y + y,
                                          target.x + HalfPatchSize, target.y + y);
            }
            return distance;
        }
        ASSERT(false);
        return 0;
    }

    template<class PixelType, bool UseSourceMask>
    template<int Direction>
    typename NNF<PixelType, UseSourceMask>::DistanceType 
        NNF<PixelType, UseSourceMask>::MoveDistanceByDy(const Point32& target)
    {
        DistanceType distance = D.Pixel(target.x, target.y).A;
        Point32 source = target + f(target);
        if (Direction == -1)
        {
            // move up
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelDistance(source.x + x, source.y + HalfPatchSize + 1,
                                          target.x + x, target.y + HalfPatchSize + 1);
                            
            }
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance -= PixelDistance(source.x + x, source.y - HalfPatchSize,
                                          target.x + x, target.y - HalfPatchSize);
            }
            return distance;
        }
        if (Direction ==  1)
        {
            // move down
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance += PixelDistance(source.x + x, source.y - HalfPatchSize - 1,
                                          target.x + x, target.y - HalfPatchSize - 1);
            }
            for (int x = -HalfPatchSize; x <= HalfPatchSize; x++)
            {
                distance -= PixelDistance(source.x + x, source.y + HalfPatchSize,
                                          target.x + x, target.y + HalfPatchSize);
            }
            return distance;
        }
        ASSERT(false);
        return 0;
    }

    template<class PixelType, bool UseSourceMask>
    inline void NNF<PixelType, UseSourceMask>::RandomSearch(const Point32& target)
    {
        if (SearchRadius < 2)
            return;

        Point16 offset = f(target);
        DistanceType bestD = D.Pixel(target.x, target.y).A;
        Point32 best(0, 0);
        bool changed = false;
        if (bestD == 0)
            return;
        bestD = bestD / 2; // use randomly searched value only it is significantly better than propagated one

        Point32 min_w = target + offset;

        // uniform random direction

        int32_t Rx = _random.Uniform<int32_t>(-SearchRadius, +SearchRadius);
        int32_t Ry = _random.Uniform<int32_t>(-SearchRadius, +SearchRadius);

        if (Rx + min_w.x <  _sourceRect.Left)   Rx = _sourceRect.Left - min_w.x;
        if (Rx + min_w.x >= _sourceRect.Right)  Rx = _sourceRect.Right - min_w.x - 1;
        if (Ry + min_w.y <  _sourceRect.Top)    Ry = _sourceRect.Top - min_w.y;
        if (Ry + min_w.y >= _sourceRect.Bottom) Ry = _sourceRect.Bottom - min_w.y - 1;

        Point32 w(Rx, Ry);

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
            D.Pixel(target.x, target.y).A = bestD;
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
                return PatchDistanceUpperBound<PixelType>(); // return > maximum possible distance to eliminate that pixel
        }
        return PixelType::Distance(Source(sx, sy), Target(tx, ty));
    }

    template<class PixelType, bool UseSourceMask>
    double NNF<PixelType, UseSourceMask>::GetMeasure()
    {
        double result = 0;
        for (int32_t y = _targetRect.Top; y < _targetRect.Bottom; y++)
        {
            for (int32_t x = _targetRect.Left; x < _targetRect.Right; x++)
            {
                result += D(x, y).A;
            }
        }
        return result / (PatchSize * PatchSize) / _targetRect.Area();
    }
}