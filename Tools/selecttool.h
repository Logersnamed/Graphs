#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "tools.h"

#include <QMouseEvent>

class SelectTool : public Tools {

public:
    SelectTool() {};

    QCursor  cursor = Qt::ArrowCursor;

    void onLeftClick(QMouseEvent *event) override {};
    QCursor getCursor() override {
        return cursor;
    };
};

#endif // SELECTTOOL_H
