#include "DEUDBClient.h"
#include "DEUShareMem.h"
#include <sstream>
#include <algorithm>
#include <OpenSP/sp.h>
#include "Common/crc.h"
#include <iostream>

namespace deudbProxy
{
	//create deudb client
	IDEUDBProxy *createDEUDBProxy(void)
	{
		OpenSP::sp<DEUDBClient> pFileCache = new DEUDBClient;
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

    void DEUDBProxyPulseThread::run(void)
    {
        while((unsigned)m_atom == 0)
        {
            DEUSem::ReleaseSem(m_pulseSemHnd);
            m_block.reset();
            if((unsigned)m_atom == 0)
            {
                m_block.block(m_nPulseSec*500);
            }
        }
    }

	DEUDBClient::DEUDBClient(void)
	{
		m_shm = new DEUShareMem();
		m_regShm = new DEUShareMem();
		m_eventClientHnd = m_eventSvrHnd = m_regHnd = m_svrRegHnd = m_startHnd = m_multiHnd = m_partHnd = NULL;
        m_pulseThread = NULL;
		m_strShmName = m_strDBPath = m_strShared = "";
		m_bReg = m_bUnReg = false;
	}


	DEUDBClient::~DEUDBClient(void)
	{
		closeDB();
		//1. delete pointer
		if(m_shm != NULL)
			delete m_shm;
		if(m_regShm != NULL)
			delete m_regShm;
		if(m_pulseThread != NULL)
            delete m_pulseThread;
		//3. set null
		m_eventClientHnd = m_eventSvrHnd = m_regHnd = m_svrRegHnd = m_startHnd = m_multiHnd = m_partHnd = NULL;
		m_strShmName = m_strDBPath = m_strShared = "";
	}
#ifdef WIN32
	std::string DEUDBClient::getExePath()
	{
		std::string strDllName = "";
#ifdef _DEBUG
		strDllName = "DEUDBProxyd.dll";
#else
		strDllName = "DEUDBProxy.dll";
#endif
		HMODULE hm = GetModuleHandle(strDllName.c_str());
		char fullPath[1024];
		GetModuleFileName(hm,fullPath,100);
		std::string strFullPath = fullPath;
		size_t nFind = strFullPath.rfind('\\');
		if(nFind == -1)
			return "";
		std::string strPath = strFullPath.substr(0,nFind);
#ifdef _DEBUG
		strPath += "\\DEUDBServerd.exe";
#else
		strPath += "\\DEUDBServer.exe";
#endif
		return strPath;
	}
#else
#define DEU_MAX_PATH 1024
	std::string DEUDBClient::GetExePath()
	{
		std::string strPath  = "";
		static  char path[DEU_MAX_PATH];
		int count = readlink("/proc/self/exe",path,DEU_MAX_PATH);
		if(count < 0 || count >= DEU_MAX_PATH)
			return strPath;
		path[count] = '\0';
		strPath = path;
		int nPos = strPath.rfind('/');
		strPath = strPath.substr(0,nPos+1);
#ifdef _DEBUG
		strPath += "DEUDBServerd";
#else
		strPath += "DEUDBServer";
#endif
		return strPath;
	}
#endif

	//open server
	bool DEUDBClient::openExistServer()
	{
		//1. get variables
		m_strSvrRegSem = m_strRegSem + "SvrRegSem";
		std::string strRegShm = m_strRegSem + "RegShm";
        m_strPulseSem = m_strRegSem + "PulseSem";

		//2. open register semahpore
		m_regHnd = DEUSem::OpenSem(m_strRegSem);
		if(m_regHnd == NULL)
		{
			DEUSem::CloseSem(m_partHnd,m_strPartSem);//close server particular semaphore
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_partHnd = m_startHnd =  NULL;
			return false;
		}

        m_pulseSemHnd = DEUSem::OpenSem(m_strPulseSem);
        if(m_pulseSemHnd == NULL)
        {
            DEUSem::CloseSem(m_partHnd,m_strPartSem);//close server particular semaphore
            DEUSem::CloseSem(m_regHnd,m_strRegSem);
            DEUSem::ReleaseSem(m_startHnd);
            DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
            m_partHnd = m_regHnd = m_startHnd =  NULL;
            return false;
        }

		//3. open server register semaphore
		m_svrRegHnd = DEUSem::OpenSem(m_strSvrRegSem);
		if(m_svrRegHnd == NULL)
		{
			DEUSem::CloseSem(m_partHnd,m_strPartSem);//close server particular semaphore
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
            DEUSem::CloseSem(m_pulseSemHnd,m_strPulseSem);
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_partHnd = m_startHnd = m_regHnd = m_pulseSemHnd = NULL;
			return false;
		}

		//4.open reg shared memroy
		if(!m_regShm->AtShm(strRegShm))
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			DEUSem::CloseSem(m_svrRegHnd,m_strSvrRegSem);
			DEUSem::CloseSem(m_partHnd,m_strPartSem);//close server particular semaphore
            DEUSem::CloseSem(m_pulseSemHnd,m_strPulseSem);
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd = m_partHnd = m_pulseSemHnd = NULL;
			return false;
		}

		//5. close server paritculare semaphore and release start semaphore
        if(m_pulseThread == NULL)
        {
            m_pulseThread = new DEUDBProxyPulseThread(m_nPulseSec,m_pulseSemHnd);
            m_pulseThread->startThread();
        }
        
		DEUSem::CloseSem(m_partHnd,m_strPartSem);
		//DEUSem::ReleaseSem(m_startHnd);
		return true;
	}

