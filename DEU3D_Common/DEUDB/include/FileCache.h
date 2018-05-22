#include <map>
#include <list>
#include <stdio.h>
#include <assert.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/Atomic>

#include "IDEUDB.h"
#include "DataBase.h"
#include "DataStruct.h"
#include "WorkingThreads.h"
#include "RoutineManager.h"

namespace deudb
{
    class FileCache : public IDEUDB
    {
    public:
        explicit FileCache(void);
        virtual ~FileCache(void);

    protected:
        virtual bool                  openDB(const std::string &strDB, UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize);
        virtual void                  closeDB(void);
        virtual bool                  isOpen(void) const;
        virtual bool                  isExist(const ID &id) const;
        virtual bool                  readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion = 0);
        virtual bool                  removeBlock(const ID &id);
        virtual bool                  addBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
        virtual bool                  updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
        virtual bool                  replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen);
        virtual unsigned              getBlockCount(void) const;
        virtual void                  getIndices(std::vector<ID> &vecIndices, unsigned nOffset = 0u, unsigned nCount = ~0u) const;
        virtual std::vector<unsigned> getVersion(const ID &id) const;

    protected:
        static      UINT_64 getRefTime(void);

        bool        createNewIndexFile(void) const;
        void        closeDataBase(void);

        bool        readIndexFile(void);
        bool        readIndexFile_v1(const unsigned char *pIndexBuffer, unsigned nBufLen);
        bool        readIndexFile_v2(const unsigned char *pIndexBuffer, unsigned nBufLen);
        bool        updateIndexFile(const std::vector<BlockInIdx_v1>& blockIdxVec);

        void        resetInternalArgs(void);

    public:
        bool        restrictMemory(void);

    protected:
        std::string     m_strDB;

        typedef struct tagDataBlock
        {
            DBBlockInfo m_infoDBBlock;
            UINT_64     m_nLastRefTime;
            unsigned    m_nVersion;
            unsigned    m_nPosInIndex;
            void       *m_pMemory;
        }DataBlock;

        class RestrictMemThread : public WorkingThread
        {
        public:
            explicit RestrictMemThread(FileCache *pFileCache)
            {
                m_pFileCache = pFileCache;
            }

        protected:
            virtual void run(void);

        protected:
            FileCache *m_pFileCache;
        };

        std::map<IDVersion, DataBlock>  m_mapDataBlocks;
        std::map<ID, VersionList>       m_mapVersion;
        OpenThreads::Mutex              m_mtxDataBlocks;
        OpenThreads::Block              m_blockBuffer;

        std::map<UINT_64, DataBlock *>  m_mapBlockBuffer;
        UINT_64                         m_nBufferSizeLimited;
        UINT_64                         m_nCurrentMemorySize;

        std::list<unsigned>             m_listGapsInIndex;

        OpenSP::sp<DataBase>            m_pDataBase;

        WorkingThread                  *m_pRestrictMemThread;

        volatile bool                   m_bIsOpen;

        OpenSP::sp<RoutineManager>      m_pRoutineManager;

        unsigned                        m_nCurPosInIndex;


    };

}
