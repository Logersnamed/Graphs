#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "tools.h"
#include <QMouseEvent>

class Canvas;

class SelectTool : public Tools {
public:
    SelectTool(Canvas* canvas);

    void onLeftClick(QMouseEvent *event) override;

private:
    Canvas* canvas;
    const QCursor selectToolCursor = Qt::ArrowCursor;
};

#endif // SELECTTOOL_H
