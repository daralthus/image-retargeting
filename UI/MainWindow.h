#pragma once

#include "WorkingArea.h"
#include "Tool.h"

class MainWindow :
    public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

    WorkingArea* workingArea() const { return _workingArea; }
    Tool* selectedTool() const { return _currentTool; }

public slots:
    void open();
    void save();
    void selectTool(Tool* tool);

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

    QList<Tool*> _tools;
    NullTool* _nullTool;
    Tool* _currentTool;
};