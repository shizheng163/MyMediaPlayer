#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <string>
#include <stdint.h>
#include <memory>
namespace fileutil {

struct FileRawData
{
    FileRawData(){
        m_pData = NULL;
    }
    ~FileRawData()
    {
        if(m_pData)
            delete m_pData;
        m_pData = NULL;
        m_uLen = 0;
    }
    uint8_t *   m_pData;
    uint32_t    m_uLen;
    std::string m_filename;
};
typedef std::shared_ptr<FileRawData> FileRawDataPtr;

FileRawDataPtr ReadFileRawData(std::string filename);

}//namespace fileutil

#endif // FILEUTIL_H
