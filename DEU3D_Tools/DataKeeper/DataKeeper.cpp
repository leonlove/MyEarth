// DataKeeper.cpp : 定义 DLL 应用程序的导出函数。
//

#include "DataKeeper.h"
#include "time.h"
#include "DEUCheck.h"
#include "DEULog.h"
#include <Common/DEUBson.h>
#include "DEUUtils.h"
#include <sys/stat.h>
#include <io.h>

namespace dk
{

    DATAKEEPER_API dk::IDataKeeper *CreateDataKeeper(void)
    {
        OpenSP::sp<dk::IDataKeeper> pIDataKeeper = dynamic_cast<dk::IDataKeeper*>(new DataKeeper());
        return pIDataKeeper.release();
    }

    DataKeeper::DataKeeper(void)
    {
        m_strRootHost = "";
        m_strRootPort = "";
        m_bIsBatchUpload = false;

        m_pIDataSetManager = NULL;
        m_pIServiceManager = NULL;
        m_pIUrlNetwork     = NULL;
    }

    DataKeeper::~DataKeeper(void)
    {
        m_pIDataSetManager = NULL;
        m_pIServiceManager = NULL;
        m_pIUrlNetwork     = NULL;
    }

    bool DataKeeper::login(const std::string& strHost, const std::string& strPort, 
                           const std::string& strUser, const std::string& strPwd)
    {
        if (m_pIUrlNetwork == NULL)
        {
            m_pIUrlNetwork = deunw::createDEUNetwork();
        }

        return m_pIUrlNetwork->login(strHost, strPort, strUser, strPwd, NULL);
    }

    bool DataKeeper::logout()
    {
        if (m_pIUrlNetwork != NULL)
        {
            return m_pIUrlNetwork->logout();
        }

        return false;
    }

    bool DataKeeper::initialize(const std::string& strHost, const std::string& strPort, bool& bConnect)
    {
        m_strRootHost = strHost;
        m_strRootPort = strPort;

        /* if(!m_pIUrlNetwork->checkApache(m_strRootHost,m_strRootPort))
        {
        return false;
        }*/

        if (m_pIUrlNetwork == NULL)
        {
            m_pIUrlNetwork = deunw::createDEUNetwork();
        }

        if (!m_pIUrlNetwork->initialize(strHost, strPort, true))
        {
            bConnect = false;
        }
        else
        {
            bConnect = true;
        }

        if (m_pIDataSetManager == NULL)
        {
            m_pIDataSetManager = dk::CreateDataSetManager();
            m_pIDataSetManager->setDataKeeper(this);
        }
        if (m_pIServiceManager == NULL)
        {
            m_pIServiceManager = dk::CreateServiceManager();
            m_pIServiceManager->setDataKeeper(this);
        }

        m_pIDataSetManager->init();
        m_pIServiceManager->init();

        return true;

    }

    OpenSP::sp<dk::IDataSetManager> DataKeeper::getDataSetManager(void)
    {
        if (m_pIDataSetManager == NULL)
        {
            m_pIDataSetManager = dk::CreateDataSetManager();
            m_pIDataSetManager->init();
        }

        return m_pIDataSetManager;
    }

    OpenSP::sp<dk::IServiceManager> DataKeeper::getServiceManager(void)
    {
        if (m_pIServiceManager == NULL)
        {
            m_pIServiceManager = dk::CreateServiceManager();
            m_pIServiceManager->init();
        }

        return m_pIServiceManager;
    }

    void DataKeeper::setTotalFiles(const std::string& sTotalFiles)
    {
        string sTotalFilesEx(sTotalFiles);

        std::set<string> setToatlFiles;
        while (sTotalFilesEx.find_first_of(";") != -1)
        {
            string sCurFile = sTotalFilesEx.substr(0, sTotalFilesEx.find_first_of(";"));
            setToatlFiles.insert(sCurFile);
            sTotalFilesEx = sTotalFilesEx.substr(sTotalFilesEx.find_first_of(";") + 1);
        }

        CDeuLog::getDefaultInstance()->WriteFilePath(setToatlFiles);
    }

    void DataKeeper::commitBegin()
    {
        CDeuLog::getDefaultInstance()->OpenLogFile();
        CDeuLog::getDefaultInstance()->WriteLogInfo("<DEULog>");
    }

    void DataKeeper::commitEnd()
    {
        CDeuLog::getDefaultInstance()->WriteLogInfo("<DEULog>");
        CDeuLog::getDefaultInstance()->CloseLogFile();
    }

    bool DataKeeper::readConfigFile(const std::string& sConfigFile)
    {
        return CDeuLog::getDefaultInstance()->ReadLogFile(sConfigFile);;
    }

    std::vector<std::string> DataKeeper::getCommitSuccessFiles()
    {
        return CDeuLog::getDefaultInstance()->GetCommitSuccessFiles();
    }

