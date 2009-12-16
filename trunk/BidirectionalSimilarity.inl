#include "BidirectionalSimilarity.h"
#include "Profiler.h"
#include "Parallel.h"

//#define DEBUG_BIDIR

namespace IRL
{
    //////////////////////////////////////////////////////////////////////////
    template<class T>
    T GCD(int a, int b)
    {
        if (b > a)
            return GCD<T>(b, a);
        while (b != 0)
        {
            int t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    //////////////////////////////////////////////////////////////////////////

    template<class PixelType, bool UseSourceMask>
    BidirectionalSimilarity<PixelType, UseSourceMask>::BidirectionalSimilarity()
    {
        Alpha = 0.5;
        NNFIterations = 4;
        SearchRadius = -1;

        _iteration = 0;
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::Iteration(bool parallel)
    {
        if (_iteration == 0)
            Initialize();

        _votes.Clear();

        UpdateSourceToTargetNNF(parallel);
        VoteSourceToTarget();
        UpdateTargetToSourceNNF(parallel);
        VoteTargetToSource();
        CollectVotes();
        DebugOutput();

        _iteration++;
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::VoteTargetToSource()
    {
        // 1) For each target patch find the most similar source patch.
        //    Colors of pixels in source patch are votes for pixels in target patch.
        //    (Coherency).
        Tools::Profiler profiler("VoteTargetToSource");
        VoteQuantityType w = (VoteQuantityType)(100 * (1.0 - Alpha) * _wcomplete);
        for (int32_t y = HalfPatchSize; y < Target.Height() - HalfPatchSize; y++)
        {
            for (int32_t x = HalfPatchSize; x < Target.Width() - HalfPatchSize; x++)
            {
                Point16 Qc(x, y);
                Point16 Pc = Qc + TargetToSource(x, y);

                for (int py = -HalfPatchSize; py <= HalfPatchSize; py++)
                {
                    for (int px = -HalfPatchSize; px <= HalfPatchSize; px++)
                    {
                        Vote(Qc.x + px, Qc.y + py, Pc.x + px, Pc.y + py, w);
                    }
                }
            }
        }
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::VoteSourceToTarget()
    {
        // 2) For each source patch find the most similar target patch.
        //    Colors of pixels in source patch are votes for pixels in target patch.
        //    (Completeness).
        Tools::Profiler profiler("VoteSourceToTarget");
        VoteQuantityType w = (VoteQuantityType)(100 * Alpha * _wcoherent);
        for (int32_t y = HalfPatchSize; y < Source.Height() - HalfPatchSize; y++)
        {
            for (int32_t x = HalfPatchSize; x < Source.Width() - HalfPatchSize; x++)
            {
                Point16 Pc(x, y);
                Point16 Qc = Pc + SourceToTarget(x, y);

                for (int py = -HalfPatchSize; py <= HalfPatchSize; py++)
                {
                    for (int px = -HalfPatchSize; px <= HalfPatchSize; px++)
                    {
                        Vote(Qc.x + px, Qc.y + py, Pc.x + px, Pc.y + py, w);
                    }
                }
            }
        }
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::CollectVotes()
    {
        Tools::Profiler profiler("CollectVotes");
        for (int32_t y = 0; y < Target.Height(); y++)
        {
            for (int32_t x = 0; x < Target.Width(); x++)
            {
                if (_votes(x, y).Norm > 0)
                    Target(x, y) = _votes(x, y).GetSum();
            }
        }
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::Reset()
    {
        _iteration = 0;
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::Initialize()
    {
        ASSERT(Source.IsValid());
        ASSERT(Target.IsValid());
        ASSERT(!UseSourceMask || (SourceMask.Width() == Source.Width() && SourceMask.Height() == Source.Height()));
        ASSERT(!SourceToTarget.IsValid() || (SourceToTarget.Width() == Source.Width() && SourceToTarget.Height() == Source.Height()));
        ASSERT(!TargetToSource.IsValid() || (TargetToSource.Width() == Target.Width() && TargetToSource.Height() == Target.Height()));

        _votes = Votes(Target.Width(), Target.Height());

        if (TypeTraits<VoteQuantityType>::IsInteger)
        {
            VoteQuantityType gcd = GCD<VoteQuantityType>(Target.GetPatchesCount(), Source.GetPatchesCount());
            _wcoherent = Target.GetPatchesCount() / gcd;
            _wcomplete = Source.GetPatchesCount() / gcd;
        } else
        {
            _wcoherent = VoteQuantityType(1.0);
            _wcomplete = VoteQuantityType((double)Source.GetPatchesCount() / Target.GetPatchesCount());
        }
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::UpdateSourceToTargetNNF(bool parallel)
    {
        Tools::Profiler profiler("SourceToTargetNNF");
        NNF<PixelType, false> s2t;
        s2t.SearchRadius = SearchRadius;
        s2t.Source = Target;
        s2t.Target = Source;
        if (SourceToTarget.IsValid())
            s2t.Field = SourceToTarget;
        else
            s2t.Field  = MakeRandomField(s2t.Target, s2t.Source);
        for (int i = 0; i < NNFIterations; i++)
            s2t.Iteration(parallel);
        SourceToTarget = s2t.Field;
#ifdef DEBUG_BIDIR
        Completeness = s2t.GetMeasure();
#endif
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::UpdateTargetToSourceNNF(bool parallel)
    {
        Tools::Profiler profiler("TargetToSourceNNF");
        NNF<PixelType, UseSourceMask> t2s; 
        t2s.SearchRadius = SearchRadius;
        t2s.Source = Source;
        if (UseSourceMask)
            t2s.SourceMask = SourceMask;
        t2s.Target = Target;
        if (TargetToSource.IsValid())
            t2s.Field = TargetToSource;
        else
            t2s.Field = MakeRandomField(t2s.Target, t2s.Source);
        if (UseSourceMask)
            t2s.Field = RemoveMaskedOffsets(t2s.Field, SourceMask);
        for (int i = 0; i < NNFIterations; i++)
            t2s.Iteration(parallel);
        TargetToSource = t2s.Field;

#ifdef DEBUG_BIDIR
        Coherency = t2s.GetMeasure();
#endif
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::Vote(int32_t tx, int32_t ty, int32_t sx, int32_t sy, VoteQuantityType w)
    {
        if (!UseSourceMask || !SourceMask(sx, sy).IsMasked())
            _votes(tx, ty).AppendAndChangeNorm(Source(sx, sy), w);
    }

    template<class PixelType, bool UseSourceMask>
    void BidirectionalSimilarity<PixelType, UseSourceMask>::DebugOutput()
    {
#ifdef DEBUG_BIDIR
        std::ostringstream str;
        //str << _iteration << " (" << Completeness + Coherency << ")";
        str << _iteration;
        std::string i = str.str();

        if (!DebugPath.empty())
        {
            _mkdir(DebugPath.c_str());
            _mkdir((DebugPath + "/Target").c_str());
            _mkdir((DebugPath + "/S2T").c_str());
            _mkdir((DebugPath + "/T2S").c_str());

            SaveImage(Target, DebugPath + "/Target/" + i + ".png");
            SaveImage(SourceToTarget, DebugPath + "/S2T/" + i + ".png");
            SaveImage(TargetToSource, DebugPath + "/T2S/" + i + ".png");
        }

        std::cout << "Iteration " << _iteration << " Completness: " << Completeness
            << " + Coherency: " << Coherency << " = " << Completeness + Coherency << "\n";
#endif
    }
}