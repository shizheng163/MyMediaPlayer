/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
*/
#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H
#include <string>
namespace ffmpegutil {

void Initialize(bool bIsDebug = false);

std::string GetStrError(int errcode);

}//namespace ffmpegutil

#endif // FFMPEGUTIL_H
