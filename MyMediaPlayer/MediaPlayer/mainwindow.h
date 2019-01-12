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
    void                        processYuv(fileutil::PictureFilePtr pPicture);

    Ui::MainWindow *ui;

    VideoGLWidget               *m_pVideoGLWidget;

    //½âÂë
    ffmpegutil::FFDecoder       *m_pDecoder;
    float                       m_fFrameDuration; //ms
    long                        m_nLastRenderedTime;
};

#endif // MAINWINDOW_H
