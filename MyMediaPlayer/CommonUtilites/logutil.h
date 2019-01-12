/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
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
