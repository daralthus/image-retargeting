#include <iostream>
#include <time.h>

#include "Image.h"
#include "Profiler.h"

using namespace IRL;

int main(int argc, char** argv)
{
    Tools::Profiler profiler("main");
    IRL::Image img("Test.jpg");
    img.ChangeColorSpace(Color::Lab);
    img.Save("Test2.jpg");
    return 0;
}