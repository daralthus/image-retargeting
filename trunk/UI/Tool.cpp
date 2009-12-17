#include "Includes.h"
#include "Tool.h"
#include "MainWindow.h"

#include "qaction.h"

Tool::Tool(MainWindow* window) : QObject(window)
{ 
    _window = window;
    _action = NULL;
}

void Tool::initialize(const QString& name, const QIcon& icon, const QCursor& cursor)
{
    _action = new QAction(icon, name, this);
    _action->setCheckable(true);
    connect(_action, SIGNAL(triggered()), this, SLOT(select()));
    _cursor = cursor;
}

void Tool::select()
{
    _window->selectTool(this);
}
