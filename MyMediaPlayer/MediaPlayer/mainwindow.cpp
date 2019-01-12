﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <io.h>
#include <iostream>
#include <thread>
#include <windows.h>
#include <QDebug>
#include <queue>
#include <condition_variable>
#include <mutex>
#include "logutil.h"
#include "ffdecoder.h"
#include "ffmpegutil.h"
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
    connect(this, &MainWindow::SignalBtnEnable, this, &MainWindow::slotBtnEnable);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pVideoGLWidget;
    if(m_pDecoder)
    {
        m_pDecoder->StopDecode();
        m_pDecoder->SetProcessDataCallback(NULL);
        delete m_pDecoder;
        m_pDecoder = NULL;
    }
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
}

void MainWindow::selectFile()
{
    QString url = QFileDialog::getOpenFileName(this, tr("打开文件"), "H://", "All File(*.mp4)");
    if(!url.isEmpty())
        playMedia(url);
}

void MainWindow::playMedia(QString url)
{
    ui->m_pLabProcessBar->setText(url);
    if(m_pDecoder)
    {
        m_pDecoder->StopDecode();
        m_pDecoder->SetProcessDataCallback(NULL);
        delete m_pDecoder;
    }
    m_pDecoder = new ffmpegutil::FFDecoder;
    if(!m_pDecoder->InitializeDecoder(url.toStdString()))
    {
        ui->m_pLabProcessBar->setText("InitializeDecoder:" + QString(m_pDecoder->ErrName().c_str()));
        delete m_pDecoder;
        return;
    }
    m_pDecoder->SetProcessDataCallback(std::bind(&MainWindow::processYuv, this, std::placeholders::_1));
    m_fFrameDuration = 1000.0/m_pDecoder->GetVideoFrameRate();
    if(!m_pDecoder->StartDecodeThread())
    {
        ui->m_pLabProcessBar->setText("StartDecodeThread:" + QString(m_pDecoder->ErrName().c_str()));
        delete m_pDecoder;
        return;
    }
    ui->m_pLabProcessBar->setText("播放中:" + url);
}

void MainWindow::processYuv(PictureFilePtr pPicture)
{
    clock_t curTime = clock();
    double waitDuration = m_fFrameDuration + m_nLastRenderedTime - curTime;
    MyLog(info, "wait duration = %.2f ms\n", waitDuration);
    if(waitDuration > 0)
        Sleep(waitDuration);
    m_pVideoGLWidget->PictureShow(pPicture);
    m_nLastRenderedTime = clock();
}
