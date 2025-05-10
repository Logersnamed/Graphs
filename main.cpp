#include "mainwindow.h"
#include "canvas.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow window;
    window.setWindowTitle("Graphs");
    window.setMinimumHeight(600);
    window.setMinimumWidth(800);
    window.setStyleSheet("background-color: white");
    window.setStatusBar(nullptr);

    Canvas *canvas = new Canvas(&window);
    window.setCentralWidget(canvas);

    window.show();

    return a.exec();
}
