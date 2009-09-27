#include <iostream>
#include <time.h>

#include "Image.h"
#include "Profiler.h"

using namespace IRL;

int main(int argc, char** argv)
{
    Tools::Profiler profiler("main");
    /*IRL::Image img("Test.jpg");
    img.ChangeColorSpace(Color::Lab);
    img.Save("Test2.jpg");*/

    int result = 0;
    for (unsigned int i = 0; i < 1000000; i++)
    {
        IRL::Color c1 = IRL::Color::FromARGB32(0xff534563 + i);
        IRL::Color c2 = IRL::Color::FromARGB32(0x00112211 - i);
        result += IRL::Color::Distance(c1, c2);
    }
    std::cout << result << "\n";

    return 0;
}