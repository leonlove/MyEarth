#include <algorithm>
#include "DEUShareMem.h"
#include "DEUSem.h"
#include "Common/crc.h"
#include <sstream>

DEUShareMem					 g_shm;                   //DEUShareMem object
std::vector<std::string>	 g_shmVec;                //shared memory vector
std::map<std::string,HANDLE> g_eventClientMap;        //client semaphore map
std::map<std::string,HANDLE> g_eventSvrMap;           //server semaphore map
std::map<std::string,HANDLE> g_threadMap;             //thread handle map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string                  g_strShmName    = "";    //shared memory name
std::string					 g_strDBPath     = "";    //DEUDB path
std::string					 g_strRegShmName = "";    //reg and unreg shared memory name
std::string                  g_strRegSem     = "";    //reg sem name
std::string                  g_strSvrRegSem  = "";    //server reg sem name
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HANDLE						 g_regHnd        = NULL;  //reg and unreg handle
HANDLE	                     g_partiHnd      = NULL;  //particular semaphore to get whether server is started for client
HANDLE	                     g_svrRegHnd     = NULL;
bool                         g_bClearFlag    = false; //clear deudb flag

//thread function
#ifdef WIN32
unsigned __stdcall ThreadProc(LPVOID lpParam)
#else
void* ThreadProc(void* lpParam)
#endif
{
	if(lpParam == NULL)
		exit(0);
	//1. get variables
	std::string strShared = (char*)lpParam;
	std::string strShm = strShared + "Shm";
	std::string strClient = strShared + "EventClient";
	std::string strSvr = strShared + "EventServer";
	//2. get client semaphore
	std::map<std::string,HANDLE>::iterator itr = g_eventClientMap.find(strClient);
	if(itr == g_eventClientMap.end())
		exit(0);
	HANDLE clientHnd = itr->second;
	//3. get server semaphore
	itr = g_eventSvrMap.find(strSvr);
	if(itr == g_eventSvrMap.end())
		exit(0);
	HANDLE serverHnd = itr->second;
	//4. make error shared object
	ShareObj errObj;
	errObj.m_nType = -1;
	//5. at shm and wait server semaphore
	g_shm.AtShm(strShm);
	DEUSem::WaitSem(serverHnd);
	//6. loop to process data
	while(1)
	{
		std::vector<std::string>::iterator itr = find(g_shmVec.begin(),g_shmVec.end(),strShared);
		if(itr == g_shmVec.end())
		{
			DEUSem::CloseSem(clientHnd,strClient);
			g_eventClientMap.erase(strClient);
			DEUSem::CloseSem(serverHnd,strSvr);
			g_eventClientMap.erase(strSvr);
#ifdef WIN32
			std::map<std::string,HANDLE>::iterator itrThread = g_threadMap.find(strShared);
			if(itrThread != g_threadMap.end())
				CloseHandle(itrThread->second);
			return 1;
#else
			return (void*)1;
#endif
		}
		//share objs
		ShareObj readObj;
		//read shared memory
		if(!g_shm.ReadInfo(strShm,readObj))
		{
			g_shm.WriteInfo(strShm,errObj);
			DEUSem::ReleaseSem(clientHnd);
		}
		else
		{
			switch(readObj.m_nType)
			{
			case DEU_READ_DATA://read data
				{
					//read block
					void* pBuffer = NULL;
					unsigned nLength = -1;
					if(!g_shm.readBlock(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit),pBuffer,nLength,readObj.m_nVersion))
						g_shm.WriteInfo(strShm,errObj);
					else if(pBuffer && nLength > 0u)
					{
						//loop
						unsigned nStart = 0,nEnd = 0;
						int nCount = nLength / readObj.m_shmSize;
						//write shared info
						for(int i = 0;i <= nCount;i++)
						{
							//write shared memory
							nStart = i*readObj.m_shmSize;
							nEnd = nLength < (i+1)*readObj.m_shmSize ? nLength : (i+1)*readObj.m_shmSize;
							ShareObj outObj;
							g_shm.GetObj((char*)pBuffer,nLength,nStart,nEnd - 1,outObj);
							g_shm.WriteInfo(strShm,outObj);

							if(i < nCount)
							{
								DEUSem::ReleaseSem(clientHnd);
								DEUSem::WaitSem(serverHnd);
							}
						}
						g_shm.freeMemory(pBuffer);
					}
				}
				break;
			case DEU_UPDATE_DATA://update data
				{
					std::vector<unsigned char> vecBuf(readObj.m_nTotalLength);

                    if(readObj.m_nTotalLength > 0)
					{
						//copy memory
						size_t sz = readObj.m_endLength - readObj.m_startLength + 1;
						memcpy(vecBuf.data(),readObj.m_vecData.data(),sz);
						int nCount = readObj.m_nTotalLength / readObj.m_shmSize;
						//get other data
						for(int i = 1;i <= nCount;i++)
						{
							DEUSem::ReleaseSem(clientHnd);
							DEUSem::WaitSem(serverHnd);
							if(!g_shm.ReadInfo(strShm,readObj))
							{
								g_shm.WriteInfo(strShm,errObj);
								break;
							}
							sz = readObj.m_endLength - readObj.m_startLength + 1;
							memcpy(vecBuf.data()+readObj.m_startLength,readObj.m_vecData.data(),sz);
						}
					}
					//update block
					if(!g_shm.updateBlock(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit),vecBuf.data(),vecBuf.size()))
						g_shm.WriteInfo(strShm,errObj);
				}
				break;
            case DEU_REPLACE_DATA://replace data
                {
                    std::vector<unsigned char> vecBuf(readObj.m_nTotalLength);

                    if(readObj.m_nTotalLength > 0)
                    {
                        //copy memory
                        size_t sz = readObj.m_endLength - readObj.m_startLength + 1;
                        memcpy(vecBuf.data(),readObj.m_vecData.data(),sz);
                        int nCount = readObj.m_nTotalLength / readObj.m_shmSize;
                        //get other data
                        for(int i = 1;i <= nCount;i++)
                        {
                            DEUSem::ReleaseSem(clientHnd);
                            DEUSem::WaitSem(serverHnd);
                            if(!g_shm.ReadInfo(strShm,readObj))
                            {
                                g_shm.WriteInfo(strShm,errObj);
                                break;
                            }
                            sz = readObj.m_endLength - readObj.m_startLength + 1;
                            memcpy(vecBuf.data()+readObj.m_startLength,readObj.m_vecData.data(),sz);
                        }
                    }
                    //update block
                    if(!g_shm.replaceBlock(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit),vecBuf.data(),vecBuf.size()))
                        g_shm.WriteInfo(strShm,errObj);
                }
                break;
			case DEU_WRITE_DATA://write data
				{
					std::vector<unsigned char> vecBuf(readObj.m_nTotalLength);
					//copy memory
					size_t sz = readObj.m_endLength - readObj.m_startLength+1;
					memcpy(vecBuf.data(),readObj.m_vecData.data(),sz);
					int nCount = readObj.m_nTotalLength / readObj.m_shmSize;
					//get other data
					for(int i = 1;i <= nCount;i++)
					{
						DEUSem::ReleaseSem(clientHnd);
						DEUSem::WaitSem(serverHnd);
						if(!g_shm.ReadInfo(strShm,readObj))
						{
							g_shm.WriteInfo(strShm,errObj);
							break;
						}
						sz = readObj.m_endLength - readObj.m_startLength + 1;
						memcpy(vecBuf.data()+readObj.m_startLength,readObj.m_vecData.data(),sz);
						
					}
					//update block
					if(!g_shm.addBlock(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit),vecBuf.data(),vecBuf.size()))
						g_shm.WriteInfo(strShm,errObj);
				}
				break;
            case DEU_SET_CLEAR_FLAG:
                {
                    g_bClearFlag = (readObj.m_nTotalLength != 0);
                }
                break;
			case DEU_REMOVE_DATA://remove data
				{
					//remove block
					if(!g_shm.removeBlock(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit)))
						g_shm.WriteInfo(strShm,errObj);
				}
				break;
			case DEU_IS_EXIST:
				{
					if(!g_shm.isExist(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit)))
						g_shm.WriteInfo(strShm,errObj);
				}
				break;
			case DEU_ALL_INDEX:
				{
					if(readObj.m_shmSize < sizeof(ID))
						g_shm.WriteInfo(strShm,errObj);
					else
					{
						//get add indices
						std::vector<ID> idVec = g_shm.getAllIndices();
						if(idVec.empty())
							g_shm.WriteInfo(strShm,errObj);
						else
						{
							//loop
							const unsigned nLength = idVec.size();
							unsigned nStart = 0,nEnd = 0;
							const unsigned nNum = readObj.m_shmSize / sizeof(ID);
							const unsigned nCount = nLength / nNum;
							//write shared info
							for(unsigned i = 0;i <= nCount;i++)
							{
								//write shared memory
								nStart  = i*nNum;
								nEnd = nLength < (i+1)*nNum ? nLength : (i+1)*nNum;
								ShareObj outObj;
								outObj.m_nType = readObj.m_nType;
								outObj.m_nTotalLength = nLength;
								outObj.m_startLength = nStart;
								outObj.m_endLength = nEnd - 1;
								outObj.m_vecData.resize((nEnd - nStart)*sizeof(ID));
								memcpy(&outObj.m_vecData[0],&*(idVec.begin() + nStart),(nEnd - nStart)*sizeof(ID));
								g_shm.WriteInfo(strShm,outObj);

								if(i < nCount)
								{
									DEUSem::ReleaseSem(clientHnd);
									DEUSem::WaitSem(serverHnd);
								}
							}
						}
					}

				}
				break;
            case DEU_GET_COUNT:
                {
                    unsigned nCount = g_shm.getBlockCount();
                    ShareObj outObj;
                    outObj.m_nType = readObj.m_nType;
                    outObj.m_nTotalLength = nCount;
                    g_shm.WriteInfo(strShm,outObj);
                }
                break;
            case DEU_GET_VERSION:
                {
                    if(readObj.m_shmSize < sizeof(unsigned))
                    {
                        g_shm.WriteInfo(strShm,errObj);
                    }
                    else
                    {
                        std::vector<unsigned> vList = g_shm.getVersion(ID(readObj.m_nHighBit,readObj.m_nMidBit,readObj.m_nLowBit));
                        if(vList.empty())
                        {
                            g_shm.WriteInfo(strShm,errObj);
                        }
                        else
                        {
                            //loop
                            const unsigned nLength = vList.size();
                            unsigned nStart = 0,nEnd = 0;
                            const unsigned nNum = readObj.m_shmSize / sizeof(unsigned);
                            const unsigned nCount = nLength / nNum;
                            //write shared info
                            for(unsigned i = 0;i <= nCount;i++)
                            {
                                //write shared memory
                                nStart  = i*nNum;
                                nEnd = nLength < (i+1)*nNum ? nLength : (i+1)*nNum;
                                ShareObj outObj;
                                outObj.m_nType = readObj.m_nType;
                                outObj.m_nTotalLength = nLength;
                                outObj.m_startLength = nStart;
                                outObj.m_endLength = nEnd - 1;
                                outObj.m_vecData.resize((nEnd - nStart)*sizeof(unsigned));
                                memcpy(&outObj.m_vecData[0],&*(vList.begin() + nStart),(nEnd - nStart)*sizeof(unsigned));
                                g_shm.WriteInfo(strShm,outObj);

                                if(i < nCount)
                                {
                                    DEUSem::ReleaseSem(clientHnd);
                                    DEUSem::WaitSem(serverHnd);
                                }
                            }
                        }
                    }
                }  
                break;
            case DEU_INDEX_INRANGE:
                {
                    if(readObj.m_shmSize < sizeof(ID))
                    {
                        g_shm.WriteInfo(strShm,errObj);
                    }
                    else
                    {
                        std::vector<ID> idVec;
                        g_shm.getIndices(idVec,readObj.m_startLength,readObj.m_endLength);
                        if(idVec.empty())
                        {
                            g_shm.WriteInfo(strShm,errObj);
                        }
                        else
                        {
                            //loop
                            const unsigned nLength = idVec.size();
                            unsigned nStart = 0,nEnd = 0;
                            const unsigned nNum = readObj.m_shmSize / sizeof(ID);
                            const unsigned nCount = nLength / nNum;
                            //write shared info
                            for(unsigned i = 0;i <= nCount;i++)
                            {
                                //write shared memory
                                nStart  = i*nNum;
                                nEnd = nLength < (i+1)*nNum ? nLength : (i+1)*nNum;
                                ShareObj outObj;
                                outObj.m_nType = readObj.m_nType;
                                outObj.m_nTotalLength = nLength;
                                outObj.m_startLength = nStart;
                                outObj.m_endLength = nEnd - 1;
                                outObj.m_vecData.resize((nEnd - nStart)*sizeof(ID));
                                memcpy(&outObj.m_vecData[0],&*(idVec.begin() + nStart),(nEnd - nStart)*sizeof(ID));
                                g_shm.WriteInfo(strShm,outObj);

                                if(i < nCount)
                                {
                                    DEUSem::ReleaseSem(clientHnd);
                                    DEUSem::WaitSem(serverHnd);
                                }
                            }
                        }
                    }
                }
                break;
			default:
				g_shm.WriteInfo(strShm,errObj);
			}
		}
		//reset event
		DEUSem::ReleaseSem(clientHnd);
		DEUSem::WaitSem(serverHnd);
	}
