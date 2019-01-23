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
    /**
     * @brief 播放缓冲队列中播放时间小于阈值的缓冲数据
     * @param threshold: 阈值, 小于此值得缓冲数据会被播放
     */
    void                        playBufferMediaData(unsigned threshold);
    /**
     * @brief 插入一个数据到缓存队列
     * @param pRawData
     * @param streamType: 媒体类型, ffmpegutil::DataDelayTask::StreamType
     */
    void                        insertMediaDataToBuffer(fileutil::RawDataPtr pRawData, unsigned streamType, unsigned uPlayTime);
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
        unsigned                uMediaType; //等于ffmpegutil::DataDelayTask::StreamType
        fileutil::RawDataPtr    pMediaData;
    };
    //非播放同步基准媒体流的数据都将被缓冲
    std::mutex              m_mutexForBufferList;
    std::list<MediaData>    m_listForMediaDataBuffer;
    //媒体流同步的基准, 等于ffmpegutil::DataDelayTask::StreamType
    unsigned                m_uBenchmark;

    //音频播放
    std::mutex              m_mutexForAudioOutput;
    QAudioOutput            *m_pAudioOutput;
    QIODevice               *m_pAudioIODevice;
};

#endif // MAINWINDOW_H
