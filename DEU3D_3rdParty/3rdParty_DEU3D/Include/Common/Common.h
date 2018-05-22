#ifndef COMMON_H_16232D0C_9D85_404C_B879_9CE2E047FB1F_INCLUDE
#define COMMON_H_16232D0C_9D85_404C_B879_9CE2E047FB1F_INCLUDE


#include <string>

#include <IDProvider/ID.h>
#include <OpenThreads/Mutex>
#include "Export.h"
#pragma warning( disable : 4996 )

namespace cmm
{

CM_EXPORT void genUniqueValue32(unsigned &nValue);
CM_EXPORT void genUniqueValue64(unsigned __int64 &nValue);

CM_EXPORT bool isFileExist(const std::string &strFilePath);
CM_EXPORT unsigned getFileLength(const std::string &strFilePath);
CM_EXPORT unsigned getFileLength(FILE *pFile, OpenThreads::Mutex *pMutex = NULL);

#if defined (WIN32) || defined (WIN64)
//获取当前应用程序的路径, bContainDLL=TRUE 表示包含当前的DLL
CM_EXPORT std::string GetAppPath(bool bContainDLL);
//获取当前系统时间，判断是否需要毫秒，如"2012-12-18 12:04.670"
CM_EXPORT std::string GetCurDateTimeString(bool bMillisecond);
//获取文本文件里的全部内容
CM_EXPORT bool getFileContext(const std::string &strFilePath, void *&pBuffer, unsigned &nBufLen);
CM_EXPORT void freeMemory(void *pBuffer);

CM_EXPORT std::string getDllFilePath(const std::string &strDllName);
CM_EXPORT std::string getDllFileDir(const std::string &strDllName);
CM_EXPORT std::string genResourceFileDir(void);
CM_EXPORT std::string genRandomLocalDB(void);
CM_EXPORT std::string genLocalTempDB(void);
CM_EXPORT std::string genLocalTempDB2(void);
CM_EXPORT UINT_64     getLocalTempDBBufferSize(void);
CM_EXPORT unsigned __int64 getProcessMemory(void);

#endif


struct TerrainTile
{
    ID          m_idTopTile;
    unsigned    m_nMaxLevel;
};

typedef std::vector<TerrainTile> TerrainTileList;

struct HeightFieldHeader
{
    unsigned m_nVersionNum;
    unsigned m_nRowCount;
    unsigned m_nColCount;
    float    m_fltInvalidValue;
    unsigned char  m_szReserved[4];
};

CM_EXPORT unsigned checkNetworkStatus(void);

/************************************************************************/
/* 字符串函数                                                           */
/************************************************************************/
#ifdef WIN32
CM_EXPORT std::wstring  ANSIToUnicode(const std::string &str);
CM_EXPORT std::string   UnicodeToANSI(const std::wstring &str);
CM_EXPORT std::wstring  UTF8ToUnicode(const std::string &str);
CM_EXPORT std::string   UnicodeToUTF8(const std::wstring &str);
#endif

struct DateTime
{
    int         m_nYear;
    int         m_nMonth;
    int         m_nDate;
    double      m_dblHours;
};

CM_EXPORT void convertUTC2TimeZone(DateTime &dateTime, int nTimeZone);
CM_EXPORT void convertTimeZone2UTC(DateTime &dateTime, int nTimeZone);
CM_EXPORT bool encodingData(const void *pBuffer, unsigned nLength, char szOuter[]);
CM_EXPORT bool decodingData(const char *pCharacter, unsigned nLength, void *pOuter);

CM_EXPORT unsigned __int64 genGlobeUniqueNumber(void);

struct FloatColor
{
    FloatColor(){memset(this, 0, sizeof(FloatColor));}
    float   m_fltR;
    float   m_fltG;
    float   m_fltB;
    float   m_fltA;
};


struct ByteColor
{
    unsigned char   m_nR;
    unsigned char   m_nG;
    unsigned char   m_nB;
    unsigned char   m_nA;
};

CM_EXPORT unsigned int FloatColor2UintColor(const cmm::FloatColor &color);
CM_EXPORT cmm::FloatColor UintColor2FloatColor(unsigned int color);

}

#endif