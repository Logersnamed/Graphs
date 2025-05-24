#include "edge.h"
#include "canvas.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

Edge::Edge(QString displayText, int edgeId, int fristId, int secodnId, qreal weight, QWidget* parent)
    : displayText(displayText), id(edgeId), startId(fristId), endId(secodnId), weight(weight) {}

static bool contains(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

QPointF getTextCenterAlign(QFontMetrics fm, QString text) {
    int width = fm.horizontalAdvance(text);
    int height = fm.ascent() - fm.descent();

    return {-width / 2.0f, height / 2.0f};
}

QPointF newVector(const QLineF& direction, const qreal length) {
    qreal directionLength = direction.length();
    qreal sin = direction.dy() / directionLength;
    qreal cos = direction.dx() / directionLength;

    return {length * cos, length * sin};
}

QPointF newVector(const QLineF& direction, const QPointF shift) {
    qreal directionLength = direction.length();
    qreal sin = direction.dy() / directionLength;
    qreal cos = direction.dx() / directionLength;

    return {shift.x() * cos, shift.y() * sin};
}

QLineF shiftLine(QLineF line, QLineF direction, qreal shiftValue) {
    QPointF shift = newVector(direction, shiftValue);
    return QLineF{line.p1() + shift, line.p2() + shift};
}

QPointF closestPoint(QLineF line, QLineF normal, QPointF origin) {
    QPointF intersectionPoint;
    line.intersects(normal, &intersectionPoint);

    QPointF closestPoint = intersectionPoint;

    qreal distToStart = QLineF{intersectionPoint, line.p1()}.length();
    qreal distToEnd = QLineF{intersectionPoint, line.p2()}.length();
    qreal edgeLineLength = line.length();

    if (distToStart > edgeLineLength || distToEnd > edgeLineLength) {
        if (QLineF{origin, line.p1()}.length() > QLineF{origin, line.p2()}.length()) {
            return line.p2();
        }
        else {
            return line.p1();
        }
    }

    return closestPoint;
}

QPointF closestPoint(QString text, QPointF textPos, QPointF textCenterOffset, QPointF origin) {
    QLineF inTextLine{textPos - textCenterOffset, origin};
    qreal textRadius = qSqrt(QPointF::dotProduct(textCenterOffset, textCenterOffset));
    if (inTextLine.length() <= textRadius) return origin;
    inTextLine.setLength(textRadius);
    return inTextLine.p2();
}

void Edge::drawArrow(QPainter& painter, QLineF invertedEdgeLine, qreal vertexRadius) {
    QLineF line = invertedEdgeLine;
    qreal distToCircle = sqrt(vertexRadius * vertexRadius - EDGE_BOTH_SHIFT * EDGE_BOTH_SHIFT / 4);
    line.setLength(distToCircle);

    QLineF wing1{line.p2(), invertedEdgeLine.p2()};
    QLineF wing2{line.p2(), invertedEdgeLine.p2()};

    wing1.setLength(ARROW_LENGTH);
    wing2.setLength(ARROW_LENGTH);

    wing1.setAngle(line.angle() + ARROW_ANGLE);
    wing2.setAngle(line.angle() - ARROW_ANGLE);

    painter.setOpacity(1);

    QPainterPath path;
    path.moveTo(wing1.p1());
    path.lineTo(wing1.p2());
    path.lineTo(wing2.p2());
    painter.drawPath(path);
}

qreal Edge::distanceToPoint(QFontMetrics fontMetrics, QPointF *textPos,
                            QLineF edgeLine, QLineF normal,
                            Vertex* start, Vertex* end,
                            QPointF point) {
    QPointF textCenterOffset = getTextCenterAlign(fontMetrics, displayText);
    QPointF shift = newVector(normal, {EDGE_TEXT_SHIFT - textCenterOffset.x(), EDGE_TEXT_SHIFT + textCenterOffset.y()});
    *textPos = edgeLine.center() + textCenterOffset + shift;

    return std::min(QLineF{point, closestPoint(edgeLine, normal, point)}.length(),
                    QLineF{point, closestPoint(displayText, *textPos, textCenterOffset, point)}.length());
}

void Edge::draw(Canvas *canvas, QPainter& painter, bool isForceBoth) {
    Vertex* start = canvas->getVertex(startId);
    Vertex* end = canvas->getVertex(endId);
    QLineF edgeLine = {start->pos, end->pos};
    QLineF normal(canvas->getScreenCenter(), {0, 0});
    normal.setAngle(edgeLine.angle() + 90);

    if (contains(start->in.vertexId, endId) || isForceBoth) {
        edgeLine = shiftLine(edgeLine, normal, EDGE_BOTH_SHIFT / 2);
    }

    QPointF textPos;
    qreal closestDist = distanceToPoint(painter.fontMetrics(), &textPos, edgeLine, normal, start, end, canvas->getScreenCenter());

    if (closestDist - LINE_THICKNESS > canvas->getHalfScreenDiagonal()) return;

    bool isSelected = start->isSelected && end->isSelected || contains(canvas->selectedEdges, id);
    if (isSelected) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
    }
    else if (contains(canvas->djCheckedEdges, id)) {
        painter.setBrush(dChekcedColor);
        painter.setPen(dChekcedColor);
    }

    painter.drawLine(edgeLine);
    drawArrow(painter, {edgeLine.p2(), edgeLine.p1()}, end->radius);

    if (isSelected) {
        painter.setPen(Qt::darkGreen);
    }
    else {
        painter.setPen(Qt::black);
    }

    painter.drawText(textPos, displayText);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
}
