#include "pentool.h"
#include "../canvas.h"

#include <QMouseEvent>
#include <QDebug>

PenTool::PenTool(Canvas *canvas) : canvas(canvas) {}

void PenTool::onLeftClick(QMouseEvent *event) {
    qDebug() << "Pen";

    canvas->createVertex(canvas->getTransformedPos(event->pos()), canvas->VERTEX_RADIUS);
}

QCursor PenTool::getCursor() {
    return cursor;
}
