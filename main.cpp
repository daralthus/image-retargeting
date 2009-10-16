#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"

using namespace IRL;

typedef LabDouble Color;

int main(int, char**)
{
    Parallel::Initialize(2);

    Tools::Profiler profiler("main");

    Image<RGB8> source = LoadImage("Test.png");
    NearestNeighborField<RGB8> nnf(source, 400, 400);
    nnf.RandomFill();

    Image<RGB8> vis;
    Convert(vis, nnf.OffsetField);
    SaveImage(vis, "Offsets.png");

    return 0;
}