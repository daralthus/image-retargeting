#include "ObjectRemoval.h"
#include "GaussianPyramid.h"
#include "BidirectionalSimilarity.h"

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> RemoveObject(const ImageWithMask<PixelType>& img)
    {
        const int Levels = int(log((float)Minimum<int>(img.Image.Width(), img.Image.Height())));

        // calculate Gaussian pyramid for source image and mask
        GaussianPyramid<PixelType> source(img.Image, Levels);
        GaussianPyramid<Alpha8> mask(img.Mask, Levels);

        BidirectionalSimilarity<PixelType, true> solver;

        // coarse to fine iteration
        for (int i = Levels - 1; i >= 0; i--)
        {
            std::stringstream debugPath;
            debugPath << "Out/" << i;

            solver.Reset();
            //solver.DebugPath = debugPath.str();
            solver.Source = source.Levels[i];
            solver.SourceMask = mask.Levels[i];
            solver.NNFIterations = 4 + i * 2;
            solver.Alpha = 0.5;
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

            _mkdir(debugPath.str().c_str());
            // SaveImage(solver.Source, debugPath.str() + "/Source.png");

            //ImageWithMask<PixelType> target;
            //target.Image = solver.Target;
            //target.Mask  = solver.SourceMask;
            //SaveImage(target, debugPath.str() + "/Target.png");

            for (int j = 0; j < 2 + i; j++)
                solver.Iteration(true);

            //SaveImage(solver.Target, debugPath.str() + "/Result.png");
        }

        return solver.Target; // final image
    }
}