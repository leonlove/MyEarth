#include "DEUShareMem.h"
#include <stdio.h>

namespace deudbProxy
{
	DEUShareMem::DEUShareMem(void)
	{
	}


	DEUShareMem::~DEUShareMem(void)
	{
	}
#ifdef WIN32
	//create shared memory
	bool DEUShareMem::CreateShm(const std::string& strShmName,unsigned nSize)
	{
		//create share memory
#ifdef x64
		m_shmHandle = CreateFileMapping((HANDLE)0xFFFFFFFFFFFFFFFF,NULL,PAGE_READWRITE,0,nSize+DEU_SHAREMEM_SIZE,strShmName.c_str());
#else
		m_shmHandle = CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,nSize+DEU_SHAREMEM_SIZE,strShmName.c_str());
#endif
		//if fail,return
		if(m_shmHandle == NULL)
		{
			DWORD dw = GetLastError();
			return false;
		}
		//at shm
		m_shmAddr = MapViewOfFile(m_shmHandle,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
		if(m_shmAddr == NULL)
		{
			CloseHandle(m_shmHandle);
			return false;
		}
		return true;
	}
	bool DEUShareMem::AtShm(const std::string& strShmName)
	{
		//open shm
		m_shmHandle = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,strShmName.c_str());
		if(m_shmHandle == NULL)
			return false;
		//at shm
		m_shmAddr = MapViewOfFile(m_shmHandle,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
		if(m_shmAddr == NULL)
			return false;
		return true;
	}
	//destroy shared memory
	bool DEUShareMem::DestroyShm()
	{
		//unmap
		UnmapViewOfFile(m_shmAddr);
		//close handle
		return (CloseHandle(m_shmHandle) != 0);
	}
#else
	//create shared memory
	bool DEUShareMem::CreateShm(const std::string& strShmName,unsigned nSize)
	{
		m_strShmName = "/dev/shm/" + strShmName;
		FILE* pf = fopen(m_strShmName.c_str(),"w"); 
		if(pf == NULL)
			return false;
		fclose(pf);
		key_t shmKey = ftok(m_strShmName.c_str(),10);
		if(shmKey == -1)
			return false;
		m_nShmID = shmget(shmKey,nSize,IPC_CREAT|0666);
		if(m_nShmID == -1)
			return false;
		//at shm
		m_shmAddr = shmat(m_nShmID,0,0);
		if(m_shmAddr == NULL)
		{
			shmctl(m_nShmID,IPC_RMID,NULL);
			return false;
		}
		return true;
	}
	bool DEUShareMem::AtShm(const std::string& strShmName)
	{
		m_strShmName = "/dev/shm/" + strShmName;
		FILE* pf = fopen(m_strShmName.c_str(),"w"); 
		if(pf == NULL)
			return false;
		fclose(pf);

		key_t shmKey = ftok(m_strShmName.c_str(),10);
		if(shmKey == -1)
			return false;
		m_nShmID = shmget(shmKey,0,0);
		if(m_nShmID == -1)
			return false;
		//at shm
		m_shmAddr = shmat(m_nShmID,0,0);
		if(m_shmAddr == NULL)
		{
			shmctl(m_nShmID,IPC_RMID,NULL);
			return false;
		}
		return true;
	}
	//destroy shared memory
	bool DEUShareMem::DestroyShm()
	{
		shmdt(m_shmAddr);
		shmctl(m_nShmID,IPC_RMID,NULL);
		remove(m_strShmName.c_str());
		return true;
	}
#endif
	//read info from shared memory
	bool DEUShareMem::ReadInfo(const std::string& strShmName,ShareObj& outObj)
	{
		//if invalid address,return 
		if(m_shmAddr == NULL)
			return false;
		//get int address
		int* nAddr = (int*)m_shmAddr;
		//get type
		outObj.m_nType = nAddr[0];
		switch(outObj.m_nType)
		{
		case DEU_WRITE_DATA:
		case DEU_UPDATE_DATA:
		case DEU_REMOVE_DATA:
        case DEU_REPLACE_DATA:
		case DEU_IS_EXIST:
        case DEU_SET_CLEAR_FLAG:
		case DEU_FAIL:
			return true;
        case DEU_GET_COUNT:
            {
                outObj.m_nTotalLength = nAddr[1];
                return true;
            }
		case DEU_READ_DATA:
			{
				outObj.m_nTotalLength = nAddr[1];
				outObj.m_startLength =  nAddr[2];
				outObj.m_endLength = nAddr[3];
				//get data address
				UINT_64* tempAddr = (UINT_64*)(nAddr+6);
				char* chAddr = (char*)(tempAddr+3);
				//read data
				size_t sz = outObj.m_endLength - outObj.m_startLength + 1;
                char* newAddr = (char *)malloc(sz+1);
                if(!newAddr)    return false;
				memset(newAddr,'\0',sz+1);
				memcpy(newAddr,chAddr,sz);
				outObj.m_pData = newAddr;
				return true;
			}
		case DEU_ALL_INDEX:
        case DEU_INDEX_INRANGE:
			{
				outObj.m_nTotalLength = nAddr[1];
				outObj.m_startLength =  nAddr[2];
				outObj.m_endLength = nAddr[3];
				//get data address
				UINT_64* tempAddr = (UINT_64*)(nAddr+6);
				char* chAddr = (char*)(tempAddr+3);
				//read data
				size_t sz = outObj.m_endLength - outObj.m_startLength + 1;
                char* newAddr = (char *)malloc(sz*sizeof(ID)+1);
                if(!newAddr)    return false;
				memset(newAddr,'\0',sz*sizeof(ID)+1);
				memcpy(newAddr,chAddr,sz*sizeof(ID));
				outObj.m_pData = newAddr;
				return true;
			}
        case DEU_GET_VERSION:
            {
                outObj.m_nTotalLength = nAddr[1];
                outObj.m_startLength =  nAddr[2];
                outObj.m_endLength = nAddr[3];
                //get data address
                UINT_64* tempAddr = (UINT_64*)(nAddr+6);
                char* chAddr = (char*)(tempAddr+3);
                //read data
                size_t sz = outObj.m_endLength - outObj.m_startLength + 1;
                char* newAddr = (char *)malloc(sz*sizeof(unsigned)+1);
                if(!newAddr)    return false;
                memset(newAddr,'\0',sz*sizeof(unsigned)+1);
                memcpy(newAddr,chAddr,sz*sizeof(unsigned));
                outObj.m_pData = newAddr;
                return true;
            }
		}
		return false;
    }
    //write info to shared memory
    bool DEUShareMem::WriteInfo(const std::string& strShmName,const ShareObj& sobj)
    {
        if(m_shmAddr == NULL)
            return false;
        //write type
        int* nAddr = (int*)m_shmAddr;
        nAddr[0] = sobj.m_nType;
        switch(sobj.m_nType)
        {
        case DEU_FAIL:
            return true;
        case DEU_SET_CLEAR_FLAG:
            {
                nAddr[1] = sobj.m_nTotalLength;
                return true;
            }
        case DEU_ALL_INDEX:
        case DEU_GET_COUNT:
            {
                nAddr[1] = nAddr[2] = nAddr[3] = 0;
                nAddr[4] = sobj.m_shmSize;
                return true;
            }
        case DEU_INDEX_INRANGE:
            {
                nAddr[1] = 0;
                nAddr[2] = sobj.m_startLength;
                nAddr[3] = sobj.m_endLength;
                nAddr[4] = sobj.m_shmSize;
                return true;
            }
        case DEU_IS_EXIST:
        case DEU_READ_DATA:
        case DEU_REMOVE_DATA:
        case DEU_GET_VERSION:
            {
                nAddr[1] = nAddr[2] = nAddr[3] = 0;
                nAddr[4] = sobj.m_shmSize;
                nAddr[5] = sobj.m_nVersion;
                //write id
                UINT_64* lAddr = (UINT_64*)(nAddr+6);
                lAddr[0] = sobj.m_nHighBit;
                lAddr[1] = sobj.m_nMidBit;
                lAddr[2] = sobj.m_nLowBit;
                return true;
            }
        case DEU_WRITE_DATA:
        case DEU_UPDATE_DATA:
        case DEU_REPLACE_DATA:
            {
                nAddr[1] = sobj.m_nTotalLength;
                nAddr[2] = sobj.m_startLength;
                nAddr[3] = sobj.m_endLength;
                nAddr[4] = sobj.m_shmSize;
                //write id
                UINT_64* lAddr = (UINT_64*)(nAddr+6);
                lAddr[0] = sobj.m_nHighBit;
                lAddr[1] = sobj.m_nMidBit;
                lAddr[2] = sobj.m_nLowBit;
                //copy data
                char* chAddr = (char*)(lAddr+3);
                memcpy(chAddr,sobj.m_pData,sobj.m_endLength - sobj.m_startLength + 1);
                return true;
            }
		}
		return false;
	}
	//read reg info
	bool DEUShareMem::ReadRegInfo(const int& nType,const std::string& strDB,const std::string& strShared)
	{
		//1. read type
		int * nAddr = (int *)(m_shmAddr);
		int nTypeIn = nAddr[0];
		//2. get length
		int nLength = nAddr[1];
		if(nLength <= 0)
			return false;
		//3. read db and shared name
        char* chTemp = (char *)malloc(nLength+1);
        if(!chTemp) return false;

		memset(chTemp,'\0',nLength+1);
		memcpy(chTemp,(char*)(nAddr+2),nLength);
		//split to get dbpath and shared name
		std::string strGet = chTemp;
		free(chTemp);
		size_t nFind = strGet.find(DEU_CONNECT_STR);
		if(nFind == -1)
			return false;
		//4. compare dbpath and shread name
		std::string strDBIn = strGet.substr(0,nFind);
		if(strDBIn != strDB)
			return false;
		std::string strSharedIn = strGet.substr(nFind+DEU_CONNECT_LEN,strGet.length()-nFind-DEU_CONNECT_LEN);
		if(strSharedIn != strShared)
			return false;
		return true;
	}
	//write reg info
	bool DEUShareMem::WriteRegInfo(const int& nType,const std::string& strRegInfo)
	{
		//1. write type
		int * nAddr = (int *)(m_shmAddr);
		nAddr[0] = nType;
		//2. write reg info
		nAddr[1] = (int)strRegInfo.length();
		char* chTemp = (char*)(nAddr+2);
		memcpy(chTemp,strRegInfo.c_str(),strRegInfo.length());
		return true;
	}
}