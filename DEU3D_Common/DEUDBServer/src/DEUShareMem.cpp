#include "DEUShareMem.h"
#include <algorithm>

DEUShareMem::DEUShareMem(void)
{
}


DEUShareMem::~DEUShareMem(void)
{
}
#if defined(WIN32) || defined(WIN64)
//at shared memory
bool DEUShareMem::AtShm(const std::string& strShmName)
{
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr != m_shmPtrMap.end())
		return true;
	//open shm
	HANDLE shmHnd = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,strShmName.c_str());
	if(shmHnd == NULL)
		return false;
	//at shm
	void* shmFile = MapViewOfFile(shmHnd,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
	if(shmFile == NULL)
		return false;
	//save 
	m_shmPtrMap[strShmName] = shmFile;
	m_shmHndMap[strShmName] = shmHnd;
	return true;
}
//dt shm
bool DEUShareMem::DtShm(const std::string& strShmName)
{
	//dt shm
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr != m_shmPtrMap.end())
	{
		UnmapViewOfFile(itr->second);
	}
	//close handle
	std::map<std::string,HANDLE>::const_iterator hndItr = m_shmHndMap.find(strShmName);
	if(hndItr != m_shmHndMap.end())
	{
		CloseHandle(hndItr->second);
	}
	//erase
	m_shmPtrMap.erase(strShmName);
	m_shmHndMap.erase(strShmName);
	return true;
}
#else
//at shared memory
bool DEUShareMem::AtShm(const std::string& strShmName)
{
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr != m_shmPtrMap.end())
		return true;

	std::string strTemp = "/dev/shm/" + strShmName;
	FILE* pf = fopen(strTemp.c_str(),"w"); 
	if(pf == NULL)
		return false;
	fclose(pf);
	//open shm
	key_t shmKey = ftok(strTemp.c_str(),10);
	if(shmKey == -1)
		return false;
	int nShmID = shmget(shmKey,0,0);
	if(nShmID == -1)
		return false;
	//at shm
	void* shmFile = shmat(nShmID,0,0);
	if(shmFile == NULL)
	{
		shmctl(nShmID,IPC_RMID,NULL);
		return false;
	}
	//save 
	m_shmIDMap[strShmName] = nShmID;
	m_shmPtrMap[strShmName] = shmFile;
	return true;
}
//dt shm
bool DEUShareMem::DtShm(const std::string& strShmName)
{
	//dt shm
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr != m_shmPtrMap.end())
		shmdt(itr->second);
	//close shm
	//close handle
	std::map<std::string,int>::const_iterator idItr = m_shmIDMap.find(strShmName);
	if(idItr != m_shmIDMap.end())
		shmctl(idItr->second,IPC_RMID,NULL);
	//erase
	m_shmPtrMap.erase(strShmName);
	m_shmIDMap.erase(strShmName);
}
#endif
bool DEUShareMem::ReadRegInfo(const std::string& strShmName,int& nType,std::string& strDB,std::string& strShared)
{
	//get shm ptr
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr == m_shmPtrMap.end())
		return false;
	int * nAddr = (int *)(itr->second);
	nType = nAddr[0];

	//get length
	const int nLength = nAddr[1];
	if(nLength <= 0)    return false;

    std::vector<char> vecBuf(nLength+1, 0);
	memcpy(vecBuf.data(), (char*)(nAddr+2), nLength);
	//split to get dbpath and shared

	const std::string strGet = vecBuf.data();
	int nFind = strGet.find(DEU_CONNECT_STR);
	if(nFind == -1)     return false;

	strDB = strGet.substr(0,nFind);
	strShared = strGet.substr(nFind+DEU_CONNECT_LEN,strGet.length()-nFind-DEU_CONNECT_LEN);

	return true;
}


