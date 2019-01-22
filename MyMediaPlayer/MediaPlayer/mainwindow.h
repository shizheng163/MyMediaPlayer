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
#include <queue>
#include <QAudioOutput>
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
    //拖放功能
    void                        dragEnterEvent(QDragEnterEvent *event);
    void                        dropEvent(QDropEvent *event);

    void                        slotBtnEnable(bool enable);
    void                        resetControls();
    void                        selectFile();
    void                        playMedia(QString url);
    void                        stopMedia();
    void                        pauseSwitchMedia();
    void                        processMediaRawData(fileutil::RawDataPtr pRawData, unsigned uPlayTime, unsigned streamType);
    void                        processDecodeThreadExit(bool bIsOccurExit);
    void                        closeDecoder();
    void                        closeAudioContext();
    /**
     * @brief 初始化音频环境
     * @param sampleRate: 采样率
     * @param channelNum: 通道数
     * @param sampleSize: 采样大小(Bit)
     */
    void                        initAudioContext(unsigned sampleRate, unsigned channelNum, unsigned sampleSize);
    void                        playAudio(fileutil::RawDataPtr pRawData);
    Ui::MainWindow *ui;

    VideoGLWidget           *m_pVideoGLWidget;

    unsigned                m_uVideoWidth;
    unsigned                m_uVideoHeight;
    //解码
    std::mutex              m_mutexForDecoder;
    ffmpegutil::FFDecoder   *m_pDecoder;

    //当前渲染数据的阈值, 大于此值得数据将被缓冲
    unsigned                m_uThreshold;
    struct MediaData
    {
        unsigned                uPlayTime;
        fileutil::RawDataPtr    pMediaData;
    };
    std::queue<MediaData>   m_queueForAudioData;
    //媒体流同步的基准, 等于ffmpegutil::DataDelayTask::StreamType
    unsigned                m_uBenchmark;

    //音频播放
    std::mutex              m_mutexForAudioOutput;
    QAudioOutput            *m_pAudioOutput;
    QIODevice               *m_pAudioIODevice;
};

#endif // MAINWINDOW_H
