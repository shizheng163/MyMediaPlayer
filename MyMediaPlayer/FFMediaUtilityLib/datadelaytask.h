/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information for reference code
 * date:   2019-1-20
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef DATADELAYTASK_H
#define DATADELAYTASK_H
#include <fileutil.h>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
struct AVStream;
/**
 * @brief 数据延迟等待器, 接收解码后的音视频数据, 等待一定时间传递给处理音视频数据的回调函数。
 */
namespace ffmpegutil {
class DataDelayTask
{
public:
    enum StreamType
    {
        kStreamVideo,
        kStreamAudio
    };
    /**
     * @brief 处理数据的回调函数
     * @param uPlayTime: 播放时间戳, 单位为ms
     * @param type: 类型为StreamType
     */
    typedef std::function<void (fileutil::RawDataPtr pRawData, unsigned uPlayTime, unsigned type) > ProcessRawDataCallback;
public:
    DataDelayTask(StreamType streamType, AVStream *pStream);
    ~DataDelayTask();
    /**
     * @brief 设置缓冲帧数量
     */
    void SetBufferSize(unsigned FrameCount);
    void Start();
    /**
     * @brief 资源释放前需要先执行Stop
     */
    void Stop();
    void SetProcessRawDataCallback(ProcessRawDataCallback callback);
    void PushData(fileutil::RawDataPtr pData, int64_t pts);
    /**
     * @brief 暂停状态切换
     * @note 第一次调用为暂停, 第二次调用为播放
     */
    void PauseSwitch();
    /**
     * @brief 判断缓冲是否为空
     */
    bool BufferIsEmpty();
private:
    struct Rational
    {
        int num, den;
    };
    struct MediaData
    {
        unsigned                uPlayTime;
        int64_t                 nPts;
        fileutil::RawDataPtr    pMediaData;
    };
    struct StreamInfo
    {
        StreamType      streamType;
        int64_t         nFirstPts;
        int64_t         nLastFramePts;
        int64_t         nLastSendTimeMicro;
        Rational        timebase;
    };

    void runInThread();
    std::mutex                  m_mutexForMediaDataQueue;
    std::queue<MediaData>       m_queueForMediaData;

    StreamInfo                  m_streamInfo;

    std::condition_variable     m_conditionValForSleep; //延迟休眠
    std::condition_variable     m_conditionValForWaitData; //等待数据

    std::mutex                  m_mutexForMediaProcessFn;
    ProcessRawDataCallback      m_fnMediaDataProcess;

    bool                        m_bIsRunThread;
    std::thread                 m_threadForRun;

    unsigned                    m_uBufferMaxSize;
    std::condition_variable     m_conditionValForBuffer; //缓冲数据

    bool                        m_bIsPause;
    std::mutex                  m_mutexForPause;
    std::condition_variable     m_conditionValForPause; //暂停发送
};
}//namespace ffmpegutil


#endif // DATADELAYTASK_H
