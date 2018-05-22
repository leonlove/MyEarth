#include "WMTSDriver.h"
#include "CSimpleHttpClient.h"
#include <common/Common.h>
#include "TileSet.h"
#include <sstream>

namespace deues
{
    WMTSDriver::WMTSDriver(void)
    {
    }


    WMTSDriver::~WMTSDriver(void)
    {
    }

    IWMTSDriver *createWMTSDriver(void)
    {
        OpenSP::sp<WMTSDriver> pDriver = new WMTSDriver;
        return pDriver.release();
    }

    void freeMemory(void *p)
    {
        free(p);
    }

    bool WMTSDriver::initialize(const std::string& strUrl,
                                const std::string& strVersion)
    {
        int nFind = strUrl.rfind('?');
        if(nFind == -1)
        {
            m_strUrl = strUrl;
        }
        else
        {
            m_strUrl = strUrl.substr(0,nFind);
        }
        m_strVersion = strVersion;
        return true;
    }

    std::string WMTSDriver::getMetaData() const
    {
        int nError = 0;
        std::string strMetaData = "";
        bool bRes = QueryMetaData(strMetaData,nError);
        return strMetaData;
    }

    std::string WMTSDriver::getParamDefinition(void) const
    {
        std::string strPath = cmm::GetAppPath(false);
        strPath += "wmts_define.xsd";

        OpenThreads::ScopedLock<OpenThreads::Mutex> lockFile(m_driverMutex);
        FILE* fp = fopen(strPath.c_str(),"rb");
        if(fp)
        {
            fseek(fp,0L,SEEK_END); 
            unsigned nLength = ftell(fp);
            char* pBuffer = (char*)malloc(nLength+1);
            memset(pBuffer,'\0',nLength+1);
            fseek(fp,0L,SEEK_SET);
            fread(pBuffer,nLength,1,fp);
            fclose(fp);
            std::string strRes = pBuffer;
            free(pBuffer);
            return strRes;
        }
        else
        {
            return "";
        }
    }

    //请求元数据
    bool WMTSDriver::QueryMetaData(std::string& strMetaData,int& nError) const
    {
        //variables
        char* szRespBuf = NULL;
        long nResplen = 0;
        //request
        SimpleHttpClient shc;
        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WMTS&VERSION="<<m_strVersion<<"&REQUEST=GetCapabilities";
        int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
        //return
        if(nRet == 0 || szRespBuf != NULL)
        {
            unsigned i = 0;
            for(i = nResplen - 1;i >= 0;i--)
            {
                if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
                {
                    break ;
                }
            }
            szRespBuf[i+1] = 0;
            strMetaData = szRespBuf;
            shc.FreeResponse(szRespBuf);
            return true;
        }
        else
        {
            nError = nRet;
            return false;
        }
    }

    ITileSet* WMTSDriver::getTileSet(void)
    {
        OpenSP::sp<TileSet> pTileSet = new TileSet;
        const std::string strMetaData = getMetaData();
        const bool bRet = pTileSet->initialize(strMetaData.c_str(), m_strUrl,m_strVersion);
        if(!bRet)   return NULL;

        return pTileSet.release();
    }
}

