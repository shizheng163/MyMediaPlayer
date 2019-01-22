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
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "logutil.h"
#include "ffdecoder.h"
#include "ffmpegutil.h"
#include "timeutil.h"
#include "datadelaytask.h"
using namespace std;
using namespace logutil;
using namespace fileutil;
MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_pVideoGLWidget(NULL)
    ,m_pDecoder(NULL)
    ,m_pAudioOutput(NULL)
    ,m_pAudioIODevice(NULL)
{
    //布局
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setStyleSheet("QMainWindow{background-color:#C9C9C9;}");
    this->setWindowTitle("MediaPlayer");

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

    this->setAcceptDrops(true);
    connect(ui->m_pBtnOpenFile, &QPushButton::clicked, this, &selectFile);
    connect(ui->m_pBtnStop, &QPushButton::clicked, this, &stopMedia);
    connect(ui->m_pBtnPlayOrPause, &QPushButton::clicked, this, &pauseSwitchMedia);
    connect(this, &MainWindow::SignalBtnEnable, this, &MainWindow::slotBtnEnable);
}

MainWindow::~MainWindow()
{
    this->closeDecoder();
    delete ui;
    delete m_pVideoGLWidget;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //如果为文件，则支持拖放
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.size() != 1)
        return;

    QString path = urls.front().path();
#ifdef WIN32
    if(path[0] == '/')
        path.remove(0,1);
#endif
    playMedia(path);
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
    m_uThreshold = 0;
    m_pDecoder->SetProcessDataCallback(std::bind(&MainWindow::processMediaRawData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pDecoder->SetDecodeThreadExitCallback(std::bind(&MainWindow::processDecodeThreadExit, this, std::placeholders::_1));
    m_pDecoder->GetVideoSize(&m_uVideoWidth, &m_uVideoHeight);
    if(m_uVideoWidth != 0 && m_uVideoHeight != 0)
        m_uBenchmark = ffmpegutil::DataDelayTask::kStreamVideo;
    else
        m_uBenchmark = ffmpegutil::DataDelayTask::kStreamAudio;

    this->closeAudioContext();
    this->initAudioContext(m_pDecoder->GetAudioSampleRate(), m_pDecoder->GetAudioChannelNum(), m_pDecoder->GetAudioSampleSize());

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

void MainWindow::processMediaRawData(RawDataPtr pRawData, unsigned uPlayTime, unsigned streamType)
{
    //有视频时以视频为基准
    if(streamType == ffmpegutil::DataDelayTask::kStreamVideo)
    {
        m_pVideoGLWidget->PictureShow(pRawData, m_uVideoWidth, m_uVideoHeight);
        ui->m_pLabProcessBar->setText(to_string(uPlayTime).c_str());
        m_uThreshold = uPlayTime;
        while(!m_queueForAudioData.empty())
        {
            MediaData & data = m_queueForAudioData.front();
            if(data.uPlayTime > m_uThreshold)
                break;
            m_queueForAudioData.pop();
        }
    }
    else if(streamType == ffmpegutil::DataDelayTask::kStreamAudio)
    {
        //如果以音频为基准
        if(m_uBenchmark == ffmpegutil::DataDelayTask::kStreamAudio)
        {
            m_uThreshold = uPlayTime;
            ui->m_pLabProcessBar->setText(to_string(uPlayTime).c_str());
            playAudio(pRawData);
        }
        else if(uPlayTime <= m_uThreshold)
            playAudio(pRawData);
        else
            m_queueForAudioData.push({uPlayTime, pRawData});
    }
}

void MainWindow::processDecodeThreadExit(bool bIsOccurExit)
{
    if(bIsOccurExit)
    {
        ui->m_pLabProcessBar->setText("解码线程意外退出:" + QString(m_pDecoder->ErrName().c_str()));
    }
    //在另一线程关闭解码器, 不允许任何形式上的delete this的操作。
    std::thread([this]{
        this->closeDecoder();
    }).detach();
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

void MainWindow::closeAudioContext()
{
    unique_lock<mutex> locker(m_mutexForAudioOutput);
    if(m_pAudioOutput)
    {
        m_pAudioOutput->stop();
        delete m_pAudioOutput;
        m_pAudioOutput = NULL;
        m_pAudioIODevice = NULL;
    }
}

void MainWindow::initAudioContext(unsigned sampleRate, unsigned channelNum, unsigned sampleSize)
{
    unique_lock<mutex> locker(m_mutexForAudioOutput);
    //没有音频时会返回0
    if(sampleRate != 0)
    {
        QAudioFormat audioFormat;
        audioFormat.setSampleRate(sampleRate);
        audioFormat.setChannelCount(channelNum);
        audioFormat.setSampleSize(sampleSize);
        audioFormat.setSampleType(QAudioFormat::SignedInt);
        audioFormat.setCodec("audio/pcm");
        m_pAudioOutput = new QAudioOutput(audioFormat);
        m_pAudioIODevice = m_pAudioOutput->start();
    }
}

void MainWindow::playAudio(RawDataPtr pRawData)
{
    unique_lock<mutex> locker(m_mutexForAudioOutput);
    if(m_pAudioIODevice)
        m_pAudioIODevice->write((char *)pRawData->m_pData, pRawData->m_uLen);
}
