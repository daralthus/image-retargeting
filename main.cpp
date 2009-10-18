#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDoulbe Lab8 Lab16 LabFloat LabDouble
typedef LabDouble Color; 

int main(int, char**)
{
    Parallel::Initialize(2);

    Tools::Profiler profiler("main");

    Image<Color> source = LoadImage<Color>("Source.bmp");
    Image<Color> target = LoadImage<Color>("Source.bmp");

    NearestNeighborField<Color> nnf(source, target);
    nnf.RandomFill();
    nnf.Save("Out/0.bmp");
    nnf.Iteration();
    nnf.Save("Out/1.bmp");
    nnf.Iteration();
    nnf.Save("Out/2.bmp");
    nnf.Iteration();
    nnf.Save("Out/3.bmp");

    return 0;
}