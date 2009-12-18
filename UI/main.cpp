#include "Includes.h"
#include "MainWindow.h"

#include "../IRL/Parallel.h"

int main(int argc, char** argv)
{
    IRL::Parallel::Initialize(4);
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
