#include "fileutil.h"
#include <io.h>
#include "logutil.h"
using namespace std;
using namespace logutil;

fileutil::FileRawDataPtr fileutil::ReadFileRawData(std::string filename)
{
    if(access(filename.c_str(), 0 ) != 0)
    {
        MyLog(err, "ReadFileRawData Failed, Filename is not existed:%s\n", filename);
        return NULL;
    }
    FileRawDataPtr pFileRawData(new FileRawData);
    FILE * fFile = fopen(filename.c_str(), "rb");
    fseek(fFile, 0, SEEK_END);
    pFileRawData->m_uLen = ftell(fFile);
    pFileRawData->m_pData = new uint8_t[pFileRawData->m_uLen];
    pFileRawData->m_filename = filename;
    fseek(fFile, 0, SEEK_SET);
    fread(pFileRawData->m_pData, sizeof(uint8_t), pFileRawData->m_uLen, fFile);
    fclose(fFile);
    return pFileRawData;
}
