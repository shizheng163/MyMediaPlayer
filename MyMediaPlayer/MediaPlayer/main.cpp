/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
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
