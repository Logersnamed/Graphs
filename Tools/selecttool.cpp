#include "selecttool.h"

#include <QMouseEvent>
#include <QDebug>

SelectTool::SelectTool() {}

void SelectTool::onLeftClick(QMouseEvent *event) {

}

QCursor SelectTool::getCursor() {
    return cursor;
}
