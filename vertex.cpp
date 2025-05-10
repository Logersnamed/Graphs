#include "vertex.h"

#include <QPainter>

Vertex::Vertex(QString displayName, int id, int radius, QPointF& pos, QWidget* parent)
    : displayName(displayName), id(id), radius(radius), pos(pos) {

}

void Vertex::draw(QPainter& painter) {
    painter.drawEllipse(pos, radius, radius);
}
