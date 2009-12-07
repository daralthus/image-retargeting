#include "BidirectionalSimilarity.h"
#include "Profiler.h"

namespace IRL
{
    const double Alpha = 0.5;

    template<class PixelType>
    BidirectionalSimilarity<PixelType>::BidirectionalSimilarity()
    {
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
        memset(_votes.Data(), 0, sizeof(Accumulator<PixelType, double>) * _votes.Width() * _votes.Height());

        // 1) For each target patch find the most similar source patch.
        //    Colors of pixels in source patch are votes for pixels in target patch.
        {
            Tools::Profiler profiler("VoteTargetToSource");
            for (int32_t y = HalfPatchSize; y < Target.Height() - HalfPatchSize; y++)
            {
                for (int32_t x = HalfPatchSize; x < Target.Width() - HalfPatchSize; x++)
                {
                    Point16 Qc(x, y);
                    Point16 Pc = Qc + _t2s(x, y);

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
                    Point16 Qc = Pc + _s2t(x, y);

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
                    Result(x, y) = _votes(x, y).GetSum();
                }
            }
        }

        std::ostringstream str;
        str << _iteration;
        std::string i = str.str();

        SaveImage(Result, "Out/Result/" + i + ".bmp");
        SaveImage(_s2t,   "Out/S2T/" + i + ".bmp");
        SaveImage(_t2s,   "Out/T2S/" + i + ".bmp");

        _iteration++;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::Initialize()
    {
        ASSERT(Source.IsValid());
        ASSERT(Target.IsValid());

        Result = Target;
        _votes = Image<Accumulator<PixelType, double> >(Target.Width(), Target.Height());
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::UpdateSourceToTargetNNF(bool parallel)
    {
        Tools::Profiler profiler("SourceToTargetNNF");
        NNF<PixelType, false> SourceToTarget;
        SourceToTarget.Source = Result;
        SourceToTarget.Target = Source;
        SourceToTarget.InitialOffsetField = RandomField;
        for (int i = 0; i < 5; i++)
            SourceToTarget.Iteration(parallel);
        _s2t = SourceToTarget.OffsetField;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::UpdateTargetToSourceNNF(bool parallel)
    {
        Tools::Profiler profiler("TargetToSourceNNF");
        NNF<PixelType, false> TargetToSource; 
        TargetToSource.Source = Source;
        TargetToSource.SourceMask = SourceMask;
        TargetToSource.Target = Result;
        TargetToSource.InitialOffsetField = RandomField;
        for (int i = 0; i < 5; i++)
            TargetToSource.Iteration(parallel);
        _t2s = TargetToSource.OffsetField;
    }

    template<class PixelType>
    void BidirectionalSimilarity<PixelType>::Vote(int32_t tx, int32_t ty, int32_t sx, int32_t sy, double w)
    {
        _votes(tx, ty).AppendAndChangeNorm(Source(sx, sy), w);
    }
}