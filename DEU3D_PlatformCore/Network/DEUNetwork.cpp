#include "DEUNetwork.h"
#include <Windows.h>
#include <string.h>
#include <common/DEUBson.h>
#include <OpenSP/sp.h>
#include <OpenThreads/ScopedLock>
#include <Common/Common.h>
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <sstream>
#include "DEUDefine.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <IDProvider/Definer.h>

#include "DEUCheck.h"
DEUCheck checker(18, 2);

namespace deunw
{
    std::map<int, std::string> MapErr;
    #define  THREADCOUNT   3

    const UINT_64       MB                  = 1024ui64 * 1024ui64;
    const UINT_64       g_nReadBufferSize   = 128ui64 * MB;
    const UINT_64       g_nWriteBufferSize  = 64ui64 * MB;

    unsigned genUniqueID(void)
    {
        unsigned nReqID = 0u;
        cmm::genUniqueValue32(nReqID);
        return nReqID;
    }

    IDEUNetwork *createDEUNetwork(void)
    {
        OpenSP::sp<DEUNetwork> pFileCache = new DEUNetwork;

        //错误映射表
        MapErr.insert(std::make_pair<int, std::string>(-8000000, "成功"));
        MapErr.insert(std::make_pair<int, std::string>(0, "部分成功"));
        MapErr.insert(std::make_pair<int, std::string>(1, "网络有问题"));
        MapErr.insert(std::make_pair<int, std::string>(2, "参数不正确"));
        MapErr.insert(std::make_pair<int, std::string>(3, "URL格式不正确"));
        MapErr.insert(std::make_pair<int, std::string>(4, "连接到服务器失败"));
        MapErr.insert(std::make_pair<int, std::string>(5, "发送请求到服务器失败"));
        MapErr.insert(std::make_pair<int, std::string>(6, "得到response长度失败"));
        MapErr.insert(std::make_pair<int, std::string>(7, "得到response内容失败"));

        MapErr.insert(std::make_pair<int, std::string>(101, "获取散列信息失败"));
        MapErr.insert(std::make_pair<int, std::string>(102, "从服务端获取到的信息没有需要的参数"));
        MapErr.insert(std::make_pair<int, std::string>(103, "端口为空"));
        MapErr.insert(std::make_pair<int, std::string>(104, "无效的ID"));
        MapErr.insert(std::make_pair<int, std::string>(105, "启动/停止服务失败"));

        MapErr.insert(std::make_pair<int, std::string>(151, "输入数据为空"));

        MapErr.insert(std::make_pair<int, std::string>(200, "bson格式无效"));
        MapErr.insert(std::make_pair<int, std::string>(201, "图层自包含"));
        MapErr.insert(std::make_pair<int, std::string>(202, "从deudb中读取失败"));
        MapErr.insert(std::make_pair<int, std::string>(203, "更新数据失败"));
        MapErr.insert(std::make_pair<int, std::string>(204, "从deudb中删除失败"));
        MapErr.insert(std::make_pair<int, std::string>(205, "获取类型失败"));
        MapErr.insert(std::make_pair<int, std::string>(206, "获取db和id失败"));
        MapErr.insert(std::make_pair<int, std::string>(207, "创建deudb失败"));
        MapErr.insert(std::make_pair<int, std::string>(208, "打开deudb失败"));
        MapErr.insert(std::make_pair<int, std::string>(209, "获取post数据失败"));
        MapErr.insert(std::make_pair<int, std::string>(210, "空的post数据"));
        MapErr.insert(std::make_pair<int, std::string>(211, "无效的post数据"));
        MapErr.insert(std::make_pair<int, std::string>(212, "无法连接mysql"));
        MapErr.insert(std::make_pair<int, std::string>(213, "选择database失败"));
        MapErr.insert(std::make_pair<int, std::string>(214, "创建database"));
        MapErr.insert(std::make_pair<int, std::string>(215, "创建表失败"));
        MapErr.insert(std::make_pair<int, std::string>(216, "删除数据集失败"));
        MapErr.insert(std::make_pair<int, std::string>(217, "删除数据集属性失败"));
        MapErr.insert(std::make_pair<int, std::string>(218, "添加属性失败"));
        MapErr.insert(std::make_pair<int, std::string>(219, "更新属性失败"));
        MapErr.insert(std::make_pair<int, std::string>(220, "读取的数据为空"));
        MapErr.insert(std::make_pair<int, std::string>(221, "根据属性查ID失败"));
        MapErr.insert(std::make_pair<int, std::string>(222, "打开文件失败"));
        MapErr.insert(std::make_pair<int, std::string>(223, "获取端口失败"));
        MapErr.insert(std::make_pair<int, std::string>(224, "读取配置文件失败"));
        MapErr.insert(std::make_pair<int, std::string>(225, "空的请求串"));
        MapErr.insert(std::make_pair<int, std::string>(226, "添加数据块失败"));
        MapErr.insert(std::make_pair<int, std::string>(227, "添加虚拟瓦片失败"));
        MapErr.insert(std::make_pair<int, std::string>(228, "数据已经存在")); 
        MapErr.insert(std::make_pair<int, std::string>(229, "替换数据失败")); 
        MapErr.insert(std::make_pair<int, std::string>(230, "获取版本失败")); 
        MapErr.insert(std::make_pair<int, std::string>(231, "添加儿子ID失败")); 
        MapErr.insert(std::make_pair<int, std::string>(232, "父图层不存在")); 
        MapErr.insert(std::make_pair<int, std::string>(233, "子图层不存")); 
        MapErr.insert(std::make_pair<int, std::string>(234, "不是符号库ID")); 
        MapErr.insert(std::make_pair<int, std::string>(235, "不是图层ID")); 
        MapErr.insert(std::make_pair<int, std::string>(236, "分配空间失败，请检查是否磁盘空间不足")); 

        MapErr.insert(std::make_pair<int, std::string>(301, "计算瓦片范围失败"));
        MapErr.insert(std::make_pair<int, std::string>(302, "初始化接口失败"));
        MapErr.insert(std::make_pair<int, std::string>(303, "获取接口失败"));
        MapErr.insert(std::make_pair<int, std::string>(304, "获取外部数据失败"));
        MapErr.insert(std::make_pair<int, std::string>(305, "根据数据集代码获取服务信息失败"));
        MapErr.insert(std::make_pair<int, std::string>(306, "初始化服务失败"));

        MapErr.insert(std::make_pair<int, std::string>(400, "未知错误"));
        MapErr.insert(std::make_pair<int, std::string>(401, "获取用户名和密码失败"));
        MapErr.insert(std::make_pair<int, std::string>(402, "用户名或密码错误"));
        MapErr.insert(std::make_pair<int, std::string>(403, "生成票据失败"));
        MapErr.insert(std::make_pair<int, std::string>(404, "获取用户条目错误"));
        MapErr.insert(std::make_pair<int, std::string>(405, "无效票据，请重新登录"));
        MapErr.insert(std::make_pair<int, std::string>(406, "没有相应的权限"));
        MapErr.insert(std::make_pair<int, std::string>(407, "获取票据失败"));
        MapErr.insert(std::make_pair<int, std::string>(408, "已有用户登录，切换用户请先将当前用户注销"));
        MapErr.insert(std::make_pair<int, std::string>(409, "该用户没有登录"));


        return pFileCache.release();
    }    

