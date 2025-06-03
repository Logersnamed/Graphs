#include "utils.h"
#include "vertex.h"
#include "canvas.h"

#include <QPainter>

Vertex::Vertex(QString displayName, int id, int radius, QPointF& pos, QWidget* parent)
    : displayName(displayName), id(id), radius(radius), pos(pos) {
    weight = UNDEFINED;
}

void Vertex::draw(Canvas *canvas, QPainter& painter) {
    qreal distToCenter = QLineF{canvas->getScreenCenter(), pos}.length();
    if (distToCenter - radius - LINE_THICKNESS > canvas->getHalfScreenDiagonal()) {
        return;
    }

    if (isSelected) {
        painter.setPen(Qt::green);
    }

    if (weight > -2) {
        if (canvas->djEndAnimation.find(id) != canvas->djEndAnimation.end()) {
            painter.setBrush(dEndAnimColor);
        }
        else if (id == canvas->djEndVertex) {
            painter.setBrush(dEndColor);
        }
        else if (id == canvas->djCurrentVertex) {
            painter.setBrush(dCurrColor);
        }
        else if (id == canvas->djStartVertex) {
            painter.setBrush(dFirstColor);
        }
        else if (utils::contains(canvas->djCheckedVertices, id)) {
            painter.setBrush(dChekcedColor);
        }
    }

    painter.drawEllipse(pos, radius, radius);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    canvas->font.setItalic(false);
    painter.setFont(canvas->font);

    QPointF textPos = pos + utils::getTextCenterAlign(painter.fontMetrics(), displayName);

    painter.drawText(textPos, displayName);

    if (weight > -2) {
        painter.setPen(dWeightColor);
        QFont weightFont = canvas->font;
        weightFont.setItalic(true);
        painter.setFont(weightFont);

        QString weightText = weight == INF ? "∞" : QString::number(weight);
        painter.drawText(pos + WEIGHT_TEXT_OFFSET + utils::getTextCenterAlign(painter.fontMetrics(), weightText), weightText);
        painter.setPen(Qt::black);
    }
}
