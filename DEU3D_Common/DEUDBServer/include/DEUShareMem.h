#ifndef _DEUSHAREMEM_H_
#define _DEUSHAREMEM_H_

#include <DEUDB/IDEUDB.h>
#include "DEUDefine.h"

struct ShareObj
{
public:
	int                  m_nType;
	unsigned             m_nTotalLength;
	unsigned             m_startLength;
	unsigned             m_endLength;
	unsigned             m_shmSize;
    unsigned             m_nVersion;
	UINT_64              m_nLowBit;
    UINT_64              m_nMidBit;
    UINT_64              m_nHighBit;      

	std::vector<unsigned char>	m_vecData;
	ShareObj()
	{
		m_nType = m_nTotalLength = m_startLength = m_endLength = m_shmSize = -1;
        m_nVersion = 0;
		m_nLowBit = m_nMidBit = m_nHighBit = 0;
	}

	~ShareObj()
	{
		m_nType = m_nTotalLength = m_startLength = m_endLength = m_shmSize = -1;
        m_nVersion = 0;
		m_nLowBit = m_nMidBit = m_nHighBit = 0;
	}
};

class DEUShareMem
{
public:
	DEUShareMem(void);
	~DEUShareMem(void);
public:
	//shm function
	bool	CreateShm(unsigned nSize);
	bool	DestroyShm();
	bool	AtShm(const std::string& strShmName);
	bool	DtShm(const std::string& strShmName);
	//////////////////////////////////////////////////////////////////////////
    //deudb function
    bool	openDB(const std::string &strDB,unsigned nReadBufferSize = 134217728u, unsigned nWriteBufferSize = 67108864u);
    void	closeDB(void);
    bool    isExist(const ID &id);
    bool    readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion);
    bool	removeBlock(const ID &id);
    bool    removeBlock(const std::vector<int>& nCodeVec);
    bool	addBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
    bool    updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
    bool    replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
    std::vector<ID> getAllIndices(void);
    unsigned getBlockCount(void) const; 
    void  getIndices(std::vector<ID> &vecIndices, unsigned nOffset = 0u, unsigned nCount = ~0u) const;
    std::vector<unsigned> getVersion(const ID &id) const;
    // free memory
	void    freeMemory(void *pData);
	//////////////////////////////////////////////////////////////////////////
	//read type
	bool	ReadType(const std::string& strShmName,int& nType);
	//read Info from shared memory
	bool	ReadInfo(const std::string& strShmName,ShareObj& outObj);
	//write info to shared memory
	bool	WriteInfo(const std::string& strShmName,const ShareObj& sobj);
	//read reg info
	bool    ReadRegInfo(const std::string& strShmName,int& nType,std::string& strDB,std::string& strShared);
	bool    WriteRegInfo(const std::string& strShmName,const int& nType,const std::string& strDB,const std::string& strShared);
	//read share obj
	bool	ReadShmObj(char* shmAddr,ShareObj& sobj);
	bool    GetObj(const char* pBuffer,unsigned nTotal,unsigned nStart,unsigned nEnd,ShareObj& outObj);

private:
	//HANDLE						    m_shmHandle;//共享内存地址
	deudb::IDEUDB*			        m_pDB;//DEUDB指针
	std::map<std::string,void*>		m_shmPtrMap;//共享内存地址
	std::map<std::string,HANDLE>	m_shmHndMap;//共享内存句柄
	std::map<std::string,int>	    m_shmIDMap;//共享内存句柄
};
#endif //_DEUSHAREMEM_H_
