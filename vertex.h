#ifndef VERTEX_H
#define VERTEX_H

#include <QWidget.h>

class Canvas;

class Vertex {

public:
    Vertex(QString displayName, int id, int radius, QPointF& pos, QWidget* parent = nullptr);

    QString displayName;
    int id;
    int radius;
    QPointF pos;
    qreal weight = -2;
    bool isSelected = false;

    void draw(Canvas *canvas, QPainter& painter);

    struct {
        std::vector<int> vertexId;
        std::vector<int> edgeId;
    } in, out;

    const QColor dFirstColor = QColor(255, 228, 212);
    const QColor dChekcedColor = QColor(255, 180, 162);
    const QColor dCurrColor = QColor(229, 152, 155);
    const QColor dEndColor = QColor(181, 131, 141);
    const QColor dWeightColor = QColor(109, 104, 117);
    const QColor dEndAnimColor = QColor(215, 255, 132);

    const QPointF WEIGHT_TEXT_OFFSET = {0, - radius - 15.0};
    const qreal LINE_THICKNESS = 5;
};

#endif // VERTEX_H
