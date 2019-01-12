#include "ffdecoder.h"
#include <memory>
extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include "logutil.h"
#include "ffmpegutil.h"


using namespace ffmpegutil;
using namespace std;
using namespace logutil;

typedef std::shared_ptr<AVPacket> AVPacketPtr;
typedef std::shared_ptr<AVFrame> AVFramePtr;

FFDecoder::FFDecoder()
    :m_pInputFormatContext(NULL)
    ,m_nVideoStreamIndex(-1)
    ,m_pCodecContext(NULL)
{
}

FFDecoder::~FFDecoder()
{
    if(m_threadForDecode.joinable())
        m_threadForDecode.join();

    if(m_pInputFormatContext)
    {
        avformat_close_input(&m_pInputFormatContext);
        avformat_free_context(m_pInputFormatContext);
        m_pInputFormatContext = NULL;
    }
    if(m_pCodecContext)
    {
        avcodec_free_context(&m_pCodecContext);
        m_pCodecContext = NULL;
    }
}

bool FFDecoder::InitializeDecoder(string url)
{
    m_szUrl = url;
    int ret = 0;
    ret = avformat_open_input(&m_pInputFormatContext, m_szUrl.c_str(), NULL, NULL);
    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder open input failed, url = %s, err: %s", m_szUrl.c_str(), GetStrError(ret).c_str());
        return false;
    }
    for(unsigned i = 0; i < m_pInputFormatContext->nb_streams; i++)
    {
        if(m_pInputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_nVideoStreamIndex = i;
            break;
        }
    }

    if( m_nVideoStreamIndex == -1)
    {
        m_szErrName = MySprintf("FFDecoder could find video stream, url = %s", m_szUrl.c_str());
        return false;
    }

    ret = avformat_find_stream_info(m_pInputFormatContext, NULL);
    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder find stream info failed, url = %s, err: %s", m_szUrl.c_str(), GetStrError(ret).c_str());
        return false;
    }

//    av_dump_format(m_pInputFormatContext, m_nVideoStreamIndex, NULL, 0);

    //InitAVDecoder
    AVCodec * pCodec = avcodec_find_decoder(m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->codec_id);
    if(!pCodec)
    {
        m_szErrName = MySprintf("FFDecoder find AVCodec failed, url = %s, codec: %s", m_szUrl.c_str(), avcodec_get_name(m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->codec_id));
        return false;
    }
    m_pCodecContext = avcodec_alloc_context3(pCodec);

    //���ƽ���������
    ret = avcodec_parameters_to_context(m_pCodecContext, m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar);

    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder avcodec_parameters_to_context failed, url = %s, err = %s", m_szUrl.c_str(), avcodec_get_name(m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->codec_id), GetStrError(ret).c_str());
        return false;
    }

    ret = avcodec_open2(m_pCodecContext, pCodec, NULL);
    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder AVCodec Open  failed, url = %s, codec: %s, err = %s", m_szUrl.c_str(), avcodec_get_name(m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->codec_id), GetStrError(ret).c_str());
        return false;
    }
    return true;
}

bool FFDecoder::StartDecodeThread()
{
    if(!m_pCodecContext)
    {
        m_szErrName = "decode context not init!";
        return false;
    }
    m_bIsRunDecodeThread = true;
    m_threadForDecode = std::thread(&FFDecoder::decodeInThread, this);
    return true;
}

void FFDecoder::SetProcessDataCallback(FFDecoder::ProcessYuvDataCallback callback)
{
    std::unique_lock<mutex> locker(m_mutexForFnProcessYuvData);
    m_fnProcssYuvData = callback;
}

void FFDecoder::StopDecode()
{
    m_bIsRunDecodeThread = false;
}

float FFDecoder::GetVideoFrameRate()
{
    if(m_pInputFormatContext)
    {
        return (float)m_pInputFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.num / m_pInputFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.den ;
    }
    return 0;
}

void FFDecoder::decodeInThread()
{
    bool isEof = false;

    AVFramePtr pFrameScale(av_frame_alloc(), [](AVFrame * ptr){
        av_frame_free(&ptr);
        //�ͷŵ���av_image_alloc������ڴ�
        av_free(&ptr->data[0]);
    });

    int ret = 0;

    //��ʼ��ͼ��ת����
    AVStream * pVideoStream = m_pInputFormatContext->streams[m_nVideoStreamIndex];
    SwsContext * pswsContext = sws_getContext(pVideoStream->codecpar->width, pVideoStream->codecpar->height, (AVPixelFormat)pVideoStream->codecpar->format,
                                              pVideoStream->codecpar->width, pVideoStream->codecpar->height, AV_PIX_FMT_YUV420P,
                                              SWS_FAST_BILINEAR, 0, 0, 0);
    //����ͼ��洢�ڴ�
    av_image_alloc(pFrameScale->data, pFrameScale->linesize, pVideoStream->codecpar->width, pVideoStream->codecpar->height, AV_PIX_FMT_YUV420P, 1);

    while(m_bIsRunDecodeThread)
    {
        AVPacketPtr ptrAVPacket(av_packet_alloc(), [](AVPacket * ptr){
            av_packet_free(&ptr);
        });
        av_init_packet(ptrAVPacket.get());
        ret = av_read_frame(m_pInputFormatContext, ptrAVPacket.get());
        if(ret < 0)
        {
            string szErrName = GetStrError(ret).c_str();
            if(szErrName != "End of file")
                m_szErrName = MySprintf("FFMpeg Read Frame Failed, Err = %s", szErrName.c_str());
            else
            {
                MyLog(info, "FFMpeg Read Frame Complete!");
                isEof = true;
            }
            break;
        }
        if(ptrAVPacket->stream_index == m_nVideoStreamIndex)
        {
            ret = avcodec_send_packet(m_pCodecContext, ptrAVPacket.get());
            AVFramePtr pTempFrame(av_frame_alloc(), [](AVFrame * ptr){
                av_frame_free(&ptr);
            });
            while(1)
            {
                ret = avcodec_receive_frame(m_pCodecContext, pTempFrame.get());
                if(ret != 0)
                    break;
                ret= sws_scale(pswsContext, (const uint8_t* const*)pTempFrame->data, pTempFrame->linesize, 0, pTempFrame->height,
                               (uint8_t* const*)pFrameScale->data, pFrameScale->linesize);
                if(ret > 0)
                {
                    fileutil::FileRawData data;
                    data.AppendData(pFrameScale->data[0], pVideoStream->codecpar->width * pVideoStream->codecpar->height);
                    data.AppendData(pFrameScale->data[1], pVideoStream->codecpar->width * pVideoStream->codecpar->height / 4);
                    data.AppendData(pFrameScale->data[2], pVideoStream->codecpar->width * pVideoStream->codecpar->height / 4);
                    YuvDataPtr pYuvData(new fileutil::PictureFile(data, pVideoStream->codecpar->width, pVideoStream->codecpar->height, fileutil::PictureFile::kFormatYuv));
                    pYuvData->m_filename = to_string(pVideoStream->codec_info_nb_frames);
                    std::unique_lock<mutex> locker(m_mutexForFnProcessYuvData);
                    if(m_fnProcssYuvData)
                        m_fnProcssYuvData(pYuvData);
                }
            }
        }
    }
    MyLog(m_bIsRunDecodeThread && !isEof ? err : info, "FFDecoder %s exit!\n", m_bIsRunDecodeThread && !isEof ? "abnormal" : "normal");
}
