#ifndef TOOLS_H
#define TOOLS_H

#include <QMouseEvent>
#include <QCursor>

class Canvas;

class Tools {

public:
    Tools() {};

    virtual void onLeftClick(QMouseEvent *event) {};

    QCursor getCursor() const { return cursor; }

protected:
    QCursor cursor;
};

#endif // TOOLS_H
