#ifndef TOOLS_H
#define TOOLS_H

#include <QMouseEvent>
#include <QCursor>

class Canvas;

class Tools {
public:
    Tools();

    QCursor cursor;

    virtual void onLeftClick(QMouseEvent *event);
    virtual QCursor getCursor();
};

#endif // TOOLS_H
