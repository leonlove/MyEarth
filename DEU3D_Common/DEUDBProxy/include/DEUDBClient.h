#ifndef DEUDB_CLIENT_H_4A280468_A1E3_403B_816F_22FD3B4C7370_INCLUDE
#define DEUDB_CLIENT_H_4A280468_A1E3_403B_816F_22FD3B4C7370_INCLUDE

#include "Export.h"
#include "DEUDefine.h"
#include "DEUSem.h"
#include "IDEUDBProxy.h"
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>
#include <OpenThreads/Block>

namespace deudbProxy
{
	class DEUShareMem;

    class DEUDBProxyPulseThread : public OpenThreads::Thread
    {
    public:
        explicit DEUDBProxyPulseThread(unsigned nPulseSec,HANDLE pulseSemHnd)
        {
            m_atom.exchange(0);
            m_nPulseSec = nPulseSec;
            m_pulseSemHnd = pulseSemHnd;
        }
        void stopThread()
        {
            m_atom.exchange(1);
            m_block.release();
        }
    protected:
        virtual void run(void);

    protected:
        OpenThreads::Block  m_block;
        OpenThreads::Atomic m_atom;
        unsigned m_nPulseSec;
        HANDLE   m_pulseSemHnd;
    };

	class DEUDBClient : public IDEUDBProxy
	{
	public:
		explicit DEUDBClient(void);
		virtual ~DEUDBClient(void);
	public:
		//open deudb
		virtual bool	openDB(const std::string &strDB, UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize);
		//close deudb
		virtual bool    closeDB(void);
        //is exist
		virtual bool    isExist(const ID &id);
		//read data
		virtual bool    readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVerion = 0);
		//remove data
		virtual bool    removeBlock(const ID &id);
		//write data
		virtual bool    addBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
		//update data
		virtual bool    updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
        //replace data
        virtual bool    replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
		//get all indices
		virtual std::vector<ID> getAllIndices(void);
        // get block count
        virtual unsigned getBlockCount(void) const;
        // get indices
        virtual void  getIndices(std::vector<ID> &vecIndices, unsigned nOffset = 0u, unsigned nCount = ~0u) const;
        // get version
        virtual std::vector<unsigned> getVersion(const ID &id) const;
        // set clear code
        virtual bool  setClearFlag(bool bFlag);

	private:
        unsigned __stdcall pulseProc(LPVOID lpParam);
		bool	           regShm(unsigned nSize);         //register shm
		bool	           writeRegInfo(const int& nType);
		std::string	       getExePath();
		bool		       openExistServer();
		bool		       openNewServer(UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize);

	private:
		std::string		m_strShared;        // 客户端与服务端共享名称
		std::string		m_strShmName;       // 共享内存名称
		std::string		m_strDBPath;        // DEUDB文件路径
		unsigned		m_nShmSize;         // 共享内存大小
		HANDLE			m_multiHnd;         // 多线程信号量
		HANDLE			m_eventClientHnd;   // 客户端共享内存信号量
		HANDLE			m_eventSvrHnd;      // 服务端读写共享内存信号量
		HANDLE          m_regHnd;           // 注册信号量
		HANDLE          m_svrRegHnd;        // 等待服务端响应信号量
        HANDLE          m_pulseSemHnd;      // 心跳信号量
		HANDLE		    m_startHnd;         // 控制多个服务启动的信号量
		HANDLE          m_partHnd;          // 服务专有信号量
		DEUShareMem*	m_shm;              // 共享内存对象指针
		DEUShareMem*	m_regShm;           // 注册共享内存对象指针
		bool			m_bReg;
		bool			m_bUnReg;
        unsigned long   m_nPulseSec;
        unsigned long   m_nPulseCount;
		///////////////////////////////////////////////////////////////
		//sem name
		std::string     m_strStartSem;
		std::string     m_strPartSem;
		std::string     m_strSvrRegSem;
		std::string     m_strRegSem;
        std::string     m_strPulseSem;
		std::string     m_strClientSem;
		std::string     m_strMultiSem;
		std::string     m_strServerSem;
		///////////////////////////////////////////////////////////////
        DEUDBProxyPulseThread*  m_pulseThread;
	};
}
#endif //_DEUCLIENT_H_