    std::vector<std::string> DataKeeper::getCommitFailedFiles()
    {
        return CDeuLog::getDefaultInstance()->GetCommitFailedFiles();
    }

    std::vector<std::string> DataKeeper::getCommitNoneFiles()
    {
        return CDeuLog::getDefaultInstance()->GetCommitNoneFiles();
    }

    std::vector<std::string> DataKeeper::getCommitAllFiles()
    {
        return CDeuLog::getDefaultInstance()->GetCommitAllFiles();
    }

    unsigned int DataKeeper::getFileCommitState(const std::string& sCurCommitFile)
    {
        return CDeuLog::getDefaultInstance()->GetFileCommitState(sCurCommitFile);
    }

    void DataKeeper::clearAllFiles()
    {
        CDeuLog::getDefaultInstance()->ClearCommitAllFiles();
    }

    bool DataKeeper::addServer(std::string& szHost, std::string& ApachePort, std::string& port)
    {
        int nError = 1;
        if (!m_pIUrlNetwork->startService(szHost, ApachePort, port,"add"))
        {
            return false;
        }

        DEUUrlServer urlServer(m_pIUrlNetwork);
        urlServer.initialize(m_strRootHost, m_strRootPort, false);

        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        bool isNotFind = true;
        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            if((strcmp(itr->first.c_str(), szHost.c_str()) == 0) &&
               (strcmp(itr->second.m_strApachePort.c_str(), ApachePort.c_str()) == 0))
            {
                for(int k=0; k<itr->second.m_portVec.size(); k++)
                {
                    if(strcmp(itr->second.m_portVec[k].c_str(), port.c_str()) == 0)
                    {
                        isNotFind = false;
                    }
                }
                if(isNotFind)
                { //仅端口不存在的追加
                    itr->second.m_portVec.push_back(port);
                    urlServer.addDataServer(szHost, itr->second);
                    urlServer.submit(&nError);
                    return true;
                }
            }
        }

        //如果进入，则表示IP和端口都不存在
        if(isNotFind)
        {
            HostInfo _hostinfo;
            _hostinfo.m_strApachePort = ApachePort;
            _hostinfo.m_portVec.push_back(port);
            urlServer.addDataServer(szHost, _hostinfo);
            urlServer.submit(&nError);
            return true;
        }

