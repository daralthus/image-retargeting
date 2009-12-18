#pragma once

namespace IRL
{
    // Save all intermediate steps
    extern bool DebugOutput;
    // > 0 to start from more detailed lod in object removal 
    extern int ObjectRemovalLODBias;
    // how many bidirection similarity alg. iterations to perform
    extern int ObjectRemovalMinIterations;
    // total iterations for level i = ObjectRemovalMinIterations + ObjectRemovalIterationsLODFactor * i
    extern int ObjectRemovalIterationsLODFactor;
    // how many NNF alg. iterations to perform
    extern int ObjectRemovalMinNNFIterations;
    // total number of NNF iteration = ObjectRemovalMinNNFIterations + ObjectRemovalNNFIterationsLODFactor * i
    extern int ObjectRemovalNNFIterationsLODFactor;
    // weight of the completness term in object removal alg.
    extern double ObjectRemovalAlpha;

    extern void ResetParameters();
}