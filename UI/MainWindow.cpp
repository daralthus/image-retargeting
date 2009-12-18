#include "Includes.h"
#include "MainWindow.h"

#include "Tools/DragTool.h"
#include "Tools/ZoomTool.h"
#include "Tools/PolygonTool.h"

MainWindow::MainWindow() : _workerThread(NULL)
{
    setupWorkingArea();
    setupActions();
    setupTools();
    setupMenu();
    setupToolbar();
    setupStatusBar();
    resize(600, 400);
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
    _tools << new PolygonTool(this);
}

void MainWindow::setupMenu()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(_openAction);
    fileMenu->addAction(_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Exit"), this, SLOT(close()));

    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    int lastCategory = -1;
    for (int i = 0; i < _tools.size(); ++i)
    {
        if (lastCategory == -1)
            lastCategory = _tools[i]->category();
        if (lastCategory != _tools[i]->category())
            toolsMenu->addSeparator();
        lastCategory = _tools[i]->category();
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
    int lastCategory = -1;
    for (int i = 0; i < _tools.size(); ++i)
    {
        if (_tools[i]->category() != lastCategory)
            fileToolBar->addSeparator();
        lastCategory = _tools[i]->category();
        QAction* action = _tools[i]->action();
        if (action)
            fileToolBar->addAction(action);
    }
}

void MainWindow::setupStatusBar()
{
    _progress = new QProgressBar();
    _progress->setTextVisible(false);
    statusBar()->addPermanentWidget(_progress, 1);
    _progress->setVisible(false);

    _statusIndicator = new QLabel();
    statusBar()->addPermanentWidget(_statusIndicator);
    setBusy(false);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;
    _workingArea->open(QImage(fileName));
    selectTool(_tools[0]);
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

void MainWindow::setBusy(bool busy)
{
    if (busy)
    {
        _statusIndicator->setPixmap(QPixmap(":/images/red.png"));
        _statusIndicator->setToolTip("Busy");
    }
    else
    {
        _statusIndicator->setPixmap(QPixmap(":/images/green.png"));
        _statusIndicator->setToolTip("Ready");
    }
}

void MainWindow::setProgress(bool visible, int current, int total)
{
    _progress->setVisible(visible);
    _progress->setMaximum(total);
    _progress->setValue(current);
}

//////////////////////////////////////////////////////////////////////////

class WorkerThread : public QThread
{
public:
    WorkerThread(MainWindow* parent) : _parent(parent)
    {
    }

protected:
    virtual void run()
    {
        while (1)
        {
            WorkItem* item = NULL;
            _parent->_lock.lock();
            while (_parent->_workItems.empty())
                _parent->_workItemsNotEmpty.wait(&_parent->_lock);
            item = _parent->_workItems.front();
            _parent->_workItems.pop_front();
            _parent->_lock.unlock();
            if (item == NULL)
                break;
            item->execute();
            delete item;
        }
    }

private:
    MainWindow* _parent;
};

void MainWindow::enqueueWorkItem(WorkItem* item)
{
    QMutexLocker locker(&_lock);
    _workItems.push_back(item);
    _workItemsNotEmpty.wakeOne();
    if (_workerThread == NULL)
    {
        _workerThread = new WorkerThread(this);
        _workerThread->start();
    }
}