bool DEUShareMem::WriteRegInfo(const std::string& strShmName,const int& nType,const std::string& strDB,const std::string& strShared)
{
	//get shm ptr
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr == m_shmPtrMap.end())
		return false;
	int * nAddr = (int *)(itr->second);
	nAddr[0] = nType;
	std::string strWrite = strDB;
	strWrite += DEU_CONNECT_STR;
	strWrite += strShared;
	nAddr[1] = strWrite.length();
	char* chTemp = (char*)(nAddr+2);
	memcpy(chTemp,strWrite.c_str(),strWrite.length());
	return true;
}
//read type
bool DEUShareMem::ReadType(const std::string& strShmName,int& nType)
{
	//get shm ptr
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr == m_shmPtrMap.end())
		return false;
	int* shmAddr = (int*)itr->second;
	//get type
	nType = shmAddr[0];
	return true;
}
//read info from shared memory
bool DEUShareMem::ReadInfo(const std::string& strShmName,ShareObj& outObj)
{
	//get shm ptr
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr == m_shmPtrMap.end())
		return false;
	int * shmAddr = (int *)(itr->second);
	//get type
	outObj.m_nType = shmAddr[0];
	if(outObj.m_nType == DEU_WRITE_DATA || outObj.m_nType == DEU_UPDATE_DATA 
       || outObj.m_nType == DEU_REPLACE_DATA || outObj.m_nType == DEU_INDEX_INRANGE
       || outObj.m_nType == DEU_SET_CLEAR_FLAG)
	{
		outObj.m_nTotalLength = shmAddr[1];
        outObj.m_startLength =  shmAddr[2];
        outObj.m_endLength = shmAddr[3];
    }
    outObj.m_shmSize = shmAddr[4];
    if(outObj.m_nType == DEU_READ_DATA)
    {
        outObj.m_nVersion = shmAddr[5];
    }
    //read id
    UINT_64* lAddr = (UINT_64*)(shmAddr+6);
    outObj.m_nHighBit = lAddr[0];
    outObj.m_nMidBit = lAddr[1];
    outObj.m_nLowBit = lAddr[2];
    //read data
    if(outObj.m_nType == DEU_WRITE_DATA || outObj.m_nType == DEU_UPDATE_DATA || outObj.m_nType == DEU_REPLACE_DATA)
    {
        //read data
        size_t sz = outObj.m_endLength - outObj.m_startLength + 1;
        char* tempAddr = (char*)(lAddr+3);
        outObj.m_vecData.resize(sz);
        memcpy(outObj.m_vecData.data(), tempAddr, sz);
    }
	return true;
}
//write info to shared memory
bool DEUShareMem::WriteInfo(const std::string& strShmName,const ShareObj& sobj)
{
	//get shm ptr
	std::map<std::string,void*>::const_iterator itr = m_shmPtrMap.find(strShmName);
	if(itr == m_shmPtrMap.end())
		return false;
	int* shmAddr = (int*)itr->second;
	shmAddr[0] = sobj.m_nType;
	switch(sobj.m_nType)
	{
	case DEU_WRITE_DATA:
	case DEU_UPDATE_DATA:
    case DEU_REPLACE_DATA:
	case DEU_REMOVE_DATA:
	case DEU_FAIL:
    case DEU_SET_CLEAR_FLAG:
		return true;
    case DEU_GET_COUNT:
        {
            shmAddr[1] = sobj.m_nTotalLength;
            return true;
        }
	case DEU_READ_DATA:
		{
			//write length
			shmAddr[1] = sobj.m_nTotalLength;
			shmAddr[2] = sobj.m_startLength;
			shmAddr[3] = sobj.m_endLength;
			UINT_64* lAddr = (UINT_64*)(shmAddr+6);
			char* tempAddr = (char*)(lAddr+3);
			size_t sz = sobj.m_endLength - sobj.m_startLength + 1;
			memcpy(tempAddr,(char*)sobj.m_vecData.data(),sz);
			return true;
		}
	case DEU_ALL_INDEX:
    case DEU_INDEX_INRANGE:
		{
			//write length
			shmAddr[1] = sobj.m_nTotalLength;
			shmAddr[2] = sobj.m_startLength;
			shmAddr[3] = sobj.m_endLength;
			UINT_64* lAddr = (UINT_64*)(shmAddr+6);
			ID* tempAddr = (ID*)(lAddr+3);
			size_t sz = sobj.m_endLength - sobj.m_startLength + 1;
			memcpy(tempAddr,sobj.m_vecData.data(),sz*sizeof(ID));
			return true;
		}
    case DEU_GET_VERSION:
        {
            //write length
            shmAddr[1] = sobj.m_nTotalLength;
            shmAddr[2] = sobj.m_startLength;
            shmAddr[3] = sobj.m_endLength;
            UINT_64* lAddr = (UINT_64*)(shmAddr+6);
            ID* tempAddr = (ID*)(lAddr+3);
            size_t sz = sobj.m_endLength - sobj.m_startLength + 1;
            memcpy(tempAddr,sobj.m_vecData.data(),sz*sizeof(unsigned));
            return true;
        }
	}
	return false;
}