#ifdef WIN32
	return 1;
#else
	return (void*)1;
#endif
}

//register shared memory
bool RegShm(const std::string& strShmName)
{
	//1. if strShmName already exists,return true
	std::vector<std::string>::iterator itr = find(g_shmVec.begin(),g_shmVec.end(),strShmName);
	if(itr != g_shmVec.end())
		return true;
	//2. create thread
	g_strShmName = strShmName;
#ifdef WIN32
	HANDLE hnd = (HANDLE)_beginthreadex(NULL,0,ThreadProc,(void*)g_strShmName.c_str(),0,NULL);
	if(hnd == NULL)
		return false;
	g_threadMap[g_strShmName] = hnd;
#else
	pthread_t tid;
    int nRes = pthread_create(&tid,NULL,ThreadProc,(void*)g_strShmName.c_str());
	if(nRes != 0)
		return false;
#endif
	//3. save strShmName and return
	g_shmVec.push_back(strShmName);
	return true;
}

bool UnRegShm(const std::string& strShmName)
{
	std::string strShm = strShmName + "Shm";
	std::string strSvr = strShmName + "EventServer";
	std::string strClient = strShmName + "EventClient";
	g_shm.DtShm(strShm);
	std::vector<std::string>::iterator itr = find(g_shmVec.begin(),g_shmVec.end(),strShmName);
	if(itr != g_shmVec.end())
	{
		g_shmVec.erase(itr);
	}
	//release and close sem
	std::map<std::string,HANDLE>::const_iterator itr1 = g_eventSvrMap.find(strSvr);
	if(itr1 != g_eventSvrMap.end())
	{
		DEUSem::ReleaseSem(itr1->second);
	}
	return true;
}

