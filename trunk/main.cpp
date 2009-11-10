#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDoulbe Lab8 Lab16 LabFloat LabDouble
typedef RGB8 Color; 

int main(int, char**)
{
    Parallel::Initialize(2);

    Tools::Profiler profiler("main");

    Image<Color> source = LoadImage<Color>("Source.bmp");
    Image<Color> target = LoadImage<Color>("1Target.bmp");

    NearestNeighborField<Color> nnf(source, target);
    nnf.RandomFill();
    {
        Tools::Profiler profiler("Main");
        for (int i = 0; i < 2; i++)
            nnf.Iteration();
    }
    nnf.Save("Out/Out.bmp");

    return 0;
}