//open db
bool  DEUShareMem::openDB(const std::string &strDB,unsigned nReadBufferSize, unsigned nWriteBufferSize)
{
	//open DEUDB
	m_pDB = deudb::createDEUDB();
	bool bRes = false;
	if(m_pDB->openDB(strDB,nReadBufferSize,nWriteBufferSize))
		return true;
	return false;
}
//remove block
bool DEUShareMem::removeBlock(const ID &id)
{
	if(m_pDB == NULL)
		return false;
	//remove block
	return m_pDB->removeBlock(id);
}
bool DEUShareMem::removeBlock(const std::vector<int>& nCodeVec)
{
    std::vector<ID> idVec;
    m_pDB->getIndices(idVec);
    if(idVec.empty() || nCodeVec.empty())
        return true;

    if(std::find(nCodeVec.begin(),nCodeVec.end(),-1) == nCodeVec.end())//delete specified dataset
    {
        for(unsigned n = 0;n < idVec.size();n++)
        {
            ID id = idVec[n];
            if(std::find(nCodeVec.begin(),nCodeVec.end(),id.ObjectID.m_nDataSetCode) != nCodeVec.end())
                m_pDB->removeBlock(idVec[n]);
        }
    }
    else//delete all dataset
    {
        for(unsigned n = 0;n < idVec.size();n++)
        {
            m_pDB->removeBlock(idVec[n]);
        }
    }
    return true;
}
//read block
bool DEUShareMem::readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion)
{
	if(m_pDB == NULL)
		return false;
	//read block
	return m_pDB->readBlock(id,pBuffer,nLength,nVersion);
}
//add  block
bool DEUShareMem::addBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
{
	if(m_pDB == NULL)
		return false;
	//add block
	return m_pDB->addBlock(id,pBuffer,nBufLen);
}
//update block
bool DEUShareMem::updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
{
	if(m_pDB == NULL)
		return false;
	//update block
	return m_pDB->updateBlock(id,pBuffer,nBufLen);
}
//replace block
bool DEUShareMem::replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
{
    if(m_pDB == NULL)
        return false;
    //update block
    return m_pDB->replaceBlock(id,pBuffer,nBufLen);
}
//free memory
void DEUShareMem::freeMemory(void *pData)
{
	deudb::freeMemory(pData);
}
//close  DEUDB
void DEUShareMem::closeDB(void)
{
	if(m_pDB != NULL)
		m_pDB->closeDB();
}
bool DEUShareMem::isExist(const ID &id)
{
	if(m_pDB == NULL)
		return false;
	//is exist
	return m_pDB->isExist(id);
}
IDList DEUShareMem::getAllIndices(void)
{
	IDList idVec;
	if(m_pDB == NULL)
		return idVec;
	m_pDB->getIndices(idVec);
    return idVec;
}

unsigned DEUShareMem::getBlockCount(void) const
{
    if(m_pDB == NULL)
        return 0;
    return m_pDB->getBlockCount();
}

void  DEUShareMem::getIndices(std::vector<ID> &vecIndices, unsigned nOffset, unsigned nCount) const
{
    if(m_pDB == NULL)
        return;
    m_pDB->getIndices(vecIndices,nOffset,nCount);
}

std::vector<unsigned> DEUShareMem::getVersion(const ID &id) const
{
    std::vector<unsigned> versionVec;
    if(m_pDB == NULL)
        return versionVec;
    return m_pDB->getVersion(id);
}
//get  object
bool DEUShareMem::GetObj(const char* pBuffer,unsigned nTotal,unsigned nStart,unsigned nEnd,ShareObj& outObj)
{
	outObj.m_nType = DEU_READ_DATA;
	outObj.m_nTotalLength = nTotal;
	outObj.m_startLength = nStart;
	outObj.m_endLength = nEnd;

	size_t sz = nEnd - nStart + 1;
	outObj.m_vecData.resize(sz);
	memcpy(&outObj.m_vecData[0],pBuffer+nStart,sz);
	return true;
}
