#include "selecttool.h"
#include "../canvas.h"

#include <QFontMetrics>
#include <QtMath>

SelectTool::SelectTool(Canvas* canvas) : canvas(canvas) {
    cursor = selectToolCursor;
}

void SelectTool::onLeftClick(QMouseEvent *event) {
    QPointF clickPos = canvas->getTransformedPos(event->pos());

    if (event->modifiers() != Qt::ShiftModifier) {
        canvas->deselectAllVertices();
        canvas->selectedEdges.clear();
    }

    Vertex* clickedVertex = canvas->getClickedVertex(clickPos);
    if (clickedVertex) {
        if (!clickedVertex->isSelected) {
            canvas->selectVertex(clickedVertex->id);
        }

        canvas->draggingVertex = clickedVertex;
        canvas->draggingOffset = clickedVertex->pos - clickPos;

        canvas->update();
        return;
    }

    QPointF center = canvas->getAbsoluteCenter();
    canvas->screenCenter = (center - canvas->offset) / canvas->scaleFactor;
    canvas->halfScreenDiagonal = qSqrt(QPointF::dotProduct(center, center)) / canvas->scaleFactor;

    qreal closest = -1;
    int closestId = -1;

    for (const auto& [id, edge] : canvas->edges) {
        Vertex* start = canvas->vertices.at(edge->startId);
        Vertex* end = canvas->vertices.at(edge->endId);
        QLineF edgeLine = {start->pos, end->pos};
        QLineF normal(clickPos, {0, 0});
        normal.setAngle(edgeLine.angle() + 90);

        QFontMetrics fontMetrics(canvas->font);
        QPointF textPos;
        qreal closestDist = edge->distanceToPoint(fontMetrics, &textPos, edgeLine, normal, start, end, clickPos);

        if (closestDist > canvas->EDGE_SELECTION_RANGE) continue;

        if (closest == -1 || closestDist < closest) {
            closest = closestDist;
            closestId = id;
        }
    }

    if (closestId != -1) canvas->selectedEdges.push_back(closestId);

    canvas->update();
}
