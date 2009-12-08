#include "BidirectionalSimilarity.h"
#include "Profiler.h"

namespace IRL
{
    template<class PixelType>
    BidirectionalSimilarity<PixelType>::BidirectionalSimilarity()
    {
        Alpha = 0.5;
        NNFIterations = 5;

        _iteration = 0;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::Iteration(bool parallel)
    {
        if (_iteration == 0)
            Initialize();

        // calculate offset fields
        UpdateSourceToTargetNNF(parallel);
        UpdateTargetToSourceNNF(parallel);

        //////////////////////////////////////////////////////////////////////////
        // Voting procedure
        
        double Nt = Alpha * 100.0;
        double Ns = (1.0 - Alpha) * 100.0 * (Target.Width() * Target.Height()) / (Source.Width() * Source.Height());

        // clear destination
        _votes.Clear();

        // 1) For each target patch find the most similar source patch.
        //    Colors of pixels in source patch are votes for pixels in target patch.
        {
            Tools::Profiler profiler("VoteTargetToSource");
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
                            Vote(Qc.x + px, Qc.y + py, Pc.x + px, Pc.y + py, Nt);
                        }
                    }
                }
            }
        }

        // 2) For each source patch find the most similar target patch.
        //    Colors of pixels in source patch are votes for pixels in target patch.
        {
            Tools::Profiler profiler("VoteSourceToTarget");
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
                            Vote(Qc.x + px, Qc.y + py, Pc.x + px, Pc.y + py, Ns);
                        }
                    }
                }
            }
        }

        // Collect the result 
        {
            Tools::Profiler profiler("CollectVotes");
            for (int32_t y = 0; y < Target.Height(); y++)
            {
                for (int32_t x = 0; x < Target.Width(); x++)
                {
                    Target(x, y) = _votes(x, y).GetSum();
                }
            }
        }

        std::ostringstream str;
        str << _iteration;
        std::string i = str.str();

        SaveImage(Target, "Out/Target/" + i + ".bmp");
        SaveImage(SourceToTarget,   "Out/S2T/" + i + ".bmp");
        SaveImage(TargetToSource,   "Out/T2S/" + i + ".bmp");

        _iteration++;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::Initialize()
    {
        ASSERT(Source.IsValid());
        ASSERT(Target.IsValid());
        _votes = Votes(Target.Width(), Target.Height());
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::UpdateSourceToTargetNNF(bool parallel)
    {
        Tools::Profiler profiler("SourceToTargetNNF");
        NNF<PixelType, false> s2t;
        s2t.Source = Target;
        s2t.Target = Source;
        s2t.Field  = MakeRandomField(s2t.Target, s2t.Source);
        for (int i = 0; i < NNFIterations; i++)
            s2t.Iteration(parallel);
        SourceToTarget = s2t.Field;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::UpdateTargetToSourceNNF(bool parallel)
    {
        Tools::Profiler profiler("TargetToSourceNNF");
        NNF<PixelType, true> t2s; 
        t2s.Source = Source;
        t2s.SourceMask = SourceMask;
        t2s.Target = Target;
        t2s.Field  = RemoveMaskedOffsets(MakeRandomField(t2s.Target, t2s.Source), SourceMask);
        for (int i = 0; i < NNFIterations; i++)
            t2s.Iteration(parallel);
        TargetToSource = t2s.Field;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::Vote(int32_t tx, int32_t ty, int32_t sx, int32_t sy, double w)
    {
        if (!SourceMask(sx, sy).IsMasked())
            _votes(tx, ty).AppendAndChangeNorm(Source(sx, sy), w);
    }
}