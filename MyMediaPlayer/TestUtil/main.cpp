#include <iostream>
#include <unistd.h>
using namespace std;
#include "ffdecoder.h"
#include "ffmpegutil.h"
#include "logutil.h"
int main()
{
//    ffmpegutil::Initialize(false);
//    ffmpegutil::FFDecoder decoder;
//    decoder.InitializeDecoder("H:\\MV\\Suger.mp4");
//    decoder.StartDecodeThread();
//    sleep(1);
    float duration = 318.4;
    printf("%.2fms\n", duration);
    logutil::MyLog(logutil::info, "%.2fms\n", duration);
    return 0;
}
