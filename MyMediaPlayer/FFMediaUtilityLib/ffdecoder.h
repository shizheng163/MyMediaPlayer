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
     * @brief ���ý������ݻص�����
     */
    void SetProcessDataCallback(ProcessYuvDataCallback callback);
    /**
     * @brief ���ý����߳��˳��Ļص�����
     */
    void SetDecodeThreadExitCallback(DecodeThreadExitCallback callback);
    /**
     * @brief ��ȡ����ԭ��
     * @note ��������ʱ, ����ԭ��ᱻ��¼
     */
    std::string ErrName() const { return m_szErrName;}
    /**
     * @brief ��ȡ��Ƶ֡��
     */
    float GetVideoFrameRate();
private:

    void decodeInThread();

    std::string                 m_szUrl;
    AVFormatContext             *m_pInputFormatContext;
    int                         m_nVideoStreamIndex;
    AVCodecContext              *m_pCodecContext;

    //�����������ݵĻص�����
    std::mutex                  m_mutexForFnProcessYuvData;
    ProcessYuvDataCallback      m_fnProcssYuvData;

    std::thread                 m_threadForDecode;
    bool                        m_bIsRunDecodeThread;

    std::string                 m_szErrName;

    //��������߳��˳��Ļص�����
    std::mutex                  m_mutexForFnThreadExit;
    DecodeThreadExitCallback    m_fnThreadExit;
};
}//namespace ffmpegutil

#endif // FFDECODER_H
