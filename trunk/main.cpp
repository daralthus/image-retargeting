#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"
#include "ImageWithMask.h"
#include "BidirectionalSimilarity.h"

#include <direct.h>

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDouble Lab8 Lab16 LabFloat LabDouble
typedef RGB8 Color; 

int main(int, char**)
{
    Parallel::Initialize(4);

    _mkdir("Out");
    _mkdir("Out/Target");
    _mkdir("Out/S2T");
    _mkdir("Out/T2S");

    Tools::Profiler profiler("main");

    ImageWithMask<Color> img = LoadImageWithMask<Color>("Man.png");

    BidirectionalSimilarity<Color> alg;
    alg.Source = img.Image;
    alg.SourceMask = img.Mask;
    alg.Target = img.Image;

    for (int i = 0; i < 1; i++)
        alg.Iteration(false);

    //ImageWithMask<Color> img = LoadImageWithMask<Color>("Water2.png");
    //NNF<Color, true> nnf;
    //nnf.Source = img.Image;
    //nnf.SourceMask = img.Mask;
    //nnf.Target = img.Image;
    //nnf.InitialOffsetField = RandomField;
    //for (int i = 0; i < 100; i++)
    //{
    //    nnf.Iteration(true);

    //    std::stringstream name;
    //    name << "Out/Result/" << i << ".bmp";
    //    nnf.Save(name.str());
    //}

    return 0;
}
