#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"

using namespace IRL;

typedef LabDouble Color;

int main(int, char**)
{
    Parallel::Initialize(2);

    Tools::Profiler profiler("main");

    // Load into RGB8
    GaussianPyramid<RGB8> pyramid(LoadImage("Test.png"), 5);
    GaussianPyramid<Color> anotherPyramid;

    {
        Tools::Profiler profiler("Converters");

        // Convert RGB -> Color
        Convert(anotherPyramid, pyramid);

        // Convert Color -> RGB
        Convert(pyramid, anotherPyramid);
    }

    // Save
    SaveGaussianPyramid(pyramid, "Test.png");

    return 0;
}