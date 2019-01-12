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
