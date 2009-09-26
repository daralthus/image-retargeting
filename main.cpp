#include <iostream>
#include <time.h>

#include "Image.h"

using namespace IRL;

int main(int argc, char** argv)
{
    clock_t start = clock();
    IRL::Image img("Test.jpg");
    img.ChangeColorSpace(Color::Lab);
    img.Save("Test2.jpg");
    std::cout << "Time: " << clock() - start << "\n";
    return 0;
}