#ifndef PENTOOL_H
#define PENTOOL_H

#include "tools.h"

#include <QMouseEvent>

class Canvas;

class PenTool : public Tools {
public:
    PenTool(Canvas *canvas);

    void onLeftClick(QMouseEvent *event) override;

protected:
    Canvas *canvas;
    QCursor penToolCursor = Qt::PointingHandCursor;
};

#endif // PENTOOL_H
