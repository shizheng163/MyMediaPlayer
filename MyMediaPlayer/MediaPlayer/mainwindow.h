/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>
#include "videoglwidget.h"


namespace Ui {
class MainWindow;
}

namespace ffmpegutil{
class FFDecoder;
}// namespace ffmpegutil

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void                        SignalBtnEnable(bool);

private:
    void                        slotBtnEnable(bool enable);
    void                        resetControls();
    void                        selectFile();
    void                        playMedia(QString url);
    void                        stopMedia();
    void                        processYuv(fileutil::PictureFilePtr pPicture);
    void                        processDecodeThreadExit(bool bIsOccurExit);
    void                        closeDecoder();

    Ui::MainWindow *ui;

    VideoGLWidget               *m_pVideoGLWidget;

    //解码
    std::mutex                  m_mutexForDecoder;
    ffmpegutil::FFDecoder       *m_pDecoder;
    float                       m_fFrameDuration; //ms
    long                        m_nLastRenderedTime;
};

#endif // MAINWINDOW_H
