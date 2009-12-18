#include "Includes.h"
#include "Parameters.h"

namespace IRL
{
    bool DebugOutput;
    int ObjectRemovalLODBias;
    int ObjectRemovalMinIterations;
    int ObjectRemovalIterationsLODFactor;
    int ObjectRemovalMinNNFIterations;
    int ObjectRemovalNNFIterationsLODFactor;
    double ObjectRemovalAlpha;

    void ResetParameters()
    {
        DebugOutput = false;
        ObjectRemovalLODBias = 0;
        ObjectRemovalMinIterations = 2;
        ObjectRemovalIterationsLODFactor = 4;
        ObjectRemovalMinNNFIterations = 4;
        ObjectRemovalNNFIterationsLODFactor = 4;
        ObjectRemovalAlpha = 0.5;
    }
}