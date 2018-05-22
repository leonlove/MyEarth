#include <Common.h>
#include <sstream>
#ifdef WIN32
#include <io.h>
#include <Windows.h>
#include <Sensapi.h>
#include <tchar.h>
#include <Psapi.h>
#else
#include <stdio.h>
#include <string.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <md2.h>
#include <OpenThreads/Thread>
#include <map>
#include <direct.h>
#include <sys/types.h>
#include <sys/timeb.h>


namespace cmm
{
    class LocalServices
    {
    public:
        LocalServices(void)
        {
            unsigned int nValue32;
            genUniqueValue32(nValue32);

            unsigned __int64 nValue64;
            genUniqueValue64(nValue64);

            genGlobeUniqueNumber();

            setlocale(LC_ALL,"chs");

            std::string tempDir = "";
#ifdef _DEBUG
            tempDir = getDllFileDir("Commond.dll");
#else
            tempDir = getDllFileDir("Common.dll");
#endif
            tempDir += "\\TempFile\\";

            _mkdir(tempDir.c_str());

            genLocalTempDB();
            genLocalTempDB2();

            genResourceFileDir();
        }
    };
    static LocalServices _LocalServices;

    void genUniqueValue32(unsigned &nValue)
    {
        static OpenThreads::Mutex mtx;
        static unsigned nUnique = 0u;
        mtx.lock();
        nUnique++;
        nValue = nUnique;
        mtx.unlock();
    }

    void genUniqueValue64(unsigned __int64 &nValue)
    {
        static OpenThreads::Mutex mtx;
        static unsigned __int64 nUnique = 0u;
        mtx.lock();
        nUnique++;
        nValue = nUnique;
        mtx.unlock();
    }

    bool isFileExist(const std::string &strFilePath)
    {
        if(-1 == access(strFilePath.c_str(), 0))
        {
            return false;
        }
        return true;
    }


    unsigned getFileLength(const std::string &strFilePath)
    {
        FILE *pFile = fopen(strFilePath.c_str(), "rb");
        if(NULL == pFile)  return 0u;

        const unsigned nLength = getFileLength(pFile);
        fclose(pFile);
        return nLength;
    }


    unsigned getFileLength(FILE *pFile, OpenThreads::Mutex *pMutex)
    {
        if(NULL != pMutex)
        {
            pMutex->lock();
        }

        const long nPos = ftell(pFile);
        fseek(pFile, 0, SEEK_END);
        const unsigned nLength = (unsigned)ftell(pFile);
        fseek(pFile, nPos, SEEK_SET);

        if(NULL != pMutex)
        {
            pMutex->unlock();
        }

        return nLength;
    }


    //获取当前应用程序的路径，
#if defined (WIN32) || defined (WIN64)
    std::string GetAppPath(bool bContainDLL)
    {
        char szAppPath[1024] = {0};
        GetModuleFileNameA(GetModuleHandleA(NULL), szAppPath, 1024);
        char *pXmlFile = _tcsrchr(szAppPath, '\\');
        if (pXmlFile)
        {
            pXmlFile++;
            *pXmlFile = '\0';
            if(bContainDLL)
            {
#if _MSC_VER >= 1400
                _tcscat_s(szAppPath, _T("Common.dll"));
#else
                _tcscat(szAppPath, _T("Common.dll"));
#endif
            }
        }

        std::string strPath = szAppPath;
        return strPath;
    }

    std::string GetCurDateTimeString(bool bMillisecond)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        std::ostringstream strTime;
        //"2012-12-18 12:04.670";
        strTime<<st.wYear<<"-";
        if(st.wMonth < 10) strTime<<"0";
        strTime<<st.wMonth<<"-";
        if(st.wDay < 10) strTime<<"0";
        strTime<<st.wDay<<" ";
        if(st.wHour < 10) strTime<<"0";
        strTime<<st.wHour<<":";
        if(st.wMinute < 10) strTime<<"0";
        strTime<<st.wMinute<<":";
        if(st.wSecond < 10) strTime<<"0";
        strTime<<st.wSecond;

