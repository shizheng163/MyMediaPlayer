#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include <iostream>
#include "ffmpegutil.h"
int main(int argc, char *argv[])
{
    ffmpegutil::Initialize(false);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
