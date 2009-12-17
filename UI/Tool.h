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
    int category() const { return _category; }

    virtual bool mousePressEvent(QMouseEvent*)   { return false; }
    virtual bool mouseReleaseEvent(QMouseEvent*) { return false; }
    virtual bool mouseMoveEvent(QMouseEvent*)    { return false; }
    virtual bool wheelEvent(QWheelEvent*)        { return false; }

    virtual void selected() {}
    virtual void unselected() {}
    virtual void reset() {}

public slots:
    void select();

protected:
    void initialize(const QString& name, const QIcon& icon, const QCursor& cursor);
    void setCategory(int category) { _category = category; }
    MainWindow* mainWindow() const { return _window; }

private:
    MainWindow* _window;
    QAction* _action;
    QCursor _cursor;
    int _category;
};

class NullTool : public Tool
{
public:
    NullTool(MainWindow* window) : Tool(window)
    { 
        initialize("NULL", QIcon(), QCursor(Qt::ArrowCursor));
    }
};