        if (bMillisecond)
        {
            strTime<<".";
            if(st.wMilliseconds < 10) strTime<<"00";
            else if(st.wMilliseconds < 100) strTime<<"0";
            strTime<<st.wMilliseconds;
        }
        return strTime.str();
    }

    bool getFileContext(const std::string &strFilePath, void *&pBuffer, unsigned &nBufLen)
    {
        FILE *pFile = fopen(strFilePath.c_str(), "rb");
        if(!pFile)  return false;

        fseek(pFile, 0, SEEK_END);
        const int nFileLen = ftell(pFile);
        if(nFileLen <= 0)
        {
            fclose(pFile);
            return false;
        }

        fseek(pFile, 0, SEEK_SET);
        void *pFileContext = malloc(nFileLen);
        if(fread(pFileContext, nFileLen, 1, pFile) != 1)
        {
            fclose(pFile);
            free(pFileContext);
            return false;
        }

        fclose(pFile);

        pBuffer = pFileContext;
        nBufLen = nFileLen;
        return true;
    }

    void freeMemory(void *pBuffer)
    {
        free(pBuffer);
    }
#endif


#ifdef WIN32
    std::wstring ANSIToUnicode(const std::string &str)
    {
        const int iTextLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

        std::wstring  wstrText(iTextLen, 0);
        ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &*wstrText.begin(), iTextLen);

        return wstrText;
    }


    std::string UnicodeToANSI(const std::wstring &str)
    {
        const int iTextLen = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        std::string  strText(iTextLen, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &*strText.begin(), iTextLen, NULL, NULL);

        return strText;
    }


    std::wstring UTF8ToUnicode(const std::string &str)
    {
        int iTextLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

        std::wstring  wstrText(iTextLen, 0);
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &*wstrText.begin(), iTextLen);

        return wstrText;
    }


    std::string UnicodeToUTF8(const std::wstring &str)
    {
        const int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        std::string  strText(iTextLen, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &*strText.begin(), iTextLen, NULL, NULL);

        return strText;
    }
