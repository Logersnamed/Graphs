#include "vertex.h"
#include "canvas.h"

#include <QPainter>

#define INF -1

Vertex::Vertex(QString displayName, int id, int radius, QPointF& pos, QWidget* parent)
    : displayName(displayName), id(id), radius(radius), pos(pos) {

}

static bool contains(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

static QPointF getTextCenterAlign(QFontMetrics fm, QString text) {
    int width = fm.horizontalAdvance(text);
    int height = fm.ascent() - fm.descent();

    return {-width / 2.0f, height / 2.0f};
}

void Vertex::draw(Canvas *canvas, QPainter& painter) {
    qreal distToCenter = QLineF{canvas->screenCenter, pos}.length();
    if (distToCenter - radius - LINE_THICKNESS > canvas->halfScreenDiagonal) {
        return;
    }

    if (isSelected) {
        painter.setPen(Qt::green);
    }

    if (weight > -2) {
        if (contains(canvas->dEndAnim, id)) {
            painter.setBrush(dEndAnimColor);
        }
        else if (id == canvas->dEnd) {
            painter.setBrush(dEndColor);
        }
        else if (id == canvas->dCurrVertex) {
            painter.setBrush(dCurrColor);
        }
        else if (id == canvas->dFirst) {
            painter.setBrush(dFirstColor);
        }
        else if (contains(canvas->dCheckedVertices, id)) {
            painter.setBrush(dChekcedColor);
        }
    }

    painter.drawEllipse(pos, radius, radius);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    canvas->font.setItalic(false);
    painter.setFont(canvas->font);

    QPointF textPos = pos + getTextCenterAlign(painter.fontMetrics(), displayName);

    painter.drawText(textPos, displayName);

    if (weight > -2) {
        painter.setPen(dWeightColor);
        QFont weightFont = canvas->font;
        weightFont.setItalic(true);
        painter.setFont(weightFont);

        QString weightText = weight == INF ? "∞" : QString::number(weight);
        painter.drawText(pos + WEIGHT_TEXT_OFFSET + getTextCenterAlign(painter.fontMetrics(), weightText), weightText);
        painter.setPen(Qt::black);
    }
}
