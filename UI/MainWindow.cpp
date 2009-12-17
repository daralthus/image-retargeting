#include "Includes.h"
#include "MainWindow.h"

#include "Tools/DragTool.h"
#include "Tools/ZoomTool.h"

MainWindow::MainWindow()
{
    setupWorkingArea();
    setupActions();
    setupTools();
    setupMenu();
    setupToolbar();
    setupStatusBar();

    selectTool(_tools[0]);
}

void MainWindow::setupWorkingArea()
{
    _workingArea = new WorkingArea(this);
    setCentralWidget(_workingArea);
}

void MainWindow::setupActions()
{
    _openAction = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    _openAction->setToolTip("Open image file");
    connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

    _saveAction = new QAction(QIcon(":/images/save.png"), tr("&Save as..."), this);
    _saveAction->setToolTip("Save current image");
    connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));
}

void MainWindow::setupTools()
{
    _nullTool = new NullTool(this);
    _currentTool = _nullTool;

    _tools << new DragTool(this);
    _tools << new ZoomTool(this, ZoomTool::ZoomIn);
    _tools << new ZoomTool(this, ZoomTool::ZoomOut);
}

void MainWindow::setupMenu()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(_openAction);
    fileMenu->addAction(_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Exit"), this, SLOT(close()));

    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    for (int i = 0; i < _tools.size(); ++i)
    {
        QAction* action = _tools[i]->action();
        if (action)
            toolsMenu->addAction(action);
    }
}

void MainWindow::setupToolbar()
{
    QToolBar* fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(_openAction);
    fileToolBar->addAction(_saveAction);
    fileToolBar->addSeparator();
    for (int i = 0; i < _tools.size(); ++i)
    {
        QAction* action = _tools[i]->action();
        if (action)
            fileToolBar->addAction(action);
    }
}

void MainWindow::setupStatusBar()
{
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;
    _workingArea->open(QImage(fileName));
}

void MainWindow::save()
{
}

void MainWindow::selectTool(Tool* tool)
{
    if (tool == NULL)
        tool = _nullTool;

    if (tool == _currentTool)
    {
        QAction* action = _currentTool->_action;
        if (action)
            action->setChecked(true);
        return;
    }

    QAction* action;
    action = _currentTool->_action;
    if (action)
        action->setChecked(false);
    _currentTool->unselected();
    _currentTool = tool;
    _currentTool->selected();
    action = _currentTool->_action;
    if (action)
        action->setChecked(true);
    _workingArea->setCursor(_currentTool->cursor());
}