#endif

    unsigned checkNetworkStatus(void)
    {
        unsigned nRetValue = 0u;
#ifdef WIN32
        DWORD   dwFlags = 0;
        if(::IsNetworkAlive(&dwFlags))
        {
            nRetValue = 0u;
        }
        else
        {
            nRetValue = 1u;
        }
#else
        ;
#endif
        return nRetValue;
    }


    bool isLeapYear(int nYear)
    {
        if(nYear % 4 != 0)
        {
            return false;
        }

        if((nYear % 100 == 0) && (nYear % 400 != 0))
        {
            return false;
        }

        return true;
    }


    int getMonthDays(int nYear, int nMonth)
    {
        if(nMonth == 2)
        {
            if(isLeapYear(nYear))
            {
                return 29;
            }
            return 28;
        }
        else if(nMonth == 4 || nMonth == 6 || nMonth == 9 || nMonth == 11)
        {
            return 30;
        }
        return 31;
    }


    void convertUTC2TimeZone(DateTime &dateTime, int nTimeZone)
    {
        dateTime.m_dblHours += nTimeZone;
        if(dateTime.m_dblHours <= 24.0)
        {
            return;
        }

        dateTime.m_dblHours -= 24.0;
        dateTime.m_nDate += 1;
        if(dateTime.m_nDate <= 28)
        {
            // no month's day count less than 28
            return;
        }

        if(dateTime.m_nMonth == 2)
        {
            if(!isLeapYear(dateTime.m_nYear))
            {
                dateTime.m_nDate  = 1;
                dateTime.m_nMonth = 3;
            }
        }
        else if(dateTime.m_nMonth == 4 || dateTime.m_nMonth == 6 || dateTime.m_nMonth == 9 || dateTime.m_nMonth == 11)
        {
            if(dateTime.m_nDate <= 30)
            {
                return;
            }
            dateTime.m_nDate   = 1;
            dateTime.m_nMonth += 1;
        }
        else
        {
            if(dateTime.m_nDate <= 31)
            {
                return;
            }
            dateTime.m_nDate  = 1;
            if(dateTime.m_nMonth == 12)
            {
                dateTime.m_nMonth = 1;
                dateTime.m_nYear += 1;
            }
            else
            {
                dateTime.m_nMonth += 1;
            }
        }
    }


    void convertTimeZone2UTC(DateTime &dateTime, int nTimeZone)
    {
        dateTime.m_dblHours -= nTimeZone;
        if(dateTime.m_dblHours >= 0.0)
        {
            return;
        }

        dateTime.m_dblHours += 24.0;
        dateTime.m_nDate -= 1;
        if(dateTime.m_nDate >= 1)
        {
            return;
        }

        if(dateTime.m_nMonth > 1)
        {
            dateTime.m_nMonth -= 1;
            dateTime.m_nDate   = getMonthDays(dateTime.m_nYear, dateTime.m_nMonth);
        }
        else
        {
            dateTime.m_nMonth = 12;
            dateTime.m_nDate  = 31;
            dateTime.m_nYear -= 1;
        }
    }

    struct BitFour
    {
        unsigned  LowBit : 4;
        unsigned  HighBit : 4;
    };

    bool encodingData(const void *pBuffer, unsigned nLength, char szOuter[])
    {
        const static char g_ch[16] = {'0','1','2','3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        if(!pBuffer || nLength < 1u)    return false;

        char *pch = szOuter;
        const char *buffer = (const char *)pBuffer;

        for(unsigned i = 0; i < nLength; i++, buffer++)
        {
            const BitFour *pBits = (const BitFour *)buffer;
            *pch++ = g_ch[pBits->LowBit];
            *pch++ = g_ch[pBits->HighBit];
        }

        return true;
    }


    bool decodingData(const char *pCharacter, unsigned nLength, void *pOuter)
    {
        if(!pCharacter || nLength == 0 || nLength % 2u != 0u)
        {
            return false;
        }

        static unsigned int Table[256] = {0};
        static bool bTableInvalid = true;
        if(bTableInvalid)
        {
            Table['0'] = 0;     Table['1'] = 1;
            Table['2'] = 2;     Table['3'] = 3;
            Table['4'] = 4;     Table['5'] = 5;
            Table['6'] = 6;     Table['7'] = 7;
            Table['8'] = 8;     Table['9'] = 9;
            Table['A'] = 10;    Table['B'] = 11;
            Table['C'] = 12;    Table['D'] = 13;
            Table['E'] = 14;    Table['F'] = 15;
            bTableInvalid = false;
        }

        unsigned    dstLen  = nLength / 2;
        char        *tmp    = (char *)pOuter;
        const char  *pch    = pCharacter;

        for(unsigned i = 0u; i < dstLen; i++, tmp++)
        {
            BitFour *pBits = (BitFour *)tmp;
            pBits->LowBit  = Table[*pch++];
            pBits->HighBit = Table[*pch++];
        }

        return true;
    }

    unsigned __int64 genGlobeUniqueNumber(void)
    {
        union ValueMask
        {
            unsigned char    szBuffer[8];
            unsigned short   nShortBit[4];
            unsigned int     nIntBit[2];
            unsigned __int64 nTime;
        }value;

        _timeb  tb;
        _ftime(&tb);
        value.nTime = tb.time;
        value.nShortBit[2] = tb.millitm;

        static bool bSrand = true;
        if(bSrand)
        {
            srand(value.nIntBit[0]);
            bSrand = false;
        }

        value.szBuffer[5] |= (rand() % 256) << 2u;
        value.szBuffer[6]  = rand() % 256;
        value.szBuffer[7]  = rand() % 256;

        return value.nTime;
    }

    union ColorConverter
    {
        struct
        {
            unsigned char   m_blue;
            unsigned char   m_green;
            unsigned char   m_red;
            unsigned char   m_alpha;
        };
        unsigned int   m_uColor;
    };


    unsigned int FloatColor2UintColor(const cmm::FloatColor &color)
    {
        ColorConverter converter;
        converter.m_red   = (unsigned char)(color.m_fltR * 255.0f);
        converter.m_green = (unsigned char)(color.m_fltG * 255.0f);
        converter.m_blue  = (unsigned char)(color.m_fltB * 255.0f);
        converter.m_alpha = (unsigned char)(color.m_fltA * 255.0f);

        return converter.m_uColor;
    }


    cmm::FloatColor UintColor2FloatColor(unsigned int color)
    {
        ColorConverter converter;
        converter.m_uColor = color;

        cmm::FloatColor clr;
        clr.m_fltR = converter.m_red   / 255.0f;
        clr.m_fltG = converter.m_green / 255.0f;
        clr.m_fltB = converter.m_blue  / 255.0f;
        clr.m_fltA = converter.m_alpha / 255.0f;
        return clr;
    }

    std::string getDllFileDir(const std::string &strDllName)
    {
        std::string strRet = getDllFilePath(strDllName);
        static const char * const PATH_SEPARATORS = "/\\";
        static unsigned int PATH_SEPARATORS_LEN = 2;

        const std::string::size_type slash = strRet.find_last_of(PATH_SEPARATORS);
        if (slash == std::string::npos) return std::string();

        strRet = std::string(strRet, 0u, slash + 1u);

        return strRet;
    }

    std::string getDllFilePath(const std::string &strDllName)
    {
        std::string strRet;
        HANDLE hHandle = ::GetModuleHandleA(strDllName.c_str());
        if(!hHandle)    return strRet;

        char szFilePath[256] = {0};
        ::GetModuleFileNameA((HMODULE)hHandle, szFilePath, 256u);
        strRet = szFilePath;
        return strRet;
    }

    std::string genResourceFileDir(void)
    {
        std::string strDB;
#ifdef _DEBUG
        strDB = getDllFileDir("Commond.dll");
#else
        strDB = getDllFileDir("Common.dll");
#endif
        strDB += "Res\\";
        return strDB;
    }

    std::string genRandomLocalDB(void)
    {
        std::string strDB;

#ifdef _DEBUG
        strDB = getDllFileDir("Commond.dll");
#else
        strDB = getDllFileDir("Common.dll");
#endif
        strDB += "TempFile\\";

        const ID id = ID::genNewID();
        char szName[40] = {0};
        encodingData(id.ObjectID.m_UniqueID, sizeof(id.ObjectID.m_UniqueID), szName);
        strDB += szName;

        return strDB;
    }

    std::string genLocalTempDB(void)
    {
        static bool bFirst = true;
        static std::string strDB;
        if(bFirst)
        {
            bFirst = false;
#ifdef _DEBUG
            strDB = getDllFileDir("Commond.dll");
#else
            strDB = getDllFileDir("Common.dll");
#endif
            strDB += "TempFile\\";

            const ID id = ID::genNewID();
            char szName[40] = {0};
            encodingData(id.ObjectID.m_UniqueID, sizeof(id.ObjectID.m_UniqueID), szName);
            strDB += szName;
        }
        return strDB;
    }

    std::string genLocalTempDB2(void)
    {
        static bool bFirst = true;
        static std::string strDB;
        if(bFirst)
        {
            bFirst = false;
#ifdef _DEBUG
            strDB = getDllFileDir("Commond.dll");
#else
            strDB = getDllFileDir("Common.dll");
#endif
            strDB += "TempFile\\";

            const ID id = ID::genNewID();
            char szName[40] = {0};
            encodingData(id.ObjectID.m_UniqueID, sizeof(id.ObjectID.m_UniqueID), szName);
            strDB += szName;
        }
        return strDB;
    }

    UINT_64 getLocalTempDBBufferSize(void)
    {
        static const UINT_64   K = 1024ui64;
        static const UINT_64   M = K * K;
        static const UINT_64   nSize = M * 512ui64;
        return nSize;
    }

    unsigned __int64 getProcessMemory(void)
    {
#ifdef WIN32
        HANDLE hProcess = ::GetCurrentProcess();

        PROCESS_MEMORY_COUNTERS  memory;
        ::GetProcessMemoryInfo(hProcess, &memory, sizeof(PROCESS_MEMORY_COUNTERS));
        const unsigned __int64 nCurSize = memory.WorkingSetSize;
        return nCurSize;
#else
        return 0u;
#endif
    }

}
