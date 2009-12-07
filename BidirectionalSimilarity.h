#pragma once

#include "NearestNeighborField.h"

namespace IRL
{
    template<class PixelType>
    class BidirectionalSimilarity
    {
    public:
        BidirectionalSimilarity();

        // Make one iteration of the algorithm.
        void Iteration(bool parallel = true);

    public:
        Image<PixelType> Source;
        Image<Alpha8>    SourceMask;
        Image<PixelType> Target;
        Image<PixelType> Result;

        // offset fields
        OffsetField SourceToTarget; // SourceToTarget[P] = arg min_{Q \in T} D(P,Q), P \in S
        OffsetField TargetToSource; // TargetToSource[Q] = arg min_{P \in S} D(P,Q), Q \in T

    private:
        typedef Image<Accumulator<PixelType, double> > Votes;

        // Initializes algorithm before first iteration
        inline void Initialize();
        // Updates SourceToTarget
        inline void UpdateSourceToTargetNNF(bool parallel);
        // Updates TargetToSource
        inline void UpdateTargetToSourceNNF(bool parallel);
        // Vote for pixel with weight
        force_inline void Vote(int32_t tx, int32_t ty, int32_t sx, int32_t sy, double w);

    private:
        // iteration number, start with 0
        int _iteration; 

        // used in voting
        Votes _votes;
    };
}

#include "BidirectionalSimilarity.inl"
