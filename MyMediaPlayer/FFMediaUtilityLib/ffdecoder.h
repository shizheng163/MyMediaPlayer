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
    FFDecoder();
    ~FFDecoder();
    bool InitializeDecoder(std::string url);
    bool StartDecodeThread();
    void SetProcessDataCallback(ProcessYuvDataCallback callback);
    void StopDecode();
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

    std::string             m_szUrl;
    AVFormatContext         *m_pInputFormatContext;
    int                     m_nVideoStreamIndex;
    AVCodecContext          *m_pCodecContext;

    std::mutex              m_mutexForFnProcessYuvData;
    ProcessYuvDataCallback  m_fnProcssYuvData;

    std::thread             m_threadForDecode;
    bool                    m_bIsRunDecodeThread;

    std::string             m_szErrName;
};
}//namespace ffmpegutil

#endif // FFDECODER_H
