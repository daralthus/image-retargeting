#pragma once

#include "WorkingArea.h"
#include "Tool.h"
#include "WorkItem.h"

class WorkerThread;

class MainWindow :
    public QMainWindow
{
    Q_OBJECT

    friend class WorkerThread;
public:
    MainWindow();

    WorkingArea* workingArea() const { return _workingArea; }
    Tool* selectedTool() const { return _currentTool; }

    void enqueueWorkItem(WorkItem* item);
    void addToHistory(const QImage& state);
    void clearHistroy();

public slots:
    void open();
    void save();
    void selectTool(Tool* tool);
    void setBusy(bool busy);
    void setProgress(bool visible, int current, int total);
    void back();
    void forward();
    void toggleDebugOutput();

private:
    void setupWorkingArea();
    void setupActions();
    void setupTools();
    void setupMenu();
    void setupToolbar();
    void setupStatusBar();

private:
    WorkingArea* _workingArea;

    QAction* _openAction;
    QAction* _saveAction;
    QAction* _backAction;
    QAction* _forwardAction;
    QAction* _debugOutputAction;

    QList<Tool*> _tools;
    NullTool* _nullTool;
    Tool* _currentTool;

    QMutex _lock;
    QList<WorkItem*> _workItems;
    QWaitCondition _workItemsNotEmpty;
    WorkerThread* _workerThread;

    QLabel* _statusIndicator;
    QProgressBar* _progress;

    QImage _savedInHistory;
};