	//open new server
	bool DEUDBClient::openNewServer(UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize)
	{
		m_strSvrRegSem = m_strRegSem + "SvrRegSem";
		std::string strRegShm = m_strRegSem + "RegShm";
        m_strPulseSem = m_strRegSem + "PulseSem";

		//1. create client register semaphore
		m_regHnd = DEUSem::CreateSem(m_strRegSem,0);
		if(m_regHnd == NULL)
		{
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);
			m_startHnd =  NULL;
			return false;
		}

		//2.create reg shared memroy
		if(!m_regShm->CreateShm(strRegShm,1024))
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd = NULL;
			return false;
		}
		//get exe path
		std::string strPath = getExePath();

#ifdef WIN32
		//3. start server
		std::ostringstream oss;
		oss<<"\""<<m_strDBPath<<"\""<<" "<<nReadBufferSize<<" "<<nWriteBufferSize<<" "<<m_nPulseSec<<" "<<m_nPulseCount;
		int nInst = (int)ShellExecute(NULL,NULL,strPath.c_str(),oss.str().c_str(),NULL,SW_SHOW);
		if(nInst <= 32)
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			m_regShm->DestroyShm();
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd =  NULL;
			return false;
		}
#else
		//	int nInst = ForkServer(strPath);
		pid_t pid = fork();
		if( pid == 0 ) // this is the child process
		{
			pid_t pc = fork();
			if(pc == 0)
			{
				int nInst = execl(strPath.c_str(),strPath.c_str(),m_strDBPath.c_str(),NULL);
				if(nInst == -1)
					exit(-1);
				exit(1);
			}
			else if(pc > 0)//parent
				exit(0);
		}
		else if(pid == -1)
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			m_regShm->DestroyShm();
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd =  NULL;
			return false;
		}
