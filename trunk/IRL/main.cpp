#include "Includes.h"
#include "IO.h"
#include "Profiler.h"
#include "GaussianPyramid.h"
#include "NearestNeighborField.h"
#include "ImageWithMask.h"
#include "BidirectionalSimilarity.h"
#include "ObjectRemoval.h"

#include <direct.h>

using namespace IRL;

// can be RGB8 RGB16 RGBFloat RGBDouble Lab8 Lab16 LabFloat LabDouble
typedef RGB8 Color; 

int main(int, char**)
{
    Parallel::Initialize(4);

    _mkdir("Out");

    //Image<Color> img1 = LoadImage<Color>("fly.png");
    //Image<Color> img2 = LoadImage<Color>("res.bmp");

    //NNF<Color, false> s2t;
    //s2t.Source = img1;
    //s2t.Target = img2;
    //s2t.Field = MakeRandomField(img1, img2);
    //for (int i = 0; i < 20; i++)
    //    s2t.Iteration();

    //NNF<Color, false> t2s;
    //t2s.Source = img2;
    //t2s.Target = img1;
    //t2s.Field = MakeRandomField(img2, img1);
    //for (int i = 0; i < 20; i++)
    //    t2s.Iteration();

    //std::cout << s2t.GetMeasure() + t2s.GetMeasure() << "\n";

    clock_t start = clock();
    SaveImage(RemoveObject(LoadImageWithMask<Color>("water3.png")), "Out/Result.png");
    std::cout << "Total time: " << clock() - start << "\n";

    //ImageWithMask<Color> img = LoadImageWithMask<Color>("Arm.png");
    //NNF<Color, true> nnf;
    //nnf.Source = img.Image;
    //nnf.SourceMask = img.Mask;
    //nnf.Target = img.Image;
    //nnf.Field = MakeSmoothField(nnf.Target, nnf.Source);

    //for (int i = 0; i < 5; i++)
    //{
    //    nnf.Iteration(false);
    //    std::cout << nnf.GetMeasure() << "\n";
    //}
    //SaveImage(nnf.Field, "Out/Result.png");

    //Image<Color> test(nnf.Target.Width(), nnf.Target.Height());
    //for (int y = 0; y < test.Height(); y++)
    //{
    //    for (int x = 0; x < test.Width(); x++)
    //    {
    //        test(x, y) = nnf.Source(x + nnf.Field(x, y).x, y + nnf.Field(x, y).y);
    //    }
    //}
    //SaveImage(test, "Out/Result2.png");

    //SaveImage(ScaleUp(ScaleDown(LoadImage<Color>("sync.png"))), "Out/Result.png");

    //Image<Color> img = LoadImage<Color>("checker.png");
    //for (int i = 0; i < 3; i++)
    //{
    //    img = ScaleDown(img);
    //    std::ostringstream s;
    //    s << "Out/S" << i << ".png";
    //    SaveImage(img, s.str());
    //}

    return 0;
}
