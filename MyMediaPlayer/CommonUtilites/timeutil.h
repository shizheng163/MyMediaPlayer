/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information for reference code
 * date:   2019-1-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef TIMEUTIL_H
#define TIMEUTIL_H
#include <stdint.h>
#include <string>
namespace timeutil {

int64_t GetSystemTimeMicrosecond();

std::string SecondToHHMMSS(unsigned inSecond);
}//namespace timeutil

#endif // TIMEUTIL_H
