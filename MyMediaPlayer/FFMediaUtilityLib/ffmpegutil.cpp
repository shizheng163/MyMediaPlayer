/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
*/
#include "ffmpegutil.h"
#include <error.h>
extern "C"
{
#include <libavformat/avformat.h>
}
using namespace std;
#define ERR_MEMORY_LEN 1024
void ffmpegutil::Initialize(bool bIsDebug)
{
    avformat_network_init();
    av_log_set_level(bIsDebug ? AV_LOG_TRACE : AV_LOG_INFO);
}

std::string ffmpegutil::GetStrError(int errcode)
{
    char errName[ERR_MEMORY_LEN]= {0};
    if(av_strerror(errcode, errName, ERR_MEMORY_LEN) == 0)
        return errName;
    if(strerror_s(errName, ERR_MEMORY_LEN, errcode) == NO_ERROR)
        return errName;
    return to_string(errcode);
}
