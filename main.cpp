#include "canvas.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Canvas canvas;
    canvas.setWindowTitle("Graphs");
    canvas.setMinimumHeight(600);
    canvas.setMinimumWidth(800);
    canvas.setStyleSheet("background-color: white");
    canvas.show();

    return a.exec();
}
