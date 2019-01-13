/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information for reference code
 * date:   2019-1-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include "timeutil.h"
#include <chrono>
using namespace std;

int64_t timeutil::GetSystemTimeMicrosecond()
{
    //定义微妙级别的时钟类型
    typedef chrono::time_point<chrono::system_clock, chrono::microseconds> microClock_type;
    //获取当前时间点，windows system_clock是100纳秒级别的(不同系统不一样，自己按照介绍的方法测试)，所以要转换
    microClock_type tp = chrono::time_point_cast<chrono::microseconds>(chrono::system_clock::now());
    return tp.time_since_epoch().count();
}
