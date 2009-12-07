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

    private:
        // Initializes algorithm before first iteration
        void Initialize();
        // Updates _s2t
        void UpdateSourceToTargetNNF(bool parallel);
        // Updates _t2s
        void UpdateTargetToSourceNNF(bool parallel);
        // Vote for pixel with weight
        inline void Vote(int32_t tx, int32_t ty, int32_t sx, int32_t sy, double w);

    private:
        // iteration number, start with 0
        int _iteration; 

        // offset fields
        Image<Point16> _s2t; // SourceToTarget[P] = arg min_{Q \in T} D(P,Q), P \in S
        Image<Point16> _t2s; // TargetToSource[Q] = arg min_{P \in S} D(P,Q), Q \in T

        // used in voting
        Image<Accumulator<PixelType, double> > _votes;
    };
}

#include "BidirectionalSimilarity.inl"