//register and unregister thread funciton
#ifdef WIN32
unsigned __stdcall RegShmProc(LPVOID lpParam)
#else
void* RegShmProc(void* lpParam)
#endif
{
	//release
	DEUSem::ReleaseSem(g_regHnd);
	//wait for reg and unreg semahpore
	while(1)
	{
		DEUSem::WaitSem(g_svrRegHnd);
		//read reg info
		int nType = -1;
		std::string strPath = "";
		std::string strShared = "";
		if(!g_shm.ReadRegInfo(g_strRegShmName,nType,strPath,strShared))
		{
			g_shm.WriteRegInfo(g_strRegShmName,DEU_FAIL,strPath,strShared);
			DEUSem::ReleaseSem(g_regHnd);
			break;
		}
		if(strPath != g_strDBPath)
		{
			g_shm.WriteRegInfo(g_strRegShmName,DEU_FAIL,strPath,strShared);
			DEUSem::ReleaseSem(g_regHnd);
			break;
		}
		switch(nType)
		{
		case DEU_REG_SHM:
			{
				std::vector<std::string>::iterator itr = find(g_shmVec.begin(),g_shmVec.end(),strShared);
				if(itr != g_shmVec.end())
				{
					g_shm.WriteRegInfo(g_strRegShmName,DEU_ALREADY_EXIST,strPath,strShared);
					DEUSem::ReleaseSem(g_regHnd);
					break;
				}
				//open event
				std::string strClient = strShared + "EventClient";
				std::string strServer = strShared + "EventServer";
				//open mutex
				HANDLE hnd = DEUSem::OpenSem(strClient);
				if(hnd == NULL)
				{
					g_shm.WriteRegInfo(g_strRegShmName,DEU_FAIL,strPath,strShared);
					DEUSem::ReleaseSem(g_regHnd);
					break;
				}
				//create semaphore
				HANDLE svrHnd = DEUSem::CreateSem(strServer,0);
				if(svrHnd == NULL)
				{
					DEUSem::CloseSem(hnd,strClient);
					g_shm.WriteRegInfo(g_strRegShmName,DEU_FAIL,strPath,strShared);
					DEUSem::ReleaseSem(g_regHnd);
					break;
				}
				g_eventClientMap[strClient] = hnd;
				g_eventSvrMap[strServer] = svrHnd;
				if(!RegShm(strShared))
				{
					DEUSem::CloseSem(hnd,strClient);
					DEUSem::CloseSem(svrHnd,strServer);
					g_eventClientMap.erase(strClient);
					g_eventSvrMap.erase(strServer);
					g_shm.WriteRegInfo(g_strRegShmName,DEU_FAIL,strPath,strShared);
				}
				else
					g_shm.WriteRegInfo(g_strRegShmName,DEU_REG_SUCCESS,strPath,strShared);
				DEUSem::ReleaseSem(g_regHnd);
			}
			break;
		case DEU_UNREG_SHM:
			{
				//unreg
				UnRegShm(strShared);
				if(g_shmVec.empty())
				{
                    //close deudb
					g_shm.closeDB();
					g_shm.WriteRegInfo(g_strRegShmName,DEU_UNREG_SUCCESS,strPath,strShared);
					DEUSem::ReleaseSem(g_regHnd);
					DEUSem::CloseSem(g_regHnd,g_strRegSem);
#ifdef WIN32
					return 1;
#else
					return (void*)1;
#endif
				}
				g_shm.WriteRegInfo(g_strRegShmName,DEU_UNREG_SUCCESS,strPath,strShared);
				DEUSem::ReleaseSem(g_regHnd);
			}
			break;
		default:
			{
				g_shm.WriteRegInfo(g_strRegShmName,DEU_ERROR_TYPE,strPath,strShared);
				DEUSem::ReleaseSem(g_regHnd);
			}
			break;
		}
	}
#ifdef WIN32
	return 1;
#else
	return (void*)1;
#endif
}

