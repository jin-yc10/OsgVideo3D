#include <iostream>
#include <QApplication>
#include "qt_mainwindow.h"
using namespace std;

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QTMainWindow* main = new QTMainWindow;
    main->show();
    return app.exec();
}