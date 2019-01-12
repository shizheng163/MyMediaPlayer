/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H
#include <string>
namespace ffmpegutil {

void Initialize(bool bIsDebug = false);

std::string GetStrError(int errcode);

}//namespace ffmpegutil

#endif // FFMPEGUTIL_H
