/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information for reference code
 * date:   2019-1-20
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include "datadelaytask.h"
extern "C"
{
#include <libavformat/avformat.h>
}
#include <unistd.h>
#include "timeutil.h"
#include "logutil.h"
using namespace ffmpegutil;
using namespace std;
DataDelayTask::DataDelayTask(StreamType streamType, AVStream *pStream)
    :m_bIsRunThread(false)
    ,m_uBufferMaxSize(10)
    ,m_bIsPause(false)
{
    m_streamInfo.nFirstPts = INT64_MAX;
    m_streamInfo.nLastFramePts = INT64_MAX;
    m_streamInfo.streamType = streamType;
    m_streamInfo.timebase.den = pStream->time_base.den;
    m_streamInfo.timebase.num = pStream->time_base.num;
}

DataDelayTask::~DataDelayTask()
{
    if(m_bIsRunThread)
        this->Stop();
}

void DataDelayTask::Start()
{
    m_bIsRunThread = true;
    m_streamInfo.nLastSendTimeMicro = timeutil::GetSystemTimeMicrosecond();
    m_threadForRun = std::thread(&DataDelayTask::runInThread, this);
}

void DataDelayTask::Stop()
{
    m_bIsRunThread = false;
    m_conditionValForWaitData.notify_one();
    m_conditionValForSleep.notify_one();
    m_conditionValForPause.notify_one();
    m_conditionValForBuffer.notify_one();
    if(m_threadForRun.joinable())
        m_threadForRun.join();
}

void DataDelayTask::SetProcessRawDataCallback(DataDelayTask::ProcessRawDataCallback callback)
{
    std::unique_lock<mutex> lockerForFnProcess(m_mutexForMediaProcessFn);
    m_fnMediaDataProcess = callback;
}

void DataDelayTask::PushData(fileutil::RawDataPtr pData, int64_t pts)
{
    std::unique_lock<mutex> lockerForVideoQueue(m_mutexForMediaDataQueue);
    MediaData data;
    if(m_streamInfo.nFirstPts == INT64_MAX)
    {
        m_streamInfo.nFirstPts = pts;
        m_streamInfo.nLastFramePts = pts;
    }
    data.nPts = pts;
    data.pMediaData = pData;
    data.uPlayTime = (data.nPts - m_streamInfo.nFirstPts) * m_streamInfo.timebase.num * 1000.0 / m_streamInfo.timebase.den;
    data.pMediaData->m_szDataDescribe = std::to_string(data.uPlayTime);
    if(m_queueForMediaData.size() >= m_uBufferMaxSize)
        m_conditionValForBuffer.wait(lockerForVideoQueue);
    m_queueForMediaData.push(data);
    m_conditionValForWaitData.notify_one();
}

void DataDelayTask::PauseSwitch()
{
    unique_lock<mutex> lockerForPause(m_mutexForPause);
    m_bIsPause = !m_bIsPause;
    if(!m_bIsPause)
        m_conditionValForPause.notify_one();
}

bool DataDelayTask::BufferIsEmpty()
{
    std::unique_lock<mutex> lockerForVideoQueue(m_mutexForMediaDataQueue);
    return m_queueForMediaData.empty();
}

void DataDelayTask::runInThread()
{
    //此locker不做同步使用, 只作为条件变量m_conditionValForSleep传参使用, 放到外围为减少开销
    std::mutex mutexForSleep;
    unique_lock<mutex> lockerForSleep(mutexForSleep);
    while(m_bIsRunThread)
    {
        std::unique_lock<mutex> lockerForMediaDataQueue(m_mutexForMediaDataQueue);
        if(m_queueForMediaData.empty())
            m_conditionValForWaitData.wait(lockerForMediaDataQueue);
        if(!m_bIsRunThread)
            break;
        MediaData data = m_queueForMediaData.front();
        m_queueForMediaData.pop();
        if(m_queueForMediaData.size() < m_uBufferMaxSize)
            m_conditionValForBuffer.notify_one();
        lockerForMediaDataQueue.unlock();

        unique_lock<mutex> lockerForPause(m_mutexForPause);
        if(m_bIsPause)
            m_conditionValForPause.wait(lockerForPause);
        lockerForPause.unlock();
        //暂停完发现任务退出则退出循环
        if(!m_bIsRunThread)
            break;

        std::unique_lock<mutex> lockerForFnProcess(m_mutexForMediaProcessFn);
        if(m_fnMediaDataProcess)
            m_fnMediaDataProcess(data.pMediaData, data.uPlayTime, m_streamInfo.streamType);
        lockerForFnProcess.unlock();

        int64_t curTime = timeutil::GetSystemTimeMicrosecond();
        int64_t needWaitDuration = (data.nPts - m_streamInfo.nLastFramePts) * 1000000.0 * m_streamInfo.timebase.num / m_streamInfo.timebase.den;
        if(curTime < m_streamInfo.nLastSendTimeMicro + needWaitDuration)
        {
            int64_t waitDuration = m_streamInfo.nLastSendTimeMicro - curTime +  needWaitDuration;
//            logutil::MyLog(logutil::info, "[streamInfo %s] wait duration = %lld us\n", m_streamInfo.streamType == kStreamVideo ? "video" : "audio", waitDuration);
            m_conditionValForSleep.wait_for(lockerForSleep, std::chrono::microseconds(waitDuration));
        }
        m_streamInfo.nLastSendTimeMicro = timeutil::GetSystemTimeMicrosecond();
        m_streamInfo.nLastFramePts = data.nPts;
    }
}
