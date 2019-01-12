#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H
#include <string>
namespace ffmpegutil {

void Initialize(bool bIsDebug = false);

std::string GetStrError(int errcode);

}//namespace ffmpegutil

#endif // FFMPEGUTIL_H
