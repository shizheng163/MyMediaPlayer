/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef FFDECODER_H
#define FFDECODER_H
#include <string>
#include <fileutil.h>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <datadelaytask.h>
struct AVFormatContext;
struct AVFrame;
struct AVCodecContext;
struct SwsContext;
struct SwrContext;
namespace ffmpegutil {
typedef fileutil::PictureFilePtr YuvDataPtr;
typedef std::shared_ptr<AVFrame> AVFramePtr;
class FFDecoder
{
public:

    typedef std::function<void (bool bIsOccurErr) > DecodeThreadExitCallback;
    FFDecoder();
    ~FFDecoder();
    bool InitializeDecoder(std::string url);
    bool StartDecodeThread();
    void StopDecodeThread();
    /**
     * @brief 设置解码数据回调函数
     */
    void SetProcessDataCallback(DataDelayTask::ProcessRawDataCallback callback);
    /**
     * @brief 设置解码线程退出的回调函数
     */
    void SetDecodeThreadExitCallback(DecodeThreadExitCallback callback);
    /**
     * @brief 获取错误原因
     * @note 当错误发生时, 错误原因会被记录
     */
    std::string ErrName() const { return m_szErrName;}
    /**
     * @brief 获取视频帧率
     */
    float GetVideoFrameRate();

    /**
     * @brief 获取视频的尺寸
     */
    bool GetVideoSize(unsigned *width, unsigned *height);

    /**
     * @brief 获取音频的采样率
     */
    int GetAudioSampleRate();

    /**
     * @brief 获取音频通道数
     */
    int GetAudioChannelNum();

    /**
     * @brief 获取采样大小 bit
     */
    int GetAudioSampleSize();

    /**
     * @brief 暂停状态切换
     * @note 第一次调用为暂停, 第二次调用为播放
     */
    void  PauseSwitch();

    bool  IsPause();

private:
    /**
     * @brief 创建解码环境
     * @param pCodecContext: 需要初始化的解码环境上下文
     * @param streamIndex: 媒体流的索引
     */
    bool InitDecodeContext(AVCodecContext ** pCodecContext, int streamIndex);
    void decodeInThread();

    std::string                 m_szUrl;
    AVFormatContext             *m_pInputFormatContext;
    bool                        m_bIsInitDecoder;
    //视频解码环境
    int                         m_nVideoStreamIndex;
    AVCodecContext              *m_pVideoCodecContext;
    //视频解码后变换颜色空间格式后存储数据的结构
    AVFramePtr                  m_pVideoImageFrame;
    //颜色空间变换上下文
    SwsContext                  *m_pSwsContext;

    //音频解码环境
    int                         m_nAudioStreamIndex;
    AVCodecContext              *m_pAudioCodecContext;
    //音频排列格式转换上下文
    SwrContext                  *m_pSwrContext;

    std::thread                 m_threadForDecode;
    bool                        m_bIsRunDecodeThread;

    std::string                 m_szErrName;

    //处理解码线程退出的回调函数
    std::mutex                  m_mutexForFnThreadExit;
    DecodeThreadExitCallback    m_fnThreadExit;

    bool                        m_bIsPause;

    //数据延迟
    DataDelayTask               *m_pVideoDelayTask;
    DataDelayTask               *m_pAudioDelayTask;
};
}//namespace ffmpegutil

#endif // FFDECODER_H
