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
        m_uLen = 0;
    }
    FileRawData(FileRawData && filedata)
    {
        m_pData = filedata.m_pData;
        m_uLen = filedata.m_uLen;
        m_filename = filedata.m_filename;
        filedata.m_pData = NULL;
        filedata.m_uLen = 0;
        filedata.m_filename.clear();
    }
    ~FileRawData()
    {
        if(m_pData)
            delete m_pData;
        m_pData = NULL;
        m_uLen = 0;
    }
    void AppendData(uint8_t * pData, unsigned uLen)
    {
        uint8_t * pBuffer = new uint8_t[m_uLen + uLen];
        if(m_pData)
            memcpy(pBuffer, m_pData, m_uLen);
        memcpy(pBuffer + m_uLen, pData, uLen);
        m_uLen += uLen;
        delete m_pData;
        m_pData = pBuffer;
    }
    uint8_t *   m_pData;
    uint32_t    m_uLen;
    std::string m_filename;
};
typedef std::shared_ptr<FileRawData> FileRawDataPtr;
struct PictureFile: public FileRawData
{
    enum PictureFormat
    {
        kFormatYuv,
        kFormatJpeg
    };
    PictureFile(FileRawData & parent, uint32_t nWeight, uint32_t nHeight, PictureFormat pictFormat)
        :FileRawData(std::move(parent))
        ,m_nWeight(nWeight)
        ,m_nHeight(nHeight)
        ,m_pictFormat(pictFormat)
    {

    }
    uint32_t        m_nWeight;
    uint32_t        m_nHeight;
    PictureFormat   m_pictFormat;
};
typedef std::shared_ptr<PictureFile> PictureFilePtr;
FileRawDataPtr ReadFileRawData(std::string filename);

}//namespace fileutil

#endif // FILEUTIL_H
