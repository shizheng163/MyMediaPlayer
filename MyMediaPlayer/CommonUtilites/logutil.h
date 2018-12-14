#ifndef LOGUTIL_H
#define LOGUTIL_H
#include <stdio.h>
#include <string>
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

std::string LogTypeToString(MyLogType type);

template<typename...Args>
void MyLog(MyLogType type, const char * fmt, const Args&... args)
{
    std::string fmtStr = LogTypeToString(type) + ":";
    fmtStr.append(fmt);
    printf(fmtStr.c_str(), args...);
    fflush(stdout);
}

}//namespace logutil

#endif // LOGUTIL_H