    //free memory
    void freeMemory(void *pData)
    {
        if(NULL == pData)
        {
            return;
        }

        free(pData);
    }

    DEUNetwork::DEUNetwork(void)
    {
        m_strTicket = "";
        m_strUser = "";
        m_bLogin = false;
        m_strAuthHost = "";
        m_strAuthPort = "";

        m_pNetworkCheckerThread = NULL;
        m_bOffLineMode    = true;
        m_bNetworkHealthy = true;
        m_nThread = 0;
        setOffLineMode(false);
    }

    DEUNetwork::~DEUNetwork(void)
    {
        for(unsigned n = 0;n < m_vecDownloadingThread.size();n++)
        {
            DownloadingThread* pThread = m_vecDownloadingThread[n];
            pThread->finishMission();
            delete pThread;
            pThread = NULL;
        }

        setOffLineMode(true);
        if(m_pDBProxy.valid())
        {
            m_pDBProxy->closeDB();
            m_pDBProxy = NULL;
        }
    }

    std::string GetErrDesc(int nErrCode)
    {
        std::string strError = "未知错误";
        std::map<int, std::string>::iterator pItr = MapErr.find(nErrCode);
        if(pItr != MapErr.end())
        {
            strError = pItr->second;
        }
        return strError;
    }


    //初始化服务信息
    bool DEUNetwork::initialize(
            const std::string& strHost,
            const std::string& strPort,
            const bool bInitForFetching,
            const std::string& strLocalCache)
    {
        m_strHost = strHost;
        m_strApachePort = strPort;
        if(bInitForFetching)
        {
            if(!m_queryData.InitHost(m_strHost,m_strApachePort))
            {
                if(m_vecDownloadingThread.size() < THREADCOUNT)
                {
                    for(unsigned n = 0;n < THREADCOUNT;n++)
                    {
                        DownloadingThread* pThread = new DownloadingThread(this);
                        pThread->startThread();
                        m_vecDownloadingThread.push_back(pThread);
                    }
                }
                removeCache(strLocalCache);
                return false;
            }
            if(strLocalCache != "")
            {
                OpenDB(strLocalCache);
            }
        }
        setOffLineMode(false);

        if(m_vecDownloadingThread.size() < THREADCOUNT)
        {
            for(unsigned n = 0;n < THREADCOUNT;n++)
            {
                DownloadingThread* pThread = new DownloadingThread(this);
                pThread->startThread();
                m_vecDownloadingThread.push_back(pThread);
            }
        }
        return true;       
    }

    // 获取数据集、散列信息
    std::string DEUNetwork::getDSInfo(const std::string &strType, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(!canUseNetwork())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK + DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return "";
        }

