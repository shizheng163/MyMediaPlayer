/*
 * copyright (c) 2018-2019 shizheng. All Rights Reserved.
 * Please retain author information while you reference code
 * date:   2019-01-13
 * author: shizheng
 * email:  shizheng163@126.com
 */
#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <string>
#include <stdint.h>
#include <memory>
namespace fileutil {

struct RawData
{
    RawData(){
        m_pData = NULL;
        m_uLen = 0;
    }
    RawData(RawData && filedata)
    {
        m_pData = filedata.m_pData;
        m_uLen = filedata.m_uLen;
        m_szDataDescribe = filedata.m_szDataDescribe;
        filedata.m_pData = NULL;
        filedata.m_uLen = 0;
        filedata.m_szDataDescribe.clear();
    }
    ~RawData()
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
    /**
     * @brief 数据描述, 可以是文件名/当前第几帧/第多少次采样
     */
    std::string m_szDataDescribe;
};
typedef std::shared_ptr<RawData> FileRawDataPtr;
struct PictureFile: public RawData
{
    enum PictureFormat
    {
        kFormatYuv,
        kFormatJpeg
    };
    PictureFile(RawData & parent, uint32_t nWeight, uint32_t nHeight, PictureFormat pictFormat)
        :RawData(std::move(parent))
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