unsigned long g_nPulse = 0;
unsigned long g_nPulseCount = 0;
std::string   g_strPulseSem = "";
HANDLE        g_pulseSemHnd = NULL;

unsigned __stdcall PulseProc(LPVOID lpParam)
{
    unsigned long nTime = 0;
    while(1)
    {
        DWORD dwState = WaitForSingleObject(g_pulseSemHnd,g_nPulse*1000);
        if(dwState == WAIT_TIMEOUT)
        {
            nTime++;
            if(nTime == g_nPulseCount)
                return 1;
        }
        else
        {
            nTime = 0;
        }
    }
    
}
//mainº¯Êý
#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance,   HINSTANCE hPrevInstance,   LPSTR lpCmdLine,   int nShowCmd )
{
	//Sleep(20000);
	//::MessageBoxA(NULL,"","",1);
	//get args
	if(__argc < 6)
		return 0;
	g_strDBPath = __argv[1];//db path
	UINT64 nReadBuf = _atoi64(__argv[2]);
	UINT64 nWriteBuf = _atoi64(__argv[3]);
    g_nPulse = atol(__argv[4]);
    g_nPulseCount = atol(__argv[5]);
#else
int main(int argc,char** argv)
{
	sleep(30);
	//get args
	if(argc < 4)
		return 0;
	g_strDBPath = argv[1];
	UINT64 nReadBuf = atoll(__argv[2]);
	UINT64 nWriteBuf = atoll(__argv[3]);
#endif
	//open start semaphore
    unsigned nReg = cmm::createHashCRC32(g_strDBPath.c_str(),g_strDBPath.length());
    std::ostringstream oss;
    oss<<nReg;
    g_strRegSem = oss.str();

	/*g_strRegSem = g_strDBPath;

	std::replace(g_strRegSem.begin(),g_strRegSem.end(),'\\','?');
	std::replace(g_strRegSem.begin(),g_strRegSem.end(),'/','?');
    std::replace(g_strRegSem.begin(),g_strRegSem.end(),' ','?');*/

	g_regHnd = DEUSem::OpenSem(g_strRegSem);
	if(g_regHnd == NULL)
		return 0;
	//open shm
	g_strRegShmName = g_strRegSem + "RegShm";
	if(!g_shm.AtShm(g_strRegShmName))
	{
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		return 0;
	}
	//open db, if fail ,return
	if(!g_shm.openDB(g_strDBPath,nReadBuf,nWriteBuf))
	{
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		return 0;
	}
	//create semaphore
	std::string strPart = g_strRegSem + "ParticularSem";
	g_partiHnd = DEUSem::CreateSem(strPart,0);
	if(g_partiHnd == NULL)
	{
		g_shm.closeDB();
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		return 0;
	}
	//create server reg semaphore
	g_strSvrRegSem = g_strRegSem + "SvrRegSem";
	g_svrRegHnd = DEUSem::CreateSem(g_strSvrRegSem,0);
	if(g_partiHnd == NULL)
	{
		g_shm.closeDB();
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		DEUSem::CloseSem(g_partiHnd,strPart);
		return 0;
	}

    g_strPulseSem = g_strRegSem + "PulseSem";
    g_pulseSemHnd = DEUSem::CreateSem(g_strPulseSem,0);
    if(g_pulseSemHnd == NULL)
    {
        g_shm.closeDB();
        DEUSem::CloseSem(g_partiHnd,strPart);
        DEUSem::CloseSem(g_svrRegHnd,g_strSvrRegSem);
        DEUSem::ReleaseSem(g_regHnd);
        DEUSem::CloseSem(g_regHnd,g_strRegSem);
        return 0;
    }

#ifdef WIN32
	//create register thread
	HANDLE hnd = (HANDLE)_beginthreadex(NULL,0,RegShmProc,NULL,0,NULL);
	if(hnd == NULL)
	{
		g_shm.closeDB();
		DEUSem::CloseSem(g_partiHnd,strPart);
		DEUSem::CloseSem(g_svrRegHnd,g_strSvrRegSem);
        DEUSem::CloseSem(g_pulseSemHnd,g_strPulseSem);
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		return 0;
	}

	//wait thread
	//WaitForSingleObject(hnd,INFINITE);
#else
	pthread_t tid;
    int nRes = pthread_create(&tid,NULL,RegShmProc,NULL);
	if(nRes != 0)
	{
		g_shm.closeDB();
		DEUSem::CloseSem(g_partiHnd,strPart);
		DEUSem::CloseSem(g_svrRegHnd,g_strSvrRegSem);
		DEUSem::ReleaseSem(g_regHnd);
		DEUSem::CloseSem(g_regHnd,g_strRegSem);
		return 0;
	}
	//wait thread
	pthread_join(tid,NULL);
#endif
    HANDLE pulseHnd = (HANDLE)_beginthreadex(NULL,0,PulseProc,NULL,0,NULL);
    if(pulseHnd == NULL)
    {
        g_shm.closeDB();
        DEUSem::CloseSem(g_partiHnd,strPart);
        DEUSem::CloseSem(g_svrRegHnd,g_strSvrRegSem);
        DEUSem::CloseSem(g_pulseSemHnd,g_strPulseSem);
        DEUSem::ReleaseSem(g_regHnd);
        DEUSem::CloseSem(g_regHnd,g_strRegSem);
        return 0;
    }
    
    HANDLE ghEvents[2];
    ghEvents[0] = hnd;
    ghEvents[1] = pulseHnd;

    DWORD dwState = WaitForMultipleObjects(2,ghEvents,FALSE,INFINITE);

    if(dwState == WAIT_OBJECT_0 + 0)
    {
        DEUSem::CloseSem(g_pulseSemHnd,g_strPulseSem);
        DEUSem::CloseSem(g_partiHnd,strPart);
        DEUSem::CloseSem(g_svrRegHnd,g_strRegSem);
        CloseHandle(hnd);
        CloseHandle(pulseHnd);
        //delete files
        if(g_bClearFlag)
        {
            std::string strFileName = g_strDBPath + std::string( ".idx");
            std::string strFileName2 = g_strDBPath + std::string( "_0.db");
            remove(strFileName.c_str());
            remove(strFileName2.c_str());
        }
        return 1;
    }
    else if(dwState == WAIT_OBJECT_0 + 1)
    {
        g_shm.closeDB();
        DEUSem::CloseSem(g_pulseSemHnd,g_strPulseSem);
        DEUSem::CloseSem(g_partiHnd,strPart);
        DEUSem::CloseSem(g_regHnd,g_strRegSem);
        DEUSem::CloseSem(g_svrRegHnd,g_strRegSem);
        CloseHandle(hnd);
        CloseHandle(pulseHnd);
        //delete files
        if(g_bClearFlag)
        {
            std::string strFileName = g_strDBPath + std::string( ".idx");
            std::string strFileName2 = g_strDBPath + std::string( "_0.db");
            remove(strFileName.c_str());
            remove(strFileName2.c_str());
        }
        return 1;
    }
    return 1;
	
}

