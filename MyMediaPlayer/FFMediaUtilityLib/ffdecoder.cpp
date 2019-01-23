/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include "ffdecoder.h"
#include <memory>
extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}
#include <unistd.h>
#include "logutil.h"
#include "ffmpegutil.h"


using namespace ffmpegutil;
using namespace std;
using namespace logutil;

typedef std::shared_ptr<AVPacket> AVPacketPtr;

FFDecoder::FFDecoder()
    :m_pInputFormatContext(NULL)
    ,m_bIsInitDecoder(false)
    ,m_nVideoStreamIndex(-1)
    ,m_pVideoCodecContext(NULL)
    ,m_pSwsContext(NULL)
    ,m_nAudioStreamIndex(-1)
    ,m_pAudioCodecContext(NULL)
    ,m_pSwrContext(NULL)
    ,m_bIsPause(false)
    ,m_pVideoDelayTask(NULL)
    ,m_pAudioDelayTask(NULL)
    ,m_uVideoFrameCount(0)
{
}

FFDecoder::~FFDecoder()
{
    m_bIsRunDecodeThread = false;

    if(m_pAudioDelayTask)
        m_pAudioDelayTask->Stop();

    if(m_pVideoDelayTask)
        m_pVideoDelayTask->Stop();

    if(m_threadForDecode.joinable())
        m_threadForDecode.join();

    if(m_pAudioDelayTask)
    {
        delete m_pAudioDelayTask;
        m_pAudioDelayTask = NULL;
    }

    if(m_pVideoDelayTask)
    {
        delete m_pVideoDelayTask;
        m_pVideoDelayTask = NULL;
    }

    if(m_pInputFormatContext)
    {
        avformat_close_input(&m_pInputFormatContext);
        avformat_free_context(m_pInputFormatContext);
        m_pInputFormatContext = NULL;
    }
    if(m_pVideoCodecContext)
    {
        avcodec_free_context(&m_pVideoCodecContext);
        m_pVideoCodecContext = NULL;
    }
    if(m_pSwrContext)
    {
        swr_free(&m_pSwrContext);
        m_pSwrContext = NULL;
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
    //先调用avformat_find_stream_info, 没有文件头的文件需要探测。
    ret = avformat_find_stream_info(m_pInputFormatContext, NULL);
    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder find stream info failed, url = %s, err: %s", m_szUrl.c_str(), GetStrError(ret).c_str());
        return false;
    }

    for(unsigned i = 0; i < m_pInputFormatContext->nb_streams; i++)
    {
        if(m_pInputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            m_nVideoStreamIndex = i;
        else if(m_pInputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            m_nAudioStreamIndex = i;

    }

    if(m_nVideoStreamIndex == -1 && m_nAudioStreamIndex == -1)
    {
        m_szErrName = MySprintf("FFDecoder could find video/audio stream, url = %s", m_szUrl.c_str());
        return false;
    }
    //av_dump_format(m_pInputFormatContext, m_nVideoStreamIndex, NULL, 0);
    //初始化视频解码器
    if(m_nVideoStreamIndex != -1 && !InitDecodeContext(&m_pVideoCodecContext, m_nVideoStreamIndex))
        return false;
    else
    {
        m_pVideoDelayTask = new DataDelayTask(DataDelayTask::kStreamVideo, m_pInputFormatContext->streams[m_nVideoStreamIndex]);
    }

    //初始化音频解码器
    if(m_nAudioStreamIndex != -1 && !InitDecodeContext(&m_pAudioCodecContext, m_nAudioStreamIndex))
        return false;
    else
    {
        m_pAudioDelayTask = new DataDelayTask(DataDelayTask::kStreamAudio, m_pInputFormatContext->streams[m_nAudioStreamIndex]);
    }

    m_bIsInitDecoder = true;
    return true;
}

bool FFDecoder::StartDecodeThread()
{
    if(!m_bIsInitDecoder)
    {
        m_szErrName = "decode context not init!";
        return false;
    }
    AVFramePtr pFrame(av_frame_alloc(), [](AVFrame * ptr)
    {
        av_frame_free(&ptr);
        //释放调用av_image_alloc申请的内存
        av_free(&ptr->data[0]);
    });
    m_pVideoImageFrame = pFrame;
    int ret = 0;

    //初始化视频图像转换环境
    if(m_nVideoStreamIndex != -1)
    {
        AVStream * pVideoStream = m_pInputFormatContext->streams[m_nVideoStreamIndex];
        m_pSwsContext = sws_getContext(pVideoStream->codecpar->width, pVideoStream->codecpar->height, (AVPixelFormat)pVideoStream->codecpar->format,
                                       pVideoStream->codecpar->width, pVideoStream->codecpar->height, AV_PIX_FMT_YUV420P,
                                       SWS_FAST_BILINEAR, 0, 0, 0);
        if(m_pSwsContext == NULL)
        {
            m_szErrName = "sws_getContext failed.";
            return false;
        }
        //申请图像存储内存
        ret = av_image_alloc(m_pVideoImageFrame->data, m_pVideoImageFrame->linesize, pVideoStream->codecpar->width, pVideoStream->codecpar->height, AV_PIX_FMT_YUV420P, 1);
        if(ret <= 0)
        {
            m_szErrName = "av_image_alloc failed:" + GetStrError(ret);
            return false;
        }
        m_pVideoDelayTask->Start();
    }
    //初始化音频格式转换环境
    if(m_nAudioStreamIndex != -1)
    {
        m_pSwrContext = swr_alloc_set_opts(nullptr, m_pAudioCodecContext->channel_layout, AV_SAMPLE_FMT_S16, m_pAudioCodecContext->sample_rate,
                                           m_pAudioCodecContext->channel_layout, m_pAudioCodecContext->sample_fmt, m_pAudioCodecContext->sample_rate, 0, NULL);
        if(m_pSwrContext == NULL)
        {
            m_szErrName = "swr_alloc_set_opts failed.";
            return false;
        }
        ret = swr_init(m_pSwrContext);
        if(ret != 0)
        {
            m_szErrName = "swr_init failed:" + GetStrError(ret);
            return false;
        }
        m_pAudioDelayTask->Start();
    }

    m_bIsRunDecodeThread = true;
    m_threadForDecode = std::thread(&FFDecoder::decodeInThread, this);
    return true;
}

void FFDecoder::SetDecodeThreadExitCallback(FFDecoder::DecodeThreadExitCallback callback)
{
    std::unique_lock<mutex> locker(m_mutexForFnThreadExit);
    m_fnThreadExit = callback;
}

void FFDecoder::StopDecodeThread()
{
    m_bIsRunDecodeThread = false;
}

void FFDecoder::SetProcessDataCallback(DataDelayTask::ProcessRawDataCallback callback)
{
    if(m_pAudioDelayTask)
        m_pAudioDelayTask->SetProcessRawDataCallback(callback);
    if(m_pVideoDelayTask)
        m_pVideoDelayTask->SetProcessRawDataCallback(callback);
}

float FFDecoder::GetVideoFrameRate()
{
    if(m_pInputFormatContext)
    {
        return (float)m_pInputFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.num / m_pInputFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.den ;
    }
    return 0;
}

bool FFDecoder::GetVideoSize(unsigned *width, unsigned *height)
{
    if(m_nVideoStreamIndex == -1)
    {
        *width = 0;
        *height = 0;
        return false;
    }
    *width = m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->width;
    *height = m_pInputFormatContext->streams[m_nVideoStreamIndex]->codecpar->height;
    return true;
}

int FFDecoder::GetAudioSampleRate()
{
    if(m_nAudioStreamIndex == -1)
        return 0;
    return m_pInputFormatContext->streams[m_nAudioStreamIndex]->codecpar->sample_rate;
}

int FFDecoder::GetAudioChannelNum()
{
    if(m_nAudioStreamIndex == -1)
        return 0;
    return m_pInputFormatContext->streams[m_nAudioStreamIndex]->codecpar->channels;
}

int FFDecoder::GetAudioSampleSize()
{
    return 8*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
}

void FFDecoder::PauseSwitch()
{
    m_bIsPause = !m_bIsPause;
    if(m_pAudioDelayTask)
        m_pAudioDelayTask->PauseSwitch();
    if(m_pVideoDelayTask)
        m_pVideoDelayTask->PauseSwitch();
}

bool FFDecoder::IsPause()
{
    return m_bIsPause;
}

DataDelayTask::StreamType FFDecoder::GetPlayBenchmark()
{
    if(m_pInputFormatContext->nb_streams == 1)
    {
        if(m_nVideoStreamIndex != -1)
            return DataDelayTask::kStreamVideo;
        if(m_nAudioStreamIndex != -1)
            return DataDelayTask::kStreamAudio;
    }
    else
    {
        if(m_nVideoStreamIndex != -1 && m_pInputFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.num !=0)
            return DataDelayTask::kStreamVideo;
        return DataDelayTask::kStreamAudio;
    }
    return DataDelayTask::kStreamVideo;
}

unsigned FFDecoder::GetPlayDuration()
{
    return m_pInputFormatContext->duration / AV_TIME_BASE;
}

bool FFDecoder::InitDecodeContext(AVCodecContext **pCodecContext, int streamIndex)
{
    int ret = 0;
    //初始化视频解码环境
    AVCodec * pCodec = avcodec_find_decoder(m_pInputFormatContext->streams[streamIndex]->codecpar->codec_id);
    if(!pCodec)
    {
        m_szErrName = MySprintf("FFDecoder find avcodec failed, codec: %s", avcodec_get_name(m_pInputFormatContext->streams[streamIndex]->codecpar->codec_id));
        return false;
    }
    *pCodecContext = avcodec_alloc_context3(pCodec);

    //复制解码器参数
    ret = avcodec_parameters_to_context(*pCodecContext, m_pInputFormatContext->streams[streamIndex]->codecpar);

    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder avcodec_parameters_to_context failed,err = %s", avcodec_get_name(m_pInputFormatContext->streams[streamIndex]->codecpar->codec_id), GetStrError(ret).c_str());
        return false;
    }

    ret = avcodec_open2(*pCodecContext, pCodec, NULL);
    if(ret < 0)
    {
        m_szErrName = MySprintf("FFDecoder avodec open failed, codec: %s, err = %s",  avcodec_get_name(m_pInputFormatContext->streams[streamIndex]->codecpar->codec_id), GetStrError(ret).c_str());
        return false;
    }
    return true;
}

void FFDecoder::decodeInThread()
{
    bool isEof = false;
    int ret = 0;
    //初始化图像转换器
    AVStream * pVideoStream = NULL;
    if(m_nVideoStreamIndex != -1)
        pVideoStream = m_pInputFormatContext->streams[m_nVideoStreamIndex];

    while(m_bIsRunDecodeThread)
    {
        //解复用
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

        AVFramePtr pTempFrame(av_frame_alloc(), [](AVFrame * ptr){
            av_frame_free(&ptr);
        });
        //解码
        if(ptrAVPacket->stream_index == m_nVideoStreamIndex)
        {
            if(ptrAVPacket->pts==AV_NOPTS_VALUE){
                AVRational time_base1=m_pInputFormatContext->streams[m_nVideoStreamIndex]->time_base;
                //Duration between 2 frames (us)
                int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(m_pInputFormatContext->streams[m_nVideoStreamIndex]->r_frame_rate);
                //Parameters
                ptrAVPacket->pts=(double)(m_uVideoFrameCount*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                ptrAVPacket->dts=ptrAVPacket->pts;
                ptrAVPacket->duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            }
            m_uVideoFrameCount++;
            ret = avcodec_send_packet(m_pVideoCodecContext, ptrAVPacket.get());
            while(1)
            {
                ret = avcodec_receive_frame(m_pVideoCodecContext, pTempFrame.get());
                if(ret != 0)
                    break;
                ret= sws_scale(m_pSwsContext, (const uint8_t* const*)pTempFrame->data, pTempFrame->linesize, 0, pTempFrame->height,
                               (uint8_t* const*)m_pVideoImageFrame->data, m_pVideoImageFrame->linesize);
                if(ret > 0)
                {
                    fileutil::RawDataPtr data(new fileutil::RawData);
                    data->AppendData(m_pVideoImageFrame->data[0], pVideoStream->codecpar->width * pVideoStream->codecpar->height);
                    data->AppendData(m_pVideoImageFrame->data[1], pVideoStream->codecpar->width * pVideoStream->codecpar->height / 4);
                    data->AppendData(m_pVideoImageFrame->data[2], pVideoStream->codecpar->width * pVideoStream->codecpar->height / 4);
                    m_pVideoDelayTask->PushData(data, pTempFrame->pts);
                }
            }
        }
        else if(ptrAVPacket->stream_index == m_nAudioStreamIndex)
        {
            ret = avcodec_send_packet(m_pAudioCodecContext, ptrAVPacket.get());
            while(1)
            {
                ret = avcodec_receive_frame(m_pAudioCodecContext, pTempFrame.get());
                if(ret != 0)
                    break;
                int bufSize = av_samples_get_buffer_size(NULL, pTempFrame->channels, pTempFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);
                fileutil::RawDataPtr data(new fileutil::RawData);
                data->m_pData = new uint8_t[bufSize];
                data->m_uLen = bufSize;
                ret = swr_convert(m_pSwrContext, &data->m_pData, pTempFrame->nb_samples, (const uint8_t**)(pTempFrame->data), pTempFrame->nb_samples);
                if(ret > 0)
                {
                    m_pAudioDelayTask->PushData(data, pTempFrame->pts);
                }
            }
        }
    }
    while(m_bIsRunDecodeThread)
    {
        if((!m_pAudioDelayTask || m_pAudioDelayTask->BufferIsEmpty()) && (!m_pVideoDelayTask || m_pVideoDelayTask->BufferIsEmpty()))
            break;
        usleep(100);
    }
    MyLog(m_bIsRunDecodeThread && !isEof ? err : info, "FFDecoder %s exit!\n", m_bIsRunDecodeThread && !isEof ? "abnormal" : "normal");
    std::unique_lock<mutex> locker(m_mutexForFnThreadExit);
    if(m_fnThreadExit)
        m_fnThreadExit(m_bIsRunDecodeThread && !isEof);
}
