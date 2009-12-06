#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"
#include "ImageWithMask.h"

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDoulbe Lab8 Lab16 LabFloat LabDouble
typedef RGB8 Color; 

int main(int, char**)
{
    Parallel::Initialize(4);

    Tools::Profiler profiler("main");

    ImageWithMask<Color> image = LoadImageWithMask<Color>("Source.png");

    NNF<Color, true> nnf;

    nnf.Source = image.Image;
    nnf.SourceMask = image.Mask;
    nnf.Target = image.Image;

    for (int i = 0; i < 10; i++)
        nnf.Iteration();

    nnf.Save("Out/NNF.bmp");
    return 0;
}
