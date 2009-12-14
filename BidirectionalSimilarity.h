#pragma once

#include "NearestNeighborField.h"

namespace IRL
{
    template<class PixelType, bool UseSourceMask>
    class BidirectionalSimilarity
    {
    public:
        typedef Image<Alpha<typename PixelType::DistanceType> > DistanceField;

        Image<PixelType> Source;     // source image
        Image<Alpha8>    SourceMask; // importance mask of the source image
        Image<PixelType> Target;     // target image and result of the algorithm

        // offset fields
        OffsetField SourceToTarget;  // SourceToTarget[P] = arg min_{Q \in T} D(P,Q), P \in S
        OffsetField TargetToSource;  // TargetToSource[Q] = arg min_{P \in S} D(P,Q), Q \in T

        double Alpha;                // completeness/coherence importance ratio, default 0.5
        int    NNFIterations;        // how many inner NNF calculation iterations to perform, default 5
        int    SearchRadius;         // random search radius in patch match algorithm

        double Completeness;          // completeness term in dissimilarity measure
        double Coherency;             // coherency term in dissimilarity measure

        std::string DebugPath;        // where to put debug files

    public:
        BidirectionalSimilarity();

        // Make one iteration of the algorithm.
        void Iteration(bool parallel = true);
        // Prepares this object for another run of iterations. Does not change public fields.
        void Reset();

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
