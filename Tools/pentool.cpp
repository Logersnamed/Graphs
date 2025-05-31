#include "pentool.h"
#include "../canvas.h"

#include <QMouseEvent>
#include <QDebug>

PenTool::PenTool(Canvas *canvas) : canvas(canvas) {
    cursor = penToolCursor;
}

void PenTool::onLeftClick(QMouseEvent *event) {
    canvas->createVertex(canvas->getTransformedPos(event->pos()), canvas->VERTEX_RADIUS);
}
