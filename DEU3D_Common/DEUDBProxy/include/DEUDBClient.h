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
		std::string		m_strShared;        // �ͻ��������˹�������
		std::string		m_strShmName;       // �����ڴ�����
		std::string		m_strDBPath;        // DEUDB�ļ�·��
		unsigned		m_nShmSize;         // �����ڴ��С
		HANDLE			m_multiHnd;         // ���߳��ź���
		HANDLE			m_eventClientHnd;   // �ͻ��˹����ڴ��ź���
		HANDLE			m_eventSvrHnd;      // ����˶�д�����ڴ��ź���
		HANDLE          m_regHnd;           // ע���ź���
		HANDLE          m_svrRegHnd;        // �ȴ��������Ӧ�ź���
        HANDLE          m_pulseSemHnd;      // �����ź���
		HANDLE		    m_startHnd;         // ���ƶ�������������ź���
		HANDLE          m_partHnd;          // ����ר���ź���
		DEUShareMem*	m_shm;              // �����ڴ����ָ��
		DEUShareMem*	m_regShm;           // ע�Ṳ���ڴ����ָ��
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