        //url
        std::ostringstream oss;
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type="<<strType<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type="<<strType<<'\0';
#endif
        //variables
        std::vector<char> vecRespBuf;

        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nRet);
                pOutExcep->setMessage(GetErrDesc(nRet));
            }
            return "";
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson) return "";

        bson::bsonElement* bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_UNKNOWN);
                pOutExcep->setMessage(GetErrDesc(DEU_UNKNOWN));
            }
            return "";
        }

        const int nRetCode = bElem->Int32Value();
        if(nRetCode == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nError = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nError = bElem->Int32Value();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return "";
        }

        bElem = bsonDoc.GetElement("Data");
        if(bElem == NULL)
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_RECEIVE_NO_PARAM);
                pOutExcep->setMessage(GetErrDesc(DEU_RECEIVE_NO_PARAM));
            }
            return "";
        }

        if(bElem->GetType() != bson::bsonBinType)
        {
            return "";
        }

        const char* tmpBson = (const char* )bElem->BinData();
        const unsigned nlen = bElem->BinDataLen();
        std::string strReturn(nlen + 1u, '\0');
        strReturn.resize(nlen + 1u, '\0');
        memcpy(&*strReturn.begin(), tmpBson, nlen);

        return strReturn;
    }

    // 创建数据集\散列信息文件
    bool DEUNetwork::setDSInfo(const std::string &strText,const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(!canUseNetwork())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
        //url
        std::ostringstream oss;
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type="<<strType<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type="<<strType<<'\0';
#endif

        std::vector<char>   vecRespBuf;
        std::vector<char>   vecResText;
        if(strText.empty())
        {
            //vecResText.push_back(' ');
        }
        else
        {
            vecResText.assign(strText.begin(), strText.end());
        }

        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, oss.str(), vecRespBuf, vecResText);
        if(vecRespBuf.empty())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nRet);
                pOutExcep->setMessage(GetErrDesc(nRet));
            }
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        bson::bsonElement* bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_UNKNOWN);
                pOutExcep->setMessage(GetErrDesc(DEU_UNKNOWN));
            }
            return false;
        }

        const int nRetCode = bElem->Int32Value();
        if(nRetCode == 0)//error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nError = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nError = bElem->Int32Value();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return false;
        }

        return true;
    }

    bool DEUNetwork::removeCache(const std::string& strDBPath)
    {
        std::string strIdxPath = strDBPath + ".idx";
        int nRes = remove(strIdxPath.c_str());
        if(nRes == -1)
        {
            if(errno == 2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        unsigned nCount = 0u;
        while(1)
        {
            std::ostringstream oss;
            oss<<strDBPath<<"_"<<nCount<<".db";
            nRes = remove(oss.str().c_str());
            if(nRes == -1)
            {
                if(errno == 2)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            nCount++;
        }
    }

    // open database
    bool DEUNetwork::OpenDB(const std::string& strDBPath)
    {
        int nPos = strDBPath.rfind("/");
        int nPos2 = strDBPath.rfind("\\");
        if(nPos == -1 && nPos2 == -1)
        {
            return false;
        }

        nPos = nPos > nPos2 ? nPos : nPos2;

        std::string strPath = "";
        strPath = strDBPath.substr(0,nPos);
        int nRes = _access(strPath.c_str(),0);
        if(nRes == -1)
        {
            return false;
        }

        unsigned __int64 nVersion = 0ui64;
        int nErrorCode = DEU_SUCCESS;
        bool bNeedRemove = true;
        if(!m_queryData.GetCacheVersion(nVersion,nErrorCode))
        {
            //removeCache(strDBPath);
            bNeedRemove = false;
        }

        m_pDBProxy = deudbProxy::createDEUDBProxy();
        if(!m_pDBProxy.valid())
        {
            return false;
        }

        if(!m_pDBProxy->openDB(strDBPath, g_nReadBufferSize, g_nWriteBufferSize))
        {
            m_pDBProxy = NULL;
            return false;
        }

        ID id(~0ui64,~0ui64,~0ui64);
        void* pBuffer = NULL;
        unsigned nLength = 0u;
        if(!m_pDBProxy->readBlock(id,pBuffer,nLength) || pBuffer == NULL)
        {
            if(bNeedRemove)
            {
                m_pDBProxy->closeDB();
                removeCache(strDBPath);
                if(!m_pDBProxy->openDB(strDBPath,g_nReadBufferSize,g_nWriteBufferSize))
                {
                    m_pDBProxy = NULL;
                    return false;
                }
            }
            m_pDBProxy->replaceBlock(id,&nVersion,sizeof(nVersion));
        }
        else
        {
            unsigned __int64 nTemp = *(unsigned __int64*)pBuffer;
            if(nTemp != nVersion)
            {
                if(bNeedRemove)
                {
                    m_pDBProxy->closeDB();
                    removeCache(strDBPath);
                    if(!m_pDBProxy->openDB(strDBPath,g_nReadBufferSize,g_nWriteBufferSize))
                    {
                        m_pDBProxy = NULL;
                        return false;
                    }
                }
                m_pDBProxy->replaceBlock(id,&nVersion,sizeof(nVersion));
            }
        }
        return true;
    }

    bool DEUNetwork::addVirtTile(const void* pBuffer,unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(pBuffer == NULL || nBufLen == 0)
            {
                nError = DEU_EMPTY_DATA;
            }
            else
            {
                const char *pStream = (const char *)pBuffer;
                std::vector<char>   vecStream(pStream, pStream + nBufLen);
                bRes = m_queryData.AddVirtTile(vecStream, m_strTicket, errVec, nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }

            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    //更新数据
    bool DEUNetwork::updateData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.UpdateData(id,m_strTicket,pBuffer,nBufLen,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    //替换数据
    bool DEUNetwork::replaceData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.ReplaceData(id,m_strTicket,pBuffer,nBufLen,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    //添加数据
    bool DEUNetwork::addData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.AddData(id,m_strTicket,pBuffer,nBufLen,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    // 添加图层
    bool DEUNetwork::addLayer(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid() || !idParent.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else if(pBuffer == NULL || nLength == 0)
            {
                nError = DEU_EMPTY_DATA;
            }
            else
            {
                bRes = m_queryData.AddLayer(id,idParent,m_strTicket,pBuffer,nLength,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    // 更新图层
    bool DEUNetwork::updateLayer(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else if(pBuffer == NULL || nLength == 0)
            {
                nError = DEU_EMPTY_DATA;
            }
            else
            {
                bRes = m_queryData.UpdateLayer(id,m_strTicket,pBuffer,nLength,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    // 添加图层
    bool DEUNetwork::addCategory(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid() || !idParent.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else if(pBuffer == NULL || nLength == 0)
            {
                nError = DEU_EMPTY_DATA;
            }
            else
            {
                bRes = m_queryData.AddCategory(id,idParent,pBuffer,nLength,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    // 更新图层
    bool DEUNetwork::updateCategory(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else if(pBuffer == NULL || nLength == 0)
            {
                nError = DEU_EMPTY_DATA;
            }
            else
            {
                bRes = m_queryData.UpdateCategory(id,pBuffer,nLength,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    // 添加属性
    bool DEUNetwork::addProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.AddProperty(id,strProperty,m_strTicket,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::updateProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.UpdateProperty(id,strProperty,m_strTicket,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    unsigned DEUNetwork::putRequestIntoList(const ID &id)
    {
        const unsigned nReqID = genUniqueID();

        OpenSP::sp<DownloadResult>  pResultItem = new DownloadResult;
        pResultItem->m_bSuccess = false;
        pResultItem->m_blockFinished.set(false);

        // 从在成果容器中开辟一项空间
        m_mtxDownloadResult.lock();
        m_mapDownloadResult[nReqID] = pResultItem;
        m_mtxDownloadResult.unlock();

        // 将请求放入请求队列
        m_mtxRequestQueue.lock();
        m_listRequestQueue.push_back(RequestItem(id, nReqID));
        m_mtxRequestQueue.unlock();

        // 唤醒下载线程
        m_vecDownloadingThread[m_nThread]->suspend(false);
        m_nThread = (m_nThread + 1) % THREADCOUNT;
        return nReqID;
    }

    bool DEUNetwork::queryData(const ID &id, void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep, unsigned nVersion)
    {
        if(!id.isValid())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_ID);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_ID));
            }
            return false;
        }
        if(m_pDBProxy != NULL)
        {
            if(m_pDBProxy->readBlock(id,pBuffer,nBufLen))
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK+DEU_SUCCESS);
                    pOutExcep->setMessage(GetErrDesc(DEU_SUCCESS));
                }
                return true;
            }
        }
        
        std::map<unsigned __int64,deues::ITileSet*>::const_iterator pItr = m_tileSetMap.find(id.TileID.m_nUniqueID);
        if(pItr == m_tileSetMap.end())//内部数据
        { 
            if(canUseNetwork())
            {
                int nError = DEU_SUCCESS;
                bool bRetValue = false;
                // 将请求放入请求队列，返回一个下载序列号
                const unsigned nReqID = putRequestIntoList(id);
                // 等待下载完成
                m_mtxDownloadResult.lock();
                OpenSP::sp<DownloadResult> &pItem = m_mapDownloadResult[nReqID];
                m_mtxDownloadResult.unlock();
                pItem->m_blockFinished.block();

                // 从成果容器中删除本下载项
                // 取出下载的结果
                if(pItem->m_bSuccess)
                {
                    nBufLen = pItem->m_vecBuffer.size();
                    pBuffer = malloc(nBufLen);
                    memcpy(pBuffer,pItem->m_vecBuffer.data(),nBufLen);
                    bRetValue = true;

                    if(m_pDBProxy != NULL)
                    {
                        m_pDBProxy->replaceBlock(id,pBuffer,nBufLen);
                    }
                }
                else
                {
                    nError = pItem->m_nErrorCode;
                }

                m_mtxDownloadResult.lock();
                m_mapDownloadResult.erase(nReqID);
                m_mtxDownloadResult.unlock();

                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK+nError);
                    pOutExcep->setMessage(GetErrDesc(nError));
                }

                return bRetValue;
            }
            else
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK + DEU_INVALID_NETWORK);
                    pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
                }
                return false;
            }
        }
        else//外部数据
        {
            int nError = DEU_SUCCESS;
            deues::ITileSet* pTileSet = pItr->second;
            bool bRes = pTileSet->queryData(id,pBuffer,nBufLen,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;            
        }

    }

    bool DEUNetwork::queryData(const ID& id,const std::string &strHost,const std::string &strPort,void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep, unsigned nVersion)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                std::vector<char>   vecBuffer;
                bRes = m_queryData.QueryData(id,nVersion,strHost,strPort, vecBuffer, nError);
                if(bRes)
                {
                    pBuffer = malloc(vecBuffer.size());
                    memcpy(pBuffer, vecBuffer.data(), vecBuffer.size());
                    nBufLen = vecBuffer.size();
                }
            }

            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false; 
        }
    }
    // 获取数据个数
    unsigned DEUNetwork::queryBlockCount(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            unsigned nBlock = m_queryData.QueryBlockCount(strHost,strPort,nDSCode,m_strTicket,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return nBlock;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return 0;
        }
        
    }
    bool DEUNetwork::queryVersion(const ID& id,std::vector<unsigned>& versionList,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        versionList.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = false;
            if(!id.isValid())
            {
                nError = DEU_INVALID_ID;
            }
            else
            {
                bRes = m_queryData.QueryVersion(id,versionList,nError);
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteLayerChildren(const std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = true;
            if(!idVec.empty())
            {
                bRes = m_queryData.DeleteLayerChildren(idVec,m_strTicket,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteData(const ID& id, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if (canUseNetwork())
        {
            std::vector<std::string> strVec = m_queryData.GetRcdUrl(id);
            if(strVec.empty())
            {
                return false;
            }

            std::vector<ID> idVec;
            std::vector<ID> errVec;
            idVec.push_back(id);

            for(unsigned k = 0;k < strVec.size();k++)
            {
                std::string strHost = strVec[k];
                std::string strIP   = strHost.substr(0, strHost.find(":"));
                std::string strPort = strHost.substr(strHost.find(":")+1);

                if (!deleteData(strHost, strPort, idVec, errVec, pOutExcep))
                {
                    return false;
                }
            }

            return true;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteData(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = true;
            if(!idVec.empty())
            {
                bRes = m_queryData.DelDatum(strHost,strPort,idVec,m_strTicket,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteAllData(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.DelAllData(strHost,strPort,nDSCode,m_strTicket,errVec,nError);
            if(bRes)
            {
                m_queryData.UpdateCacheVersion();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteProperty(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,const std::vector<ID>& idVec,
                                    std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = true;
            if(!idVec.empty())
            {
                bRes = m_queryData.DelProperty(strHost,strPort,nDSCode,idVec,m_strTicket,errVec,nError);
                if(bRes)
                {
                    m_queryData.UpdateCacheVersion();
                }
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteAllProperty(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.DelAllProperty(strHost,strPort,nDSCode,m_strTicket,nError);
            if(bRes)
            {
                m_queryData.UpdateCacheVersion();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::deleteDataSet(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.DelDataSet(nDataSet,strHost,strPort,m_strTicket,nError);
            if(bRes)
            {
                m_queryData.UpdateCacheVersion();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    bool DEUNetwork::deleteDataSetAttr(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.DelDataSetAttr(nDataSet,strHost,strPort,m_strTicket,nError);
            if(bRes)
            {
                m_queryData.UpdateCacheVersion();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    // 获取指定范围的ID
    bool DEUNetwork::queryIndices(const std::string& strHost,const std::string& strPort,unsigned nDSCode,
                                  unsigned nOffset,unsigned nCount,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        idVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.QueryIndices(strHost,strPort,nDSCode,m_strTicket,nOffset,nCount,idVec,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    // 根据属性获取ID
    bool DEUNetwork::queryIdByProperty(const std::string& strProperty, std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        idVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.QueryIdByProperty(strProperty,m_strTicket,idVec,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::queryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        idVec.clear();
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool bRes = m_queryData.QueryIdByProperty(strProperty,strHost,strPort,m_strTicket,idVec,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }


    bool DEUNetwork::startServiceFun(const std::string &strUrl,std::vector<std::string>& errVec,int& nErrorCode)
    {
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = bElem->Int32Value();
        if(nRetCode == 0)
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            if(bElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = bElem->Int32Value();
            }
            return false;
        }

        bElem = bsonDoc.GetElement("ErrPort");
        if(bElem != NULL && bElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)bElem;
            errVec.resize(pArrayElem->ChildCount());
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                errVec[i] = pChildElem->StrValue();
            }
        }
        else
        {
            nErrorCode = DEU_RECEIVE_NO_PARAM;
        }
        return true;
    }


    bool DEUNetwork::startService(const std::string &strIP,const std::string &strApachePort,const std::string &strPort,const std::string& strType,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(!canUseNetwork())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
        std::ostringstream oss;
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strIP<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrld?type="<<strType<<"&port="<<strPort<<'\0';
#else
        oss<<"http://"<<strIP<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrl?type="<<strType<<"&port="<<strPort<<'\0';
#endif
        std::vector<std::string> errVec;
        int nError = DEU_SUCCESS;
        if(!startServiceFun(oss.str(),errVec,nError))
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            errVec.push_back(strPort);
            return false;
        }
        else
        {
            if(!errVec.empty())
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK + DEU_FAIL_DATA_PORT);
                    pOutExcep->setMessage(GetErrDesc(DEU_FAIL_DATA_PORT));
                }
                return false;
            }
            else
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK+DEU_SUCCESS);
                    pOutExcep->setMessage(GetErrDesc(DEU_SUCCESS));
                }
                return true;
            }
        }
    }

    bool DEUNetwork::startService(const std::string &strIP,const std::string &strApachePort,const std::vector<std::string>& strPortVec,
        const std::string& strType,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        errVec.clear();
        if(strPortVec.empty())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_EMPTY_PORTS);
                pOutExcep->setMessage(GetErrDesc(DEU_EMPTY_PORTS));
            }
            return false;
        }
        if(!canUseNetwork())
        {
            errVec = strPortVec;
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
        std::string strPort = "";
        for(unsigned n = 0;n < strPortVec.size()-1;n++)
        {
            strPort += strPortVec[n] + "-";
        }
        strPort += strPortVec[strPortVec.size() - 1];

        //url
        std::ostringstream oss;
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strIP<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrld?type="<<strType<<"&port="<<strPort<<'\0';
#else
        oss<<"http://"<<strIP<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrl?type="<<strType<<"&port="<<strPort<<'\0';
#endif
        int nError = DEU_SUCCESS;
        if(startServiceFun(oss.str(),errVec,nError))
        {
            if(errVec.empty())
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK+DEU_SUCCESS);
                    pOutExcep->setMessage(GetErrDesc(DEU_SUCCESS));
                }
                return true;
            }
            else
            {
                if(pOutExcep.valid())
                {
                    pOutExcep->setReturnCode(EC_NET_WORK+DEU_PART_SUCCESS);
                    pOutExcep->setMessage(GetErrDesc(DEU_PART_SUCCESS));
                }
                return true;
            }
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            errVec = strPortVec;
            return false;
        }
    }

    // 查询开启的端口
    bool DEUNetwork::getActivePorts(const std::string& strHost,const std::string& strApachePort,std::vector<std::string>& strPortVec,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        strPortVec.clear();

        if(!canUseNetwork())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
        //url
        std::ostringstream oss;
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrld?type=getActivePorts"<<"&port=all"<<'\0';
#else
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrl?type=getActivePorts"<<"&port=all"<<'\0';
#endif
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK + nRet);
                pOutExcep->setMessage(GetErrDesc(nRet));
            }
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_UNKNOWN);
                pOutExcep->setMessage(GetErrDesc(DEU_UNKNOWN));
            }
            return false;
        }

        const int nRetCode = bElem->Int32Value();
        if(nRetCode == 0)
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nValue = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nValue = bElem->Int32Value();
            }
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nValue);
                pOutExcep->setMessage(GetErrDesc(nValue));
            }
            return false;
        }

        bElem = bsonDoc.GetElement("Port");
        if(bElem != NULL)
        {
            if(bElem->GetType() == bson::bsonArrayType)
            {
                bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)bElem;
                for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
                {
                    bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                    if(pChildElem == NULL)
                    {
                        continue;
                    }

                    const std::string strPort = pChildElem->StrValue();
                    if (std::find(strPortVec.begin(), strPortVec.end(), strPort) == strPortVec.end())
                    {
                        strPortVec.push_back(strPort);
                    }
                }
            }
        }

        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + DEU_SUCCESS);
            pOutExcep->setMessage(GetErrDesc(DEU_SUCCESS));
        }

        return true;
    }


    void DEUNetwork::setOffLineMode(bool bOffLine)
    {
        if(bOffLine == m_bOffLineMode)  return;
        m_bOffLineMode = bOffLine;

        if(bOffLine)
        {
            m_pNetworkCheckerThread->finishMission();
            delete m_pNetworkCheckerThread;
            m_pNetworkCheckerThread = NULL;
        }
        else
        {
            m_pNetworkCheckerThread = new NetworkCheckerThread(this);
            m_pNetworkCheckerThread->startThread();
        }
    }

    void DEUNetwork::NetworkCheckerThread::run(void)
    {
        while(0u == (unsigned)m_MissionFinished)
        {
            const unsigned nStatus = cmm::checkNetworkStatus();
            m_pNetwork->m_bNetworkHealthy = (nStatus == 0);
            m_block.block(10u * 1000u);
        }
    }

    bool DEUNetwork::getOffLineMode(void) const
    {
        return ((unsigned)m_bOffLineMode == 1u);
    }

    bool DEUNetwork::canUseNetwork(void) const
    {
        if(!m_bNetworkHealthy)
        {
            return false;
        }

        if(m_bOffLineMode)
        {
            return false;
        }

        return true;
    }

    unsigned calcRequestSize(const ID &id)
    {
        // 本函数根据ID类型，估测一个请求的大小，以KB为单位
        switch(id.ObjectID.m_nType)
        {
            case VIRTUAL_CUBE:                  return 2u;
            case TERRAIN_TILE:                  return 1u;
            case TERRAIN_TILE_HEIGHT_FIELD:     return 4u;
            case TERRAIN_TILE_IMAGE:            return 192u;
            case MODEL_ID:                      return 1024u;
            case IMAGE_ID:                      return 1024u;
            case SHARE_IMAGE_ID:                return 1024u;
            case SHARE_MODEL_ID:                return 1024u;
            case CULTURE_LAYER_ID:              return 1024u;
            case TERRAIN_DEM_LAYER_ID:          return 2u;
            case TERRAIN_DOM_LAYER_ID:          return 2u;
            case SYMBOL_CATEGORY_ID:            return 1024u;
            case TERRAIN_DEM_ID:                return 1024u;
            case TERRAIN_DOM_ID:                return 1024u;
            case SYMBOL_ID:                     return 1u;
            case PARAM_POINT_ID:                return 1u;
            case PARAM_LINE_ID:                 return 1u;
            case PARAM_FACE_ID:                 return 1u;
            case DETAIL_PIPE_CONNECTOR_ID:      return 1u;
            case DETAIL_CUBE_ID:                return 1u;
            case DETAIL_CYLINDER_ID:            return 1u;
            case DETAIL_PRISM_ID:               return 1u;
            case DETAIL_PYRAMID_ID:             return 1u;
            case DETAIL_SPHERE_ID:              return 1u;
            case DETAIL_SECTOR_ID:              return 1u;
            case DETAIL_STATIC_MODEL_ID:        return 1u;
            case DETAIL_DYN_POINT_ID:           return 1u;
            case DETAIL_DYN_LINE_ID:            return 1u;
            case DETAIL_DYN_FACE_ID:            return 1u;
            case DETAIL_DYN_IMAGE_ID:           return 1u;
            case DETAIL_BUBBLE_TEXT_ID:         return 1u;
            case DETAIL_POLYGON_ID:             return 1u;
            case DETAIL_ROUND_TABLE_ID:         return 1u;
			case DETAIL_DYN_POINT_CLOUD_ID:		return 1u;
            default:                            return 5u;
        }

        return 5u;
    }

    void DEUNetwork::fetchRequest(std::list<RequestItem> &listRequests,std::string& strHost)
    {
        listRequests.clear();
        strHost.clear();

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxRequestQueue);

        if(m_listRequestQueue.empty())
        {
            return;
        }

        const unsigned nMostRequestSize = 8192u;    // 最多只允许一次下载8192KB的请求

        // 从m_listRequestQueue中取出本次批量可以下载的请求项
        // 规则是：
        // 1、从m_listRequestQueue的取出最后一项，看它可以从几个服务器中下载
        // 2、随机选择一个服务器selectedServer
        // 3、查找m_listRequestQueue中的所有请求，凡是能从selectedServer中下载的请求，全部剪切放入listRequests
        // 4、返回listRequests
        unsigned nTotalReqSize = 0u;
        std::list<RequestItem>::const_iterator itor = m_listRequestQueue.cbegin();
        while (itor != m_listRequestQueue.cend() && (nTotalReqSize < nMostRequestSize))
        {
            const RequestItem &item = *itor;
            const ID &id = item.first;
            const unsigned nReqSize = calcRequestSize(id);
            if(nTotalReqSize + nReqSize >= nMostRequestSize)
            {
                ++itor;
                continue;
            }

            const std::vector<std::string> vecServers = m_queryData.GetRcdUrl(itor->first);
            if(vecServers.empty())
            {
                listRequests.push_back(*itor);
                itor = m_listRequestQueue.erase(itor);
                nTotalReqSize += nReqSize;
                continue;
            }

            if(strHost.empty())
            {
                const unsigned nServer = rand() % vecServers.size();
                strHost = vecServers[nServer];

                listRequests.push_back(*itor);
                itor = m_listRequestQueue.erase(itor);
                nTotalReqSize += nReqSize;
                continue;
            }

            if(std::find(vecServers.cbegin(),vecServers.cend(), strHost) != vecServers.cend())
            {
                listRequests.push_back(*itor);
                itor = m_listRequestQueue.erase(itor);
                nTotalReqSize += nReqSize;
                continue;
            }
            itor++;
        }
    }

    bool DEUNetwork::queryDatum(const std::string& strHost,const std::vector<ID> &idVec,std::vector<char> &vecBuffer)
    {
        vecBuffer.clear();
        int nError = DEU_SUCCESS;
        if(!canUseNetwork())
        {
            nError = DEU_INVALID_NETWORK;
            return false;
        }
        bool bSucceeded = false;
        try
        {
            bSucceeded = m_queryData.QueryDatum(strHost,idVec,m_strTicket,vecBuffer,nError);
        }
        catch(...)
        {
            std::cout << "Some exception occured in queryDatum of Network transporting.\n";
            bSucceeded = false;
        }

        return bSucceeded;
    }

    void DEUNetwork::DownloadingThread::run(void)
    {
        struct Transformer
        {
            const ID &operator()(const RequestItem &item) const
            {
                return item.first;
            }
        };
        std::vector<char> vecDownloadBuffer;
        while((unsigned)m_MissionFinished == 0u)
        {
            // 取出本次批量下载的所有ID
            std::list<RequestItem> listCurrentReqs;
            std::string strHost = "";
             m_pThis->fetchRequest(listCurrentReqs,strHost);

            if(listCurrentReqs.empty())
            {
                suspend(true);
                m_block.block();
                continue;
            }

            // 批量下载
            std::vector<ID> idVec(listCurrentReqs.size());
            std::transform(listCurrentReqs.begin(), listCurrentReqs.end(), idVec.begin(), Transformer());

            std::vector<char> vecDownloadBuffer;
            const bool bQuery = m_pThis->queryDatum(strHost,idVec,vecDownloadBuffer);
            if(!bQuery)
            {
                downloadingResultFailed(listCurrentReqs);
                continue;
            }

            // 将下载的结果存入成果容器
            bson::bsonDocument bDoc;
            const bool bConv2Bson = bDoc.FromBsonStream(vecDownloadBuffer.data(), vecDownloadBuffer.size());
            if(!bConv2Bson)
            {
                downloadingResultFailed(listCurrentReqs);
                continue;
            }

            std::list<RequestItem>::const_iterator itor = listCurrentReqs.cbegin();
            while(itor != listCurrentReqs.cend())
            {
                m_pThis->m_mtxDownloadResult.lock();
                OpenSP::sp<DownloadResult> &pItem = m_pThis->m_mapDownloadResult[itor->second];

                bson::bsonElement* pChildElem = bDoc.GetElement(itor->first.toString().c_str());
                if(pChildElem == NULL)
                {
                    pItem->m_bSuccess = false;
                    pItem->m_nErrorCode = DEU_FAIL_READ_BLOCK;
                }
                else if(pChildElem->GetType() == bson::bsonBinType)
                {
                    bson::bsonBinaryEle* pBinElem = (bson::bsonBinaryEle*)pChildElem;
                    pItem->m_bSuccess = true;
                    pItem->m_nErrorCode = DEU_SUCCESS;
                    const char *pBuffer = (const char *)pBinElem->BinData();
                    pItem->m_vecBuffer.assign(pBuffer, pBuffer + pBinElem->BinDataLen());
                }
                else if(pChildElem->GetType() == bson::bsonInt32Type)
                {
                    bson::bsonInt32Ele* pIntElem = (bson::bsonInt32Ele*)pChildElem;
                    pItem->m_bSuccess = false;
                    pItem->m_nErrorCode = pIntElem->Int32Value();
                }
                else
                {
                    pItem->m_bSuccess = false;
                    pItem->m_nErrorCode = DEU_FAIL_READ_BLOCK;
                }
                m_pThis->m_mtxDownloadResult.unlock();
                pItem->m_blockFinished.release();
                itor++;
            }
        }
    }

    void DEUNetwork::DownloadingThread::downloadingResultFailed(const std::list<RequestItem> &listCurrentReqs)
    {
        std::list<RequestItem>::const_iterator itor = listCurrentReqs.cbegin();
        while(itor != listCurrentReqs.cend())
        {
            m_pThis->m_mtxDownloadResult.lock();
            OpenSP::sp<DownloadResult> &pItem = m_pThis->m_mapDownloadResult[itor->second];
            pItem->m_bSuccess = false;
            m_pThis->m_mtxDownloadResult.unlock();

            pItem->m_blockFinished.release();
            itor++;
        }
    }

    bool DEUNetwork::login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            std::vector<std::string> strPermVec;
            bool  bRes = m_queryData.Login(strHost,strPort,strUser,strPwd,strPermVec,m_strTicket,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            if(bRes)
            {
                m_strUser = strUser;
                m_bLogin = true;
                m_strAuthHost = strHost;
                m_strAuthPort = strPort;
            }
            else
            {
                m_strTicket = m_strUser = "";
                m_bLogin = false;
                m_strAuthHost = m_strAuthPort = "";
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::logout(OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool  bRes = m_queryData.Logout(m_strAuthHost,m_strAuthPort,m_strTicket,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            m_strTicket = "";
            m_strUser = "";
            m_bLogin = false;
            m_strAuthHost = m_strAuthPort = "";
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::authPerm(const std::string& strPerm,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool  bRes = m_queryData.AuthPerm(m_strAuthHost,m_strAuthPort,m_strTicket,strPerm,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }
    bool DEUNetwork::authRes(const unsigned nDB,OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        if(canUseNetwork())
        {
            int nError = DEU_SUCCESS;
            bool  bRes = m_queryData.AuthRes(m_strAuthHost,m_strAuthPort,m_strTicket,nDB,nError);
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+nError);
                pOutExcep->setMessage(GetErrDesc(nError));
            }
            return bRes;
        }
        else
        {
            if(pOutExcep.valid())
            {
                pOutExcep->setReturnCode(EC_NET_WORK+DEU_INVALID_NETWORK);
                pOutExcep->setMessage(GetErrDesc(DEU_INVALID_NETWORK));
            }
            return false;
        }
    }

    void DEUNetwork::addTileSet(deues::ITileSet* pTileSet)
    {
        const unsigned __int64 nUniqueID = pTileSet->getUniqueFlag();
        m_tileSetMap[nUniqueID] = pTileSet;
    }

    void DEUNetwork::removeTileSet(deues::ITileSet* pTileSet)
    {
        const unsigned __int64 nUniqueID = pTileSet->getUniqueFlag();
        m_tileSetMap.erase(nUniqueID);
    }

//增加、删除服务配置 liubo 20151116
    bool DEUNetwork::addServer(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        int nErr = -1;

        //url
        std::ostringstream oss;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        //get return code
        bson::bsonDocument bsonDoc;

        bson::bsonElement* bElem = NULL;

        bool b = false;

        if (!canUseNetwork())
        {
            nErr = DEU_INVALID_NETWORK;
            goto ERR;
        }

        if(strHost == "" || strApachePort == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }

        //检测服务是否可用
        if(!checkApache(strHost, strApachePort, pOutExcep))
        {
            return false;
        }

        
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type=addServerInfo"<<"&host="<<strHost<<"&apacheport="<<strApachePort<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=addServerInfo"<<"&host="<<strHost<<"&apacheport="<<strApachePort<<'\0';
#endif
        
        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            goto ERR;
        }


        //解析返回数据
        b = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        if(!b) 
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        if(bElem->Int32Value() == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nErr = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nErr = bElem->Int32Value();
            }
            goto ERR;
        }

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }

    bool DEUNetwork::delServer(const std::string& strHost, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        int nErr = -1;

        //url
        std::ostringstream oss;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        //get return code
        bson::bsonDocument bsonDoc;

        bson::bsonElement* bElem = NULL;

        bool b = false;

        if (!canUseNetwork())
        {
            nErr = DEU_INVALID_NETWORK;
            goto ERR;
        }
        if(strHost == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }
        
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type=delServerInfo"<<"&host="<<strHost<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=delServerInfo"<<"&host="<<strHost<<'\0';
#endif
        
        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            goto ERR;
        }

        //解析返回数据
        b = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        if(!b) 
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        if(bElem->Int32Value() == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nErr = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nErr = bElem->Int32Value();
            }
            goto ERR;
        }

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }


    bool DEUNetwork::addService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        int nErr = -1;

        //url
        std::ostringstream oss;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        //get return code
        bson::bsonDocument bsonDoc;

        bson::bsonElement* bElem = NULL;

        bool b = false;

        if (!canUseNetwork())
        {
            nErr = DEU_INVALID_NETWORK;
            goto ERR;
        }
        if(strHost == "" || strApachePort == "" || strSerPort == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }
        //检测服务是否可用
        if(!checkApache(strHost, strApachePort, pOutExcep))
        {
            return false;
        }

        
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type=addServiceInfo"<<"&host="<<strHost<<"&apacheport="<<strApachePort<<"&addport="<<strSerPort<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=addServiceInfo"<<"&host="<<strHost<<"&apacheport="<<strApachePort<<"&addport="<<strSerPort<<'\0';
#endif
        
        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            goto ERR;
        }


        //解析返回数据
        b = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        if(!b) 
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        if(bElem->Int32Value() == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nErr = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nErr = bElem->Int32Value();
            }
            goto ERR;
        }

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }

    bool DEUNetwork::delService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {

        int nErr = -1;

        //url
        std::ostringstream oss;

        std::vector<std::string> errVec;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        //get return code
        bson::bsonDocument bsonDoc;

        bson::bsonElement* bElem = NULL;

        bool b = false;

        if (!canUseNetwork())
        {
            nErr = DEU_INVALID_NETWORK;
            goto ERR;
        }

        if(strHost == "" || strSerPort == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }

        
        //删除服务前先停止服务
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrld?type=stop"<<"&port="<<strSerPort<<'\0';
#else
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrl?type=stop"<<"&port="<<strSerPort<<'\0';
#endif
        
        if(!startServiceFun(oss.str(),errVec,nErr))
        {
            goto ERR;
        }

        oss.str("");
        //删除数据端口配置
#ifdef _SERVER_DEBUG_
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConfd?type=delServiceInfo"<<"&host="<<strHost<<"&port="<<strSerPort<<'\0';
#else
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=delServiceInfo"<<"&host="<<strHost<<"&port="<<strSerPort<<'\0';
#endif
        
        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            goto ERR;
        }


        //解析返回数据
        b = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        if(!b) 
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        if(bElem->Int32Value() == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nErr = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nErr = bElem->Int32Value();
            }
            goto ERR;
        }

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }

 
    //检测apache主服务是否启动 liubo 20151116
    bool DEUNetwork::checkApache(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {
        
        int nErr = -1;

        std::ostringstream oss;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        if(strHost == "" || strApachePort == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }

#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUServerConfd?type=getServerInfo"<<'\0';
#else
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUServerConf?type=getServerInfo"<<'\0';
#endif
        
        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            goto ERR;
        }

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }

    //检测数据端口是否启动  liubo 20151118
    bool DEUNetwork::checkPortIsActive(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep)
    {

        int nErr = -1;

        std::ostringstream oss;

        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;

        //get return code
        bson::bsonDocument bsonDoc;

        bson::bsonElement* bElem = NULL;

        bool b = false;

        if(strHost == "" || strApachePort == "" || strSerPort == "")
        {
            nErr = DEU_INVALID_REQUEST_PARAM;
            goto ERR;
        }

#ifdef _SERVER_DEBUG_
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrld?type=checkPortIsActive"<<"&port="<<strSerPort<<'\0';
#else
        oss<<"http://"<<strHost<<":"<<strApachePort<<"/cgi-bin/DEUDataPubCtrl?type=checkPortIsActive"<<"&port="<<strSerPort<<'\0';
#endif

        nErr = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())//连接失败
        {
            goto ERR;
        }
     
        //解析返回数据
        b = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());

        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();

        if(!b) 
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        bElem = bsonDoc.GetElement("RetCode");
        if(bElem == NULL)
        {
            nErr = DEU_UNKNOWN;
            goto ERR;
        }

        if(bElem->Int32Value() == 0) //error
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            int nErr = DEU_UNKNOWN;
            if(bElem != NULL)
            {
                nErr = bElem->Int32Value();
            }
            goto ERR;
        }

        return true;

ERR:
        if(pOutExcep.valid())
        {
            pOutExcep->setReturnCode(EC_NET_WORK + nErr);
            pOutExcep->setMessage(GetErrDesc(nErr));
        }
        return false;

    }



}