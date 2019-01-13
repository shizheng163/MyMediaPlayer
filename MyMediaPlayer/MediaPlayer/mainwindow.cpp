/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <io.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <QDebug>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <assert.h>
#include "logutil.h"
#include "ffdecoder.h"
#include "ffmpegutil.h"
#include "timeutil.h"
using namespace std;
using namespace logutil;
using namespace fileutil;
MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_pVideoGLWidget(NULL)
    ,m_pDecoder(NULL)
    ,m_fFrameDuration(40)
    ,m_nLastRenderedTime(0)
{
    //布局
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setStyleSheet("QMainWindow{background-color:#C9C9C9;}");

    unsigned mainWidth(this->width()), mainHeight(this->height());
    m_pVideoGLWidget = new VideoGLWidget(this);
    m_pVideoGLWidget->move(0, 0);
    m_pVideoGLWidget->resize(mainWidth, mainHeight * 0.85);
    m_pVideoGLWidget->setStyleSheet("background-color:#000000");

    ui->m_pLabProcessBar->move(0, this->height() * 0.85);
    ui->m_pLabProcessBar->resize(m_pVideoGLWidget->width(), this->height()*0.05);
    ui->m_pLabProcessBar->setStyleSheet("background-color:#66CDAA");

    //设置按钮透明
    ui->m_pBtnOpenFile->setFlat(true);
    ui->m_pBtnPlayOrPause->setFlat(true);
    ui->m_pBtnStop->setFlat(true);
    ui->m_pBtnSpeedSlow->setFlat(true);
    ui->m_pBtnSpeedUp->setFlat(true);

    this->resetControls();

    connect(ui->m_pBtnOpenFile, &QPushButton::clicked, this, &selectFile);
    connect(ui->m_pBtnStop, &QPushButton::clicked, this, &stopMedia);
    connect(ui->m_pBtnPlayOrPause, &QPushButton::clicked, this, &pauseSwitchMedia);
    connect(this, &MainWindow::SignalBtnEnable, this, &MainWindow::slotBtnEnable);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pVideoGLWidget;
    this->closeDecoder();
}

void MainWindow::slotBtnEnable(bool enable)
{
    ui->m_pBtnOpenFile->setEnabled(enable);
}

void MainWindow::resetControls()
{
    emit SignalBtnEnable(true);
    ui->m_pLabProcessBar->setText(tr("当前无文件播放"));
    m_pVideoGLWidget->DefaultPictureShow();
    QIcon button_ico("../../images/play.ico");
    ui->m_pBtnPlayOrPause->setIcon(button_ico);
}

void MainWindow::selectFile()
{
    QString url = QFileDialog::getOpenFileName(this, tr("打开文件"), "H://", "All File(*)");
    if(!url.isEmpty())
        playMedia(url);
}

void MainWindow::playMedia(QString url)
{
    ui->m_pLabProcessBar->setText(url);
    unique_lock<mutex> locker(m_mutexForDecoder);
    if(m_pDecoder)
    {
        locker.unlock();
        this->closeDecoder();
        this->resetControls();
        locker.lock();
    }
    m_pDecoder = new ffmpegutil::FFDecoder;
    if(!m_pDecoder->InitializeDecoder(url.toStdString()))
    {
        ui->m_pLabProcessBar->setText("InitializeDecoder:" + QString(m_pDecoder->ErrName().c_str()));
        delete m_pDecoder;
        m_pDecoder = NULL;
        return;
    }
    m_pDecoder->SetProcessDataCallback(std::bind(&MainWindow::processYuv, this, std::placeholders::_1));
    m_pDecoder->SetDecodeThreadExitCallback(std::bind(&MainWindow::processDecodeThreadExit, this, std::placeholders::_1));
    //帧率保留两位小数, 四舍五入
    float frameRate = m_pDecoder->GetVideoFrameRate() + 0.005;
    m_fFrameDuration = 100000.0/int(frameRate * 100);
    m_nLastRenderedTime = timeutil::GetSystemTimeMicrosecond();
    if(!m_pDecoder->StartDecodeThread())
    {
        ui->m_pLabProcessBar->setText("StartDecodeThread:" + QString(m_pDecoder->ErrName().c_str()));
        delete m_pDecoder;
        m_pDecoder = NULL;
        return;
    }
    ui->m_pLabProcessBar->setText("播放中:" + url);
}

void MainWindow::stopMedia()
{
    this->closeDecoder();
    this->resetControls();
}

void MainWindow::pauseSwitchMedia()
{
    unique_lock<mutex> locker(m_mutexForDecoder);
    if(m_pDecoder)
    {
        m_pDecoder->PauseSwitch();
        QIcon button_ico(m_pDecoder->IsPause() ? "../../images/pause.ico": "../../images/play.ico");
        ui->m_pBtnPlayOrPause->setIcon(button_ico);
    }
}

void MainWindow::processYuv(PictureFilePtr pPicture)
{
    m_pVideoGLWidget->PictureShow(pPicture);
    int64_t curTime = timeutil::GetSystemTimeMicrosecond();
    int64_t waitDuration = m_fFrameDuration * 1000 + int64_t(m_nLastRenderedTime - curTime);
    if(waitDuration > 0)
    {
        //        MyLog(info, "wait duration = %.2f ms\n", (double)waitDuration/1000);
        usleep(waitDuration);
    }
    m_nLastRenderedTime = timeutil::GetSystemTimeMicrosecond();
}

void MainWindow::processDecodeThreadExit(bool bIsOccurExit)
{
    if(bIsOccurExit)
    {
        ui->m_pLabProcessBar->setText("解码线程意外退出:" + QString(m_pDecoder->ErrName().c_str()));
        //在另一线程关闭解码器, 不允许任何形式上的delete this的操作。
        std::thread([this]{
            this->closeDecoder();
        }).detach();
    }
    this->resetControls();
}

void MainWindow::closeDecoder()
{
    unique_lock<mutex> locker(m_mutexForDecoder);
    if(m_pDecoder)
    {
        m_pDecoder->StopDecodeThread();
        m_pDecoder->SetProcessDataCallback(NULL);
        m_pDecoder->SetDecodeThreadExitCallback(NULL);
        delete m_pDecoder;
        m_pDecoder = NULL;
    }
}
