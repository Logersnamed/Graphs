#ifndef EDGE_H
#define EDGE_H

#include <QWidget>

class Vertex;
class Canvas;

class Edge {

public:
    Edge(QString displayText, int edgeId, int fristId, int secodnId, qreal weight, QWidget* parent = nullptr);

    qreal distanceToPoint(QFontMetrics fontMetrics, QPointF *textPos,
                          QLineF edgeLine, QLineF normal,
                          Vertex* start, Vertex* end,
                          QPointF point);
    void draw(Canvas *canvas, QPainter& painter, bool isForceBoth);

    QString displayText;
    int id;
    int startId;
    int endId;

    const qreal EDGE_TEXT_SHIFT = 15;
    const qreal EDGE_BOTH_SHIFT = 12;
    const qreal ARROW_LENGTH = 13;
    const qreal ARROW_ANGLE = 13;
    const qreal LINE_THICKNESS = 5;
    const QColor dChekcedColor = QColor(255, 180, 162);

    qreal weight;

private:
    void drawArrow(QPainter& painter, QLineF invertedEdgeLine, qreal vertexRadius);
};

#endif // EDGE_H
