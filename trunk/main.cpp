#include <iostream>
#include <time.h>

#include "Image.h"
#include "Profiler.h"
#include "Parallel.h"
#include "GaussianPyramid.h"

using namespace IRL;

int main(int argc, char** argv)
{
    Parallel::Initialize(2);

    Tools::Profiler profiler("main");
    IRL::Image img("Test.png");
    IRL::GaussianPyramid pyr(img, 8);
    pyr.Save("TestPyr.png");

    return 0;
}