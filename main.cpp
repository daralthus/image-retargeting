#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"
#include "ImageWithMask.h"
#include "BidirectionalSimilarity.h"

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDouble Lab8 Lab16 LabFloat LabDouble
typedef RGB8 Color; 

int main(int, char**)
{
    Parallel::Initialize(4);

    Tools::Profiler profiler("main");

    BidirectionalSimilarity<Color> alg;
    alg.Source = LoadImage<Color>("Source.bmp");
    alg.Target = LoadImage<Color>("Target.bmp");

    for (int i = 0; i < 1; i++)
        alg.Iteration(true);

    return 0;
}
