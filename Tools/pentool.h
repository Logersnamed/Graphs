#ifndef PENTOOL_H
#define PENTOOL_H

#include "tools.h"

// #include "../canvas.h"

#include <QMouseEvent>

class Canvas;

class PenTool : public Tools {
public:
    PenTool(Canvas *canvas);

    QCursor cursor = Qt::PointingHandCursor;

    void onLeftClick(QMouseEvent *event) override;
    QCursor getCursor() override;

protected:
    Canvas *canvas;
};

#endif // PENTOOL_H
