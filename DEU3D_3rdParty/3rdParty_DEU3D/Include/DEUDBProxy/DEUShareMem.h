#ifndef DEUDB_SHARE_MEM_H_8C66DA9F_D2C3_4A27_9E7E_E9D3623EAE5C_INCLUDE
#define DEUDB_SHARE_MEM_H_8C66DA9F_D2C3_4A27_9E7E_E9D3623EAE5C_INCLUDE

#include "DEUDefine.h"
namespace deudbProxy
{
	struct ShareObj
	{
	public:
		int                  m_nType;
		unsigned             m_nTotalLength;
		unsigned             m_startLength;
		unsigned             m_endLength;
		unsigned			 m_shmSize;
        unsigned             m_nVersion;
		UINT_64              m_nLowBit;
		UINT_64              m_nMidBit;
		UINT_64              m_nHighBit;      

		void*                m_pData;
		ShareObj()
		{
			m_nType = m_nTotalLength = m_startLength = m_endLength = m_shmSize = -1;
            m_nVersion = 0;
			m_nLowBit = m_nMidBit = m_nHighBit = 0;
			m_pData = NULL;
		}

		~ShareObj()
		{
			m_nType = m_nTotalLength = m_startLength = m_endLength = m_shmSize = -1;
            m_nVersion = 0;
			m_nLowBit = m_nMidBit = m_nHighBit = 0;
			m_pData = NULL;
		}
	};

	class DEUShareMem
	{
	public:
		DEUShareMem(void);
		~DEUShareMem(void);
	public:
		//shm function
		bool	CreateShm(const std::string& strShmName,unsigned nSize);
		bool	DestroyShm();
		bool	AtShm(const std::string& strShmName);
		//read info from shared memory
		bool	ReadInfo(const std::string& strShmName,ShareObj& outObj);
		//write info to shared memory
		bool	WriteInfo(const std::string& strShmName,const ShareObj& sobj);
		//read reg info
		bool    ReadRegInfo(const int& nType,const std::string& strDB,const std::string& strShared);
		bool    WriteRegInfo(const int& nType,const std::string& strRegInfo);

	private:
		HANDLE	m_shmHandle;
		std::string m_strShmName;
		int     m_nShmID;
		void*   m_shmAddr;

	};
}
#endif //_DEUCLIENTSHAREMEM_H_
