/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
*/
#ifndef LOGUTIL_H
#define LOGUTIL_H
#include <stdio.h>
#include <string>
#include <stdarg.h>
namespace logutil {

enum MyLogType
{
    trace,
    debug,
    info,
    warn,
    err,
    fatal
};

void MyLog(MyLogType type, const char * fmt, ...);
std::string MySprintf(const char * fmt, ...);
}//namespace logutil

#endif // LOGUTIL_H
