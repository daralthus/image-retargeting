#include "ObjectRemoval.h"
#include "GaussianPyramid.h"
#include "BidirectionalSimilarity.h"
#include "Parameters.h"

#include <direct.h>

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> RemoveObject(const ImageWithMask<PixelType>& img, OperationCallback<PixelType>* callback)
    {
        const int Levels = ceil(log((float)Minimum<int>(img.Image.Width(), img.Image.Height()))) + ObjectRemovalLODBias;

        // calculate Gaussian pyramid for source image and mask
        GaussianPyramid<PixelType> source(img.Image, Levels);
        GaussianPyramid<Alpha8> mask(img.Mask, Levels);

        BidirectionalSimilarity<PixelType, true> solver;

        int progress = 0;
        int total = 0;
        for (int i = Levels - 1; i >= 0; i--)
        {
            for (int j = 0; j < ObjectRemovalMinIterations + ObjectRemovalIterationsLODFactor * i; j++)
            {
                total += 1;
            }
        }

        if (DebugOutput)
            _mkdir("Out/");

        // coarse to fine iteration
        for (int i = Levels - 1; i >= 0; i--)
        {
            std::stringstream debugPath;
            debugPath << "Out/" << i;

            solver.Reset();
            if (DebugOutput)
                solver.DebugPath = debugPath.str();
            solver.Source = source.Levels[i];
            solver.SourceMask = mask.Levels[i];
            solver.NNFIterations = ObjectRemovalMinNNFIterations + i * ObjectRemovalNNFIterationsLODFactor;
            solver.Alpha = ObjectRemovalAlpha;
            if (solver.Target.IsValid())
            {
                solver.Target = MixImages(solver.Source, ScaleUp(solver.Target), solver.SourceMask);
                solver.SourceToTarget = ClampField(ScaleUp(solver.SourceToTarget), solver.Target);
                solver.TargetToSource = ClampField(ScaleUp(solver.TargetToSource), solver.Source);
            } else
            {
                solver.Target = solver.Source; // use existing image
                solver.SourceToTarget = MakeRandomField(solver.Source, solver.Target);
                solver.TargetToSource = MakeRandomField(solver.Target, solver.Source);
            }

            if (DebugOutput)
            {
                _mkdir(debugPath.str().c_str());
                SaveImage(solver.Source, debugPath.str() + "/Source.png");
                SaveImage(solver.Target, debugPath.str() + "/Target.png");
            }

            for (int j = 0; j < ObjectRemovalMinIterations + ObjectRemovalIterationsLODFactor * i; j++)
            {
                solver.Iteration(true);
                progress ++;
                if (!(i == 0 && j == ObjectRemovalMinIterations + ObjectRemovalIterationsLODFactor * i - 1) && callback)
                    callback->IntermediateResult(solver.Target, progress, total);
            }

            if (DebugOutput)
                SaveImage(solver.Target, debugPath.str() + "/Result.png");
        }

        if (callback) callback->OperationEnded(solver.Target);
        return solver.Target; // final image
    }
}