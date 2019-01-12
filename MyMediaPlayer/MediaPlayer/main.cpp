/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
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
