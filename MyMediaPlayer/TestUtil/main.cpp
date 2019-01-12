/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#include <iostream>
#include <unistd.h>
using namespace std;
#include "ffdecoder.h"
#include "ffmpegutil.h"
#include "logutil.h"
int main()
{
    ffmpegutil::Initialize(false);
    ffmpegutil::FFDecoder decoder;
    decoder.InitializeDecoder("H:\\MV\\Suger.mp4");
    decoder.StartDecodeThread();
    sleep(1);
    return 0;
}
