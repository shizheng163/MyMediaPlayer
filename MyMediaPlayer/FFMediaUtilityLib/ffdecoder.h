/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
*/
#ifndef FFDECODER_H
#define FFDECODER_H
#include <string>
#include <fileutil.h>
#include <functional>
#include <mutex>
#include <thread>
struct AVFormatContext;
struct AVFrame;
struct AVCodecContext;
namespace ffmpegutil {
typedef fileutil::PictureFilePtr YuvDataPtr;
class FFDecoder
{
public:
    typedef std::function<void (YuvDataPtr pYuvData) > ProcessYuvDataCallback;
    typedef std::function<void (bool bIsOccurErr) > DecodeThreadExitCallback;
    FFDecoder();
    ~FFDecoder();
    bool InitializeDecoder(std::string url);
    bool StartDecodeThread();
    void StopDecode();
    /**
     * @brief 设置解码数据回调函数
     */
    void SetProcessDataCallback(ProcessYuvDataCallback callback);
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
private:

    void decodeInThread();

    std::string                 m_szUrl;
    AVFormatContext             *m_pInputFormatContext;
    int                         m_nVideoStreamIndex;
    AVCodecContext              *m_pCodecContext;

    //处理解码后数据的回调函数
    std::mutex                  m_mutexForFnProcessYuvData;
    ProcessYuvDataCallback      m_fnProcssYuvData;

    std::thread                 m_threadForDecode;
    bool                        m_bIsRunDecodeThread;

    std::string                 m_szErrName;

    //处理解码线程退出的回调函数
    std::mutex                  m_mutexForFnThreadExit;
    DecodeThreadExitCallback    m_fnThreadExit;
};
}//namespace ffmpegutil

#endif // FFDECODER_H
