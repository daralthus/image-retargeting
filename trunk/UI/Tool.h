#pragma once

class MainWindow;

class Tool :
    public QObject
{
    Q_OBJECT

    friend class MainWindow;
public:
    Tool(MainWindow* window);

    QAction* action() const { return _action; }
    const QCursor& cursor() { return _cursor; }

    virtual bool mousePressEvent(QMouseEvent*)   { return false; }
    virtual bool mouseReleaseEvent(QMouseEvent*) { return false; }
    virtual bool mouseMoveEvent(QMouseEvent*)    { return false; }
    virtual bool wheelEvent(QWheelEvent*)        { return false; }

public slots:
    void select();

protected:
    void initialize(const QString& name, const QIcon& icon, const QCursor& cursor);
    MainWindow* mainWindow() const { return _window; }

    virtual void selected() {}
    virtual void unselected() {}

private:
    MainWindow* _window;
    QAction* _action;
    QCursor _cursor;
};

class NullTool : public Tool
{
public:
    NullTool(MainWindow* window) : Tool(window)
    { 
        initialize("NULL", QIcon(), QCursor(Qt::ArrowCursor));
    }
};