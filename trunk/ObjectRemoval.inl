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
            solver.DebugPath = debugPath.str();
            solver.Source = source.Levels[i];
            solver.SourceMask = mask.Levels[i];
            solver.NNFIterations = 4 + 2 * i;
            solver.Alpha = 0.5;
            if (solver.Target.IsValid())
            {
                // scale up previous level
                Image<PixelType> generated = ScaleUp(solver.Target);
                // and mix with known mip level
                //Image<PixelType> target = solver.Source;
                //for (int y = 0; y < target.Height(); y++)
                //{
                //    for (int x = 0; x < target.Width(); x++)
                //    {
                //        if (solver.SourceMask(x, y).IsMasked())
                //            target(x, y) = generated(x, y);
                //    }
                //}
                //solver.Target = target;
                solver.Target = generated;
                solver.SourceToTarget = ClampField(ScaleUp(solver.SourceToTarget), solver.Target);
                solver.TargetToSource = ClampField(ScaleUp(solver.TargetToSource), solver.Source);
            } else
            {
                solver.Target = solver.Source; // use existing image
                solver.SourceToTarget = MakeRandomField(solver.Source, solver.Target);
                solver.TargetToSource = MakeRandomField(solver.Target, solver.Source);
            }

            _mkdir(solver.DebugPath.c_str());
            SaveImage(solver.Source, solver.DebugPath + "/Source.png");
            SaveImage(solver.Target, solver.DebugPath + "/Target.png");

            for (int j = 0; j < 4; j++)
            {
                solver.Iteration(true);
            }

            SaveImage(solver.Target, solver.DebugPath + "/Result.png");
        }

        return solver.Target; // final image
    }
}