        return true;
    }

    bool DataKeeper::delServer(std::string& szHost, std::string& ApachePort, std::string& port)
    {
        int nError = 1;
        if (!m_pIUrlNetwork->startService(szHost, ApachePort, port,"stop"))
        {
            return false;
        }

        DEUUrlServer urlServer(m_pIUrlNetwork);
        urlServer.initialize(m_strRootHost, m_strRootPort, false);

        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        bool isNotFind = false;
        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            if((strcmp(itr->first.c_str(), szHost.c_str()) == 0) &&
               (strcmp(itr->second.m_strApachePort.c_str(), ApachePort.c_str()) == 0))
            {
                std::vector<std::string>::iterator itVec = itr->second.m_portVec.begin();
                for(; itVec != itr->second.m_portVec.end(); itVec++)
                {
                    if(strcmp(itVec->c_str(), port.c_str()) == 0)
                    {
                        itr->second.m_portVec.erase(itVec);
                        urlServer.addDataServer(itr->first, itr->second);
                        isNotFind = true;
                        break;
                    }
                }
                if(isNotFind)
                {
                    urlServer.submit(&nError);
                    return true;
                }
            }
        }

        return true;
    }

    bool DataKeeper::getServer(std::vector<std::string>& vecSrvHost)
    {
        DEUUrlServer urlServer(m_pIUrlNetwork);
        urlServer.initialize(m_strRootHost, m_strRootPort, false);

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        std::map<std::string,HostInfo>::iterator itr(hostMap.begin());
        for(;itr != hostMap.end(); itr++)
        {
            std::string strSrvHost = "";
            for(int k=0; k<itr->second.m_portVec.size(); k++)
            {
                strSrvHost = itr->first.c_str();
                strSrvHost+= ":";
                strSrvHost+= itr->second.m_portVec[k].c_str();
                vecSrvHost.push_back(strSrvHost);
            }
        }

        return true;
    }

    void* DataKeeper::formJsonToBsonPtr(const std::string& sJsonString, unsigned int* pBuffLen)
    {
        bson::bsonDocument bsonDoc;
        void* pBuff;
        bool bret = bsonDoc.FromJsonString(sJsonString);
        if (bret)
        {
            bson::bsonStream bsonStream;
            bsonDoc.Write(&bsonStream);
            //pBuff = bsonStream.Data();

            *pBuffLen = bsonStream.DataLen();
            pBuff = new char[*pBuffLen];
            memcpy(pBuff, bsonStream.Data(), bsonStream.DataLen());
        }
        return pBuff;
    }

    void DataKeeper::exportRcdAndUrlFile(const std::string& sRcdAndUrlFile)
    {
        DEUUrlServer urlServer(m_pIUrlNetwork);
        urlServer.initialize(m_strRootHost, m_strRootPort, false);

        int nError=0;
        std::map<unsigned int,RcdInfo> rcdMap;
        std::map<std::string,HostInfo> hostMap;
        urlServer.getInfo(rcdMap, hostMap, &nError);

        std::ofstream ofWriter;
        chmod(sRcdAndUrlFile.c_str(), S_IWRITE);
        ofWriter.open(sRcdAndUrlFile.c_str(), ios_base::trunc);

        string strRcdInfo, strHostInfo;
        urlServer.getInfo(strRcdInfo, strHostInfo);
        ofWriter << strRcdInfo << endl;
        ofWriter << strHostInfo << endl;
        ofWriter.flush();

        ofWriter.close();
        chmod(sRcdAndUrlFile.c_str(), S_IREAD);

        return;
    }

    void DataKeeper::importRcdAndUrlFile(const std::string& sRcdAndUrlFile)
    {
        std::ifstream iReader;
        iReader.open(sRcdAndUrlFile.c_str());
        if (!iReader.is_open())
        {
            return;
        }
        
        //- 解析rcd配置文件
        string strRcdLine, strHostLine;
        getline(iReader, strRcdLine);
        getline(iReader, strHostLine);
        iReader.close();

        //- 导入的rcd
        std::map<unsigned int,RcdInfo> importRcdMap;
        DEUUtils::getRcdFromBson(strRcdLine,importRcdMap);

        //- 导入的url
        m_mapImportHost.clear();
        DEUUtils::getHostFromBson(strHostLine,m_mapImportHost);

        //- 用导入的rcd覆盖现有的rcd，导出至临时文件
        string strTempRcdFile = sRcdAndUrlFile.substr(0, sRcdAndUrlFile.find_last_of(".")) + "Temp.txt";
        exportTempRcdFile(strTempRcdFile, importRcdMap);

        return;
    }

    void DataKeeper::exportTempRcdFile(const std::string& sTempRcdFile, std::map<unsigned int,RcdInfo>& rcdMap)
    {
        std::ofstream ofWriter;
        //chmod(sTempRcdFile.c_str(), S_IWRITE);
        ofWriter.open(sTempRcdFile.c_str(), ios_base::trunc);
        
        //- 完整项
        std::map<unsigned int,RcdInfo>::iterator it = rcdMap.begin();
        for(; it!=rcdMap.end(); it++)
        {
            string strName   = it->second.m_strName.c_str();
            unsigned nDataSetID = it->first;
            ofWriter << "NameAndCode: " << strName << ":" << nDataSetID << endl;

            //- 散列区间
            unsigned int nURLTotal = it->second.m_urlInfoVec.size();
            std::vector<UrlInfo> vecUrl=it->second.m_urlInfoVec;
            for(int m=0; m<nURLTotal; m++)
            {
                ofWriter << "Percent: " << vecUrl[m].m_nStart << "~" << vecUrl[m].m_nEnd << "%" << endl;

                //- 服务器
                std::map<std::string,std::vector<std::string>> hostMap = vecUrl[m].m_urlMap;
                std::map<std::string,std::vector<std::string>>::iterator itHostMap(hostMap.begin());
                for(; itHostMap != hostMap.end(); itHostMap++)
                {
                    //- 服务器端口
                    std::vector<std::string> vecPort = itHostMap->second;
                    for(int k=0; k<vecPort.size(); k++)
                    {
                        ofWriter << "Host: " << itHostMap->first << ":" << vecPort[k] << endl;
                    }
                }
            }
            ofWriter << "----------------------------------" << endl;
        }

        ofWriter.close();
        //chmod(sTempRcdFile.c_str(), S_IREAD);
    }

    void DataKeeper::getImportUrl(std::map<std::string,HostInfo>& hostImportMap)
    {
        hostImportMap = m_mapImportHost;
    }

    std::string DataKeeper::getRootHost() 
    {
        return m_strRootHost;
    }

    std::string DataKeeper::getRootPort()
    {
        return m_strRootPort;
    }

    OpenSP::sp<deunw::IDEUNetwork> DataKeeper::getDEUNetWork()
    {
        return m_pIUrlNetwork;
    }


    //- 设置是否为批量上传
    void DataKeeper::setBatchUpload(const bool bBatch)
    {
        m_bIsBatchUpload = bBatch;
    }

    //- 获取是否批量上传
    bool DataKeeper::getBatchUpload()
    {
        return m_bIsBatchUpload;
    }

}

