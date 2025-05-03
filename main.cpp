#include "mainwindow.h"
#include "canvas.h"
#include "sidebar.h"

#include <QApplication>
#include <QDockWidget>

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

    // SideBar *sidebar = new SideBar();
    // sidebar->setStyleSheet("background-color:  #f9f9f9");
    // window.addDockWidget(Qt::RightDockWidgetArea, sidebar);

    window.show();

    return a.exec();
}
