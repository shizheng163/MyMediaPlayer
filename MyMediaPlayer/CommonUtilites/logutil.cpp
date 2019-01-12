/*
Copyright © 2018-2019 shizheng. All Rights Reserved.
日期: 2019-1-13
作者: 史正
邮箱: shizheng163@126.com
*/
#include "logutil.h"
#include <unordered_map>
#include <iostream>
#define BUFFSIZE 65536
const static std::unordered_map<int, std::string> g_logTypeStringMap=
{
    {logutil::trace,    "trace"},
    {logutil::debug,    "debug"},
    {logutil::info,     "info "},
    {logutil::warn,     "warn "},
    {logutil::err,      "err  "},
    {logutil::fatal,    "fatal "},
};

void logutil::MyLog(logutil::MyLogType type, const char *fmt, ...)
{
    char pBuff[BUFFSIZE];
    va_list valist;
    va_start(valist, fmt);
    vsprintf(pBuff, fmt, valist);
    va_end(valist);
    std::string szLog = pBuff;
    szLog.insert(0, g_logTypeStringMap.at(type) + ":");
    if(szLog.back() == '\n')
        szLog.pop_back();
    std::cout << szLog << std::endl;
}

std::__cxx11::string logutil::MySprintf(const char *fmt, ...)
{
    char pBuff[BUFFSIZE];
    va_list valist;
    va_start(valist, fmt);
    vsprintf(pBuff, fmt, valist);
    va_end(valist);
    return pBuff;
}