#endif

		//4. wait server semaphore
		DEUSem::WaitSem(m_regHnd);//WaitForSingleObject(m_regHnd,INFINITE);

        m_pulseSemHnd = DEUSem::OpenSem(m_strPulseSem);
        if(m_pulseSemHnd == NULL)
        {
            DEUSem::CloseSem(m_regHnd,m_strRegSem);
            m_regShm->DestroyShm();
            DEUSem::ReleaseSem(m_startHnd);
            DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
            m_regHnd = m_startHnd =  NULL;
            return false;
        }

		//5. open server reg semaphore
		m_svrRegHnd = DEUSem::OpenSem(m_strSvrRegSem);
		if(m_svrRegHnd == NULL)
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			m_regShm->DestroyShm();
            DEUSem::CloseSem(m_pulseSemHnd,m_strPulseSem);
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd =  m_pulseSemHnd = NULL;
			return false;
		}
        
		//6. open particular semaphore
		m_partHnd = DEUSem::OpenSem(m_strPartSem);
		if(m_partHnd == NULL)
		{
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			DEUSem::CloseSem(m_svrRegHnd,m_strSvrRegSem);
			m_regShm->DestroyShm();
            DEUSem::CloseSem(m_pulseSemHnd,m_strPulseSem);
			DEUSem::ReleaseSem(m_startHnd);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);//close start semaphore
			m_regHnd = m_startHnd = m_svrRegHnd = m_pulseSemHnd = NULL;
			return false;
		}
        m_pulseThread = new DEUDBProxyPulseThread(m_nPulseSec,m_pulseSemHnd);
        m_pulseThread->startThread();
		//7. close server paritculare semaphore and release start semaphore
		DEUSem::CloseSem(m_partHnd,m_strPartSem);
		//DEUSem::ReleaseSem(m_startHnd);
		return true;
	}

  
	//start server
	bool DEUDBClient::openDB(const std::string& strDBPath, UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize)
	{
        if(strDBPath.empty())
        {
            return false;
        }

		//1.get varibles
		m_strDBPath = strDBPath;
#ifdef WIN32
        std::transform(strDBPath.begin(), strDBPath.end(), m_strDBPath.begin(), ::toupper);
        std::replace(m_strDBPath.begin(), m_strDBPath.end(), '/', '\\');
#endif
        m_nPulseSec = 1u;
#ifdef _DEBUG
        m_nPulseCount = 2000u;
#else
        m_nPulseCount = 5u;
#endif
        std::cout<<m_strDBPath<<std::endl;
        std::cout<<"time="<<m_nPulseSec<<"    count="<<m_nPulseCount<<std::endl;

        unsigned nReg = cmm::createHashCRC32(m_strDBPath.c_str(),m_strDBPath.length());

        std::ostringstream oss;
        oss<<nReg;
        m_strRegSem = oss.str();

     /*  m_strRegSem = strDBPath;
		std::replace(m_strRegSem.begin(),m_strRegSem.end(),'\\','?');
		std::replace(m_strRegSem.begin(),m_strRegSem.end(),'/','?');*/

		m_strStartSem = m_strRegSem + "StartSem";
		m_strPartSem = m_strRegSem + "ParticularSem";

		

		//2.create start server semaphore
		m_startHnd = DEUSem::CreateSem(m_strStartSem,1);		
		if(m_startHnd == NULL)
        {
			return false;
        }
		DEUSem::WaitSem(m_startHnd);

        if(m_strMultiSem != "")
        {
            m_multiHnd = DEUSem::OpenSem(m_strMultiSem);
            if(m_multiHnd != NULL)
            {
                DEUSem::ReleaseSem(m_startHnd);
                return true;
            }
        }

		//3.whether server is already started
		m_partHnd = DEUSem::OpenSem(m_strPartSem);
		if(m_partHnd != NULL)   //3.1 server is already started
        {
			if(!openExistServer())
            {
                return false;
            }
        }
		else    //3.2 start server
        {
			
			if(!openNewServer(nReadBufferSize,nWriteBufferSize))
            {
                return false;
            }
        }
		
        const unsigned nShareBufferSize = 4194304u;
		bool bRes = regShm(nShareBufferSize);
        DEUSem::ReleaseSem(m_startHnd);
        return bRes;
	}
	
	//register shm
	bool DEUDBClient::regShm(unsigned nSize/* = 1024u*/)
	{
		//0. if reg and no unreg,return true
		if(m_bReg && !m_bUnReg)
			return true;
		m_bReg = m_bUnReg = false;

		//1.get unique shm name
#ifdef WIN32
		UUID uuid;
		::UuidCreate(&uuid);
		unsigned char* chUUid = NULL;
		UuidToString(&uuid,&chUUid);
		m_strShared = (char*)chUUid;
#else
		uuid_t uuid;
		uuid_generate(uuid);
		char chUUid[124];
		uuid_unparse(uuid,chUUid);
		m_strShared = chUUid;
#endif
		m_strClientSem = m_strShared + "EventClient";//信号量
		m_strMultiSem = m_strShared + "EventMulti";//多进程信号量
		m_strShmName = m_strShared + "Shm";//共享内存

		
		//2.create client semaphore
		m_eventClientHnd = DEUSem::CreateSem(m_strClientSem,0);
		if(m_eventClientHnd == NULL)
		{
			return false;
		}

		//3. create shared memory
		m_nShmSize = nSize;
		if(!m_shm->CreateShm(m_strShmName,m_nShmSize))
		{
			DEUSem::CloseSem(m_eventClientHnd,m_strClientSem);
			return false;
		}

		//4.create multi-thread semaphore
		m_multiHnd = DEUSem::CreateSem(m_strMultiSem,1);
		if(m_multiHnd == NULL)
		{
			DEUSem::CloseSem(m_eventClientHnd,m_strClientSem);
			m_shm->DestroyShm();
			return false;
		}

		//5. write reg info
		if(!writeRegInfo(DEU_REG_SHM))
			return false;
		
		m_bReg = true;
		return true;
	}


	//unregister shm
	bool DEUDBClient::closeDB()
	{
        std::cout<<"close:"<<m_strDBPath<<std::endl;
		//0. if reged and unreged,unreg
		if(m_bReg && !m_bUnReg)
		{
			//1. wait multi-thread semaphore
			//   to make sure no other thread is working
			DEUSem::WaitSem(m_multiHnd);

			//2. close  all handle
			DEUSem::CloseSem(m_eventClientHnd,m_strClientSem);
			DEUSem::CloseSem(m_eventSvrHnd,m_strServerSem);
			DEUSem::CloseSem(m_multiHnd,m_strMultiSem);
			m_shm->DestroyShm();

			//3. write unreg info
			writeRegInfo(DEU_UNREG_SHM);

			//4.close sem and destroy shm
			DEUSem::CloseSem(m_regHnd,m_strRegSem);
			DEUSem::CloseSem(m_svrRegHnd,m_strSvrRegSem);
			DEUSem::CloseSem(m_startHnd,m_strStartSem);
			m_regShm->DestroyShm();
			
			m_bUnReg = true;

            if(m_pulseThread != NULL)
            {
                m_pulseThread->stopThread();
                m_pulseThread->join();
            }
            std::cout<<"execute close:"<<m_strDBPath<<std::endl;
		}
		return true;
	}

	//write reg info
	bool DEUDBClient::writeRegInfo(const int& nType)
	{
		//1. get reg info
		std::string strWrite = m_strDBPath;
		strWrite += DEU_CONNECT_STR;
		strWrite += m_strShared;

		//2. write shared memory
		//DEUSem::WaitSem(m_startHnd);
		m_regShm->WriteRegInfo(nType,strWrite);

		//3. release and wait semaphore
		DEUSem::ReleaseSem(m_svrRegHnd);
		DEUSem::WaitSem(m_regHnd);

		//4. read share memory
		if(!m_regShm->ReadRegInfo(nType,m_strDBPath,m_strShared))
		{
			DEUSem::CloseSem(m_eventClientHnd,m_strClientSem);
			DEUSem::CloseSem(m_multiHnd,m_strMultiSem);
			m_shm->DestroyShm();
			//DEUSem::ReleaseSem(m_startHnd);
			return false;
		}
		//5. if reg success,open server semaphore
		if(nType == 1)
		{
			//open semaphore
			m_strServerSem = m_strShared + "EventServer";
			m_eventSvrHnd = DEUSem::OpenSem(m_strServerSem);
			if(m_eventSvrHnd == NULL)
			{
				DEUSem::CloseSem(m_eventClientHnd,m_strClientSem);
				DEUSem::CloseSem(m_multiHnd,m_strMultiSem);
				m_shm->DestroyShm();
				//DEUSem::ReleaseSem(m_startHnd);
				return false;		
			}
		}
		//DEUSem::ReleaseSem(m_startHnd);
		return true;
	}
    // get block count
    unsigned DEUDBClient::getBlockCount(void) const
    {
        unsigned nCount = 0;
        //1. make shared object 
        ShareObj obj;
        obj.m_nType = DEU_GET_COUNT;
        obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
        obj.m_shmSize = m_nShmSize;
        obj.m_nHighBit = obj.m_nMidBit = obj.m_nLowBit = 0;

        //2. wait multi-thread semaphore and write shared info
        DEUSem::WaitSem(m_multiHnd);
        if(!m_shm->WriteInfo(m_strShmName,obj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return nCount;
        }

        //3. release server semaphore and wait client semaphore
        DEUSem::ReleaseSem(m_eventSvrHnd);
        DEUSem::WaitSem(m_eventClientHnd);

        //4. read info
        ShareObj outObj;
        if(!m_shm->ReadInfo(m_strShmName,outObj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return nCount;
        }
        if(outObj.m_nType == -1)
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return nCount;
        }

        //5. malloc memory to save data returned from server
        nCount = outObj.m_nTotalLength;

        //7. release multi-thread semaphore
        DEUSem::ReleaseSem(m_multiHnd);
        return nCount;
    }
    // get indices
    void  DEUDBClient::getIndices(std::vector<ID> &vecIndices, unsigned nOffset, unsigned nCount) const
    {
        vecIndices.clear();
        //1. make shared object 
        ShareObj obj;
        obj.m_nType = DEU_INDEX_INRANGE;
        obj.m_nTotalLength = 0;
        obj.m_startLength = nOffset;
        obj.m_endLength = nCount;
        obj.m_shmSize = m_nShmSize;
        obj.m_nHighBit = obj.m_nMidBit = obj.m_nLowBit = 0;

        //2. wait multi-thread semaphore and write shared info
        DEUSem::WaitSem(m_multiHnd);
        if(!m_shm->WriteInfo(m_strShmName,obj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return;
        }
        //3. release server semaphore and wait client semaphore
        DEUSem::ReleaseSem(m_eventSvrHnd);
        DEUSem::WaitSem(m_eventClientHnd);

        //4. read info
        ShareObj outObj;
        if(!m_shm->ReadInfo(m_strShmName,outObj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return;
        }
        if(outObj.m_nType == -1)
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return;
        }

        //5. malloc memory to save data returned from server
        unsigned nLength = outObj.m_nTotalLength;
        if(nLength == 0)
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return;
        }

        ID* pBuffer = (ID*)outObj.m_pData;
        for(unsigned i = 0; i < outObj.m_endLength - outObj.m_startLength + 1u;i++)
        {
            vecIndices.push_back(pBuffer[i]);
        }
        freeMemory(outObj.m_pData);

        unsigned nTemp = outObj.m_endLength - outObj.m_startLength + 1u;

        //6. read left data from server
        while(nTemp < nLength)
        {
            DEUSem::ReleaseSem(m_eventSvrHnd);
            DEUSem::WaitSem(m_eventClientHnd);
            if(!m_shm->ReadInfo(m_strShmName,outObj))
            {
                freeMemory(pBuffer);
                DEUSem::ReleaseSem(m_multiHnd);
                return;
            }
            pBuffer = (ID*)outObj.m_pData;
            const unsigned sz = outObj.m_endLength - outObj.m_startLength + 1;
            for(unsigned i = 0; i < sz; i++)
            {
                vecIndices.push_back(pBuffer[i]);
            }
            freeMemory(outObj.m_pData);
            nTemp += sz;
        }

        //7. release multi-thread semaphore
        DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
        return;
    }
    std::vector<unsigned> DEUDBClient::getVersion(const ID &id) const
    {
        std::vector<unsigned> vList;
        //1. make shared object 
        ShareObj obj;
        obj.m_nType = DEU_GET_VERSION;
        obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
        obj.m_shmSize = m_nShmSize;
        obj.m_nHighBit = id.m_nHighBit;
        obj.m_nMidBit = id.m_nMidBit;
        obj.m_nLowBit = id.m_nLowBit;

        //2. wait multi-thread semaphore and write shared info
        DEUSem::WaitSem(m_multiHnd);
        if(!m_shm->WriteInfo(m_strShmName,obj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return vList;
        }

        //3. release server semaphore and wait client semaphore
        DEUSem::ReleaseSem(m_eventSvrHnd);
        DEUSem::WaitSem(m_eventClientHnd);

        //4. read info
        ShareObj outObj;
        if(!m_shm->ReadInfo(m_strShmName,outObj))
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return vList;
        }
        if(outObj.m_nType == -1)
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return vList;
        }

        //5. malloc memory to save data returned from server
        unsigned nLength = outObj.m_nTotalLength;
        if(nLength == 0)
        {
            DEUSem::ReleaseSem(m_multiHnd);
            return vList;
        }

        unsigned* pBuffer = (unsigned*)outObj.m_pData;
        for(unsigned i = 0; i < outObj.m_endLength - outObj.m_startLength + 1u;i++)
        {
            vList.push_back(pBuffer[i]);
        }
        freeMemory(outObj.m_pData);

        unsigned nCount = outObj.m_endLength - outObj.m_startLength + 1u;

        //6. read left data from server
        while(nCount < nLength)
        {
            DEUSem::ReleaseSem(m_eventSvrHnd);
            DEUSem::WaitSem(m_eventClientHnd);
            if(!m_shm->ReadInfo(m_strShmName,outObj))
            {
                freeMemory(pBuffer);
                DEUSem::ReleaseSem(m_multiHnd);
                return vList;
            }
            pBuffer = (unsigned*)outObj.m_pData;
            const unsigned sz = outObj.m_endLength - outObj.m_startLength + 1;
            for(unsigned i = 0; i < sz; i++)
            {
                vList.push_back(pBuffer[i]);
            }
            freeMemory(outObj.m_pData);
            nCount += sz;
        }

        //7. release multi-thread semaphore
        DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
        return vList;
    }

	std::vector<ID> DEUDBClient::getAllIndices(void)
	{
		std::vector<ID> idVec;
		//1. make shared object 
		ShareObj obj;
		obj.m_nType = DEU_ALL_INDEX;
		obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
		obj.m_shmSize = m_nShmSize;
		obj.m_nHighBit = obj.m_nMidBit = obj.m_nLowBit = 0;

		//2. wait multi-thread semaphore and write shared info
		DEUSem::WaitSem(m_multiHnd);
		if(!m_shm->WriteInfo(m_strShmName,obj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return idVec;
		}

		//3. release server semaphore and wait client semaphore
		DEUSem::ReleaseSem(m_eventSvrHnd);
		DEUSem::WaitSem(m_eventClientHnd);

		//4. read info
		ShareObj outObj;
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return idVec;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return idVec;
		}

		//5. malloc memory to save data returned from server
		unsigned nLength = outObj.m_nTotalLength;
		if(nLength == 0)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return idVec;
		}

		ID* pBuffer = (ID*)outObj.m_pData;
		for(unsigned i = 0; i < outObj.m_endLength - outObj.m_startLength + 1u;i++)
        {
			idVec.push_back(pBuffer[i]);
        }
		freeMemory(outObj.m_pData);

		unsigned nCount = outObj.m_endLength - outObj.m_startLength + 1u;

		//6. read left data from server
		while(nCount < nLength)
		{
			DEUSem::ReleaseSem(m_eventSvrHnd);
			DEUSem::WaitSem(m_eventClientHnd);
			if(!m_shm->ReadInfo(m_strShmName,outObj))
			{
				freeMemory(pBuffer);
				DEUSem::ReleaseSem(m_multiHnd);
				return idVec;
			}
			pBuffer = (ID*)outObj.m_pData;
			const unsigned sz = outObj.m_endLength - outObj.m_startLength + 1;
			for(unsigned i = 0; i <= sz; i++)
            {
				idVec.push_back(pBuffer[i]);
            }
			freeMemory(outObj.m_pData);
			nCount += sz;
		}

		//7. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
		return idVec;

	}
	//is exist
	bool DEUDBClient::isExist(const ID &id)
	{
		//1. make shared object 
		std::string strID = id.toString();
		ShareObj obj;
		obj.m_nType = DEU_IS_EXIST;
		obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
		obj.m_shmSize = m_nShmSize;
		obj.m_nHighBit = id.m_nHighBit;
		obj.m_nMidBit = id.m_nMidBit;
		obj.m_nLowBit = id.m_nLowBit;

		//2. wait multi-thread semaphore and write shared info
		DEUSem::WaitSem(m_multiHnd);
		if(!m_shm->WriteInfo(m_strShmName,obj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//3. release server semaphore and wait client semaphore
		DEUSem::ReleaseSem(m_eventSvrHnd);
		DEUSem::WaitSem(m_eventClientHnd);

		//4. read shared info
		ShareObj outObj;
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//5. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);
		return true;
	}

	//read data
	bool DEUDBClient::readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion)
	{
		//1. make shared object
		ShareObj obj;
		obj.m_nType = DEU_READ_DATA;
		obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
		obj.m_shmSize = m_nShmSize;
        obj.m_nVersion = nVersion;
		obj.m_nHighBit = id.m_nHighBit;
		obj.m_nMidBit = id.m_nMidBit;
		obj.m_nLowBit = id.m_nLowBit;

		//2. wait multi-thread semaphore and write shared info
		DEUSem::WaitSem(m_multiHnd);
		if(!m_shm->WriteInfo(m_strShmName,obj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//3. release server semaphore and wait client semaphore
		DEUSem::ReleaseSem(m_eventSvrHnd);
		DEUSem::WaitSem(m_eventClientHnd);

		//4. read info
		ShareObj outObj;
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//5. malloc memory to save data returned from server
		nLength = outObj.m_nTotalLength;
		if(nLength == 0)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return true;
		}
		pBuffer = malloc(nLength);
        if(!pBuffer)
        {
            nLength = 0u;
            freeMemory(outObj.m_pData);
            DEUSem::ReleaseSem(m_multiHnd);
            return false;
        }
		memcpy((char*)pBuffer+outObj.m_startLength,outObj.m_pData,outObj.m_endLength - outObj.m_startLength + 1);
		freeMemory(outObj.m_pData);
		int nCount = nLength / m_nShmSize;

		//6. read left data from server
		unsigned sz = 0;
		for(int i = 1;i <= nCount;i++)
		{
			DEUSem::ReleaseSem(m_eventSvrHnd);
			DEUSem::WaitSem(m_eventClientHnd);
			if(!m_shm->ReadInfo(m_strShmName,outObj))
			{
				freeMemory(pBuffer);
				DEUSem::ReleaseSem(m_multiHnd);
				return false;
			}
			sz = outObj.m_endLength - outObj.m_startLength + 1;
			memcpy((char*)pBuffer+outObj.m_startLength,outObj.m_pData,sz);
			freeMemory(outObj.m_pData);
		}

		//7. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);
		return true;
	}


	//remove data
	bool DEUDBClient::removeBlock(const ID &id)
	{
		//1. make shared object 
		std::string strID = id.toString();
		ShareObj obj;
		obj.m_nType = DEU_REMOVE_DATA;
		obj.m_nTotalLength = obj.m_startLength = obj.m_endLength = 0;
		obj.m_shmSize = m_nShmSize;
		obj.m_nHighBit = id.m_nHighBit;
		obj.m_nMidBit = id.m_nMidBit;
		obj.m_nLowBit = id.m_nLowBit;

		//2. wait multi-thread semaphore and write shared info
		DEUSem::WaitSem(m_multiHnd);
		if(!m_shm->WriteInfo(m_strShmName,obj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//3. release server semaphore and wait client semaphore
		DEUSem::ReleaseSem(m_eventSvrHnd);
		DEUSem::WaitSem(m_eventClientHnd);

		//4. read shared info
		ShareObj outObj;
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//5. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);
		return true;
	}

	//write data
	bool DEUDBClient::addBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
	{
		unsigned nStart = 0,nEnd = 0;
		int nCount = nBufLen / m_nShmSize;

		//1. wait multi-thread semaphore
		ShareObj outObj;
		std::string strID = id.toString();
		DEUSem::WaitSem(m_multiHnd);

		//2. loop to write data
		for(int i = 0;i <= nCount;i++)
		{
			//2.1 make shared object
			outObj.m_nType = DEU_WRITE_DATA;
			outObj.m_nTotalLength = nBufLen;
			nStart = i * m_nShmSize;
			nEnd = nBufLen < (i+1)*m_nShmSize ? nBufLen-1 : (i+1)*m_nShmSize-1;
			outObj.m_startLength = nStart;
			outObj.m_endLength = nEnd;
			outObj.m_shmSize = m_nShmSize;
			outObj.m_nHighBit = id.m_nHighBit;
			outObj.m_nMidBit = id.m_nMidBit;
			outObj.m_nLowBit = id.m_nLowBit;
			void*  pTemp = malloc(nEnd - nStart + 1);
            if(!pTemp)
            {
                DEUSem::ReleaseSem(m_multiHnd);
                return false;
            }
			memcpy(pTemp,(char*)pBuffer+nStart,nEnd-nStart+1);
			outObj.m_pData = pTemp;

			//2.2 write shared object
			m_shm->WriteInfo(m_strShmName,outObj);
			freeMemory(pTemp);//free memory

			//2.3 release server semaphore and wait server semaphore
			DEUSem::ReleaseSem(m_eventSvrHnd);
			DEUSem::WaitSem(m_eventClientHnd);
		}

		//3. read shared info
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);
			return false;
		}

		//4. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);
		return true;
	}
	//update data
	bool DEUDBClient::updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
	{
		unsigned nStart = 0,nEnd = 0;
		int nCount = nBufLen / m_nShmSize;

		//1. wait multi-thread semaphore
		ShareObj outObj;
		std::string strID = id.toString();
		DEUSem::WaitSem(m_multiHnd);//WaitForSingleObject(m_multiHnd,INFINITE);

		//2. loop to write data
		for(int i = 0;i <= nCount;i++)
		{
			//2.1 make shared object
			outObj.m_nType = DEU_UPDATE_DATA;
			outObj.m_nTotalLength = nBufLen;
			nStart = i * m_nShmSize;
			nEnd = nBufLen < (i+1)*m_nShmSize ? nBufLen -1 : (i+1)*m_nShmSize - 1;
			outObj.m_startLength = nStart;
			outObj.m_endLength = nEnd;
			outObj.m_shmSize = m_nShmSize;
			outObj.m_nHighBit = id.m_nHighBit;
			outObj.m_nMidBit = id.m_nMidBit;
			outObj.m_nLowBit = id.m_nLowBit;
			void*  pTemp = malloc(nEnd - nStart + 1);
            if(!pTemp)
            {
                DEUSem::ReleaseSem(m_multiHnd);
                return false;
            }
			memcpy(pTemp,(char*)pBuffer+nStart,nEnd-nStart+1);
			outObj.m_pData = pTemp;

			//2.2 write shared object
			m_shm->WriteInfo(m_strShmName,outObj);
			freeMemory(pTemp);//free memory

			//2.3 release server semaphore and wait server semaphore
			DEUSem::ReleaseSem(m_eventSvrHnd);
			DEUSem::WaitSem(m_eventClientHnd);
		}

		//3. read shared info
		if(!m_shm->ReadInfo(m_strShmName,outObj))
		{
			DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
			return false;
		}
		if(outObj.m_nType == -1)
		{
			DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
			return false;
		}

		//4. release multi-thread semaphore
		DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
		return true;
	}
    //replace data
    bool DEUDBClient::replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
    {
        unsigned nStart = 0,nEnd = 0;
        int nCount = nBufLen / m_nShmSize;

        //1. wait multi-thread semaphore
        ShareObj outObj;
        std::string strID = id.toString();
        DEUSem::WaitSem(m_multiHnd);//WaitForSingleObject(m_multiHnd,INFINITE);

        //2. loop to write data
        for(int i = 0;i <= nCount;i++)
        {
            //2.1 make shared object
            outObj.m_nType = DEU_REPLACE_DATA;
            outObj.m_nTotalLength = nBufLen;
            nStart = i * m_nShmSize;
            nEnd = nBufLen < (i+1)*m_nShmSize ? nBufLen -1 : (i+1)*m_nShmSize - 1;
            outObj.m_startLength = nStart;
            outObj.m_endLength = nEnd;
            outObj.m_shmSize = m_nShmSize;
            outObj.m_nHighBit = id.m_nHighBit;
            outObj.m_nMidBit = id.m_nMidBit;
            outObj.m_nLowBit = id.m_nLowBit;
            void*  pTemp = malloc(nEnd - nStart + 1);
            if(!pTemp)
            {
                DEUSem::ReleaseSem(m_multiHnd);
                return false;
            }
            memcpy(pTemp,(char*)pBuffer+nStart,nEnd-nStart+1);
            outObj.m_pData = pTemp;

            //2.2 write shared object
            m_shm->WriteInfo(m_strShmName,outObj);
            freeMemory(pTemp);//free memory

            //2.3 release server semaphore and wait server semaphore
            DEUSem::ReleaseSem(m_eventSvrHnd);
            DEUSem::WaitSem(m_eventClientHnd);
        }

        //3. read shared info
        if(!m_shm->ReadInfo(m_strShmName,outObj))
        {
            DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
            return false;
        }
        if(outObj.m_nType == -1)
        {
            DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
            return false;
        }

        //4. release multi-thread semaphore
        DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
        return true;
    }
    // set clear code
    bool DEUDBClient::setClearFlag(bool bFlag)
    {
        //1. wait multi-thread semaphore
        ShareObj outObj;
        DEUSem::WaitSem(m_multiHnd);//WaitForSingleObject(m_multiHnd,INFINITE);

        //2 make shared object
        outObj.m_nType = DEU_SET_CLEAR_FLAG;
        outObj.m_nTotalLength = bFlag;

        //3. write shared object
        m_shm->WriteInfo(m_strShmName,outObj);

        //4. release server semaphore and wait server semaphore
        DEUSem::ReleaseSem(m_eventSvrHnd);
        DEUSem::WaitSem(m_eventClientHnd);

        //5. read shared info
        if(!m_shm->ReadInfo(m_strShmName,outObj))
        {
            DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
            return false;
        }
        if(outObj.m_nType == -1)
        {
            DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
            return false;
        }
        //6. release multi-thread semaphore
        DEUSem::ReleaseSem(m_multiHnd);//ReleaseSemaphore(m_multiHnd,1,NULL);
        return true;
    }
}
