#include "RoutineManager.h"
#include "DataBase.h"
#include <assert.h>
#include <map>
#include <algorithm>

namespace deudb
{
    void RoutineManager::RoutineThread::run(void)
    {
        while((unsigned)m_MissionFinished == 0u)
        {
            m_block.block();

            while(true)
            {
                const bool bTaskRunOut = m_pRoutineMgr->doAction();
                if(bTaskRunOut) break;
            }

            suspend(true);
        }
    }


    RoutineManager::RoutineManager(void)
    {
        m_nWriteBufferLimited = 4u * 1024u * 1024u;
        m_nWriteBufferSize    = 0u;
        m_pRoutineThread      = NULL;
        m_pIndexFile          = NULL;
    }


    RoutineManager::~RoutineManager(void)
    {
        m_pRoutineThread->finishMission();
        delete m_pRoutineThread;

        fclose(m_pIndexFile);
    }


    bool RoutineManager::init(const std::string &strIndexFile, DataBase *pDataBase, UINT_64 nWriteBufferLimited)
    {
        m_nWriteBufferLimited = nWriteBufferLimited;
        m_nWriteBufferSize    = 0u;

        m_pDataBase           = pDataBase;

        m_pIndexFile = fopen(strIndexFile.c_str(), "rb+");
        if(!m_pIndexFile)   return false;

        m_pRoutineThread = new RoutineThread(this);
        m_pRoutineThread->startThread();

        m_blockWriteBuffer.release();

        return true;
    }


    bool RoutineManager::doAction(void)
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxRoutines);
            if(m_queueRoutines.empty())
            {
                return true;
            }
        }

        struct TaskEraser
        {
            RoutineManager     *m_pRoutineManager;
            TaskEraser(RoutineManager *pRoutineManager) : m_pRoutineManager(pRoutineManager)
            {}
            ~TaskEraser(void)
            {
                void *pMemFree = NULL;
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_pRoutineManager->m_mtxRoutines);
                    std::list<RoutineTask>::iterator itorTask = m_pRoutineManager->m_queueRoutines.begin();
                    const Routine &routine = itorTask->second;
                    pMemFree = routine.m_pDataBlock;
                    m_pRoutineManager->m_nWriteBufferSize -= routine.m_infoDBBlock.m_gap.m_nLength;
                    if(m_pRoutineManager->m_nWriteBufferSize <= m_pRoutineManager->m_nWriteBufferLimited)
                    {
                        m_pRoutineManager->m_blockWriteBuffer.set(true);
                    }
                    m_pRoutineManager->m_queueRoutines.erase(itorTask);
                }
                if(pMemFree)    free(pMemFree);
            }
        }eraser(this);

        ID &id = m_queueRoutines.front().first;
        Routine &routine = m_queueRoutines.front().second;

        if(routine.m_eRoutineType == Routine::RT_ADD)
        {
            // 1. write data block to db file
            if(routine.m_pDataBlock && routine.m_infoDBBlock.m_gap.m_nLength > 0u)
            {
                const bool bWrite = m_pDataBase->writeBlock(routine.m_infoDBBlock, routine.m_pDataBlock);
                if(!bWrite) return false;
            }

            // 2. write index info to index file
            BlockInIdx  index;
            index.m_id          = id;
            index.m_infoDBBlock = routine.m_infoDBBlock;
            index.m_nVersion    = routine.m_nVersion;
            index.m_bRemove      = false;
            if(routine.m_nPosInIndex == ~0u)
            {
                fseek(m_pIndexFile, 0, SEEK_END);
            }
            else
            {
                fseek(m_pIndexFile, routine.m_nPosInIndex, SEEK_SET);
            }
            fwrite(&index, sizeof(BlockInIdx), 1, m_pIndexFile);
#ifndef WIN32
            fflush(m_pIndexFile);
#endif
        }
        else if(routine.m_eRoutineType == Routine::RT_UPDATE)
        {
            // 1. write data block to db file
            const bool bWrite = m_pDataBase->writeBlock(routine.m_infoDBBlock, routine.m_pDataBlock);
            if(!bWrite)
            {
                return false;
            }

            // 2. write index file to update block version
            BlockInIdx  index;
            index.m_id          = id;
            index.m_infoDBBlock = routine.m_infoDBBlock;
            index.m_nVersion    = routine.m_nVersion;
            index.m_bRemove      = false;

            if(routine.m_nPosInIndex == ~0u)
            {
                fseek(m_pIndexFile, 0, SEEK_END);
            }
            else
            {
                fseek(m_pIndexFile, routine.m_nPosInIndex, SEEK_SET);
            }
            fwrite(&index, sizeof(BlockInIdx), 1, m_pIndexFile);
#ifndef WIN32
            fflush(m_pIndexFile);
#endif
        }
        else if(routine.m_eRoutineType == Routine::RT_REMOVE)
        {
            BlockInIdx  index;
            index.m_id          = id;
            index.m_infoDBBlock = routine.m_infoDBBlock;
            index.m_nVersion    = routine.m_nVersion;
            index.m_bRemove      = true;
            if(routine.m_nPosInIndex == ~0u)
            {
                fseek(m_pIndexFile, 0, SEEK_END);
            }
            else
            {
                fseek(m_pIndexFile, routine.m_nPosInIndex, SEEK_SET);
            }
            fwrite(&index, sizeof(BlockInIdx), 1, m_pIndexFile);
#ifndef WIN32
            fflush(m_pIndexFile);
#endif
        }
        else
        {
            assert(false);
        }

        if(routine.m_pDataBlock)
        {
            free(routine.m_pDataBlock);
            routine.m_pDataBlock = NULL;
        }

        return false;
    }


    bool RoutineManager::addRoutine(const ID &id, const Routine &routine)
    {
        m_blockWriteBuffer.block();

        void *pOldDataBlock = NULL;
        unsigned nOldDataBlockSize = 0u;
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxRoutines);

        m_queueRoutines.push_back(RoutineTask(id, routine));

        m_nWriteBufferSize += routine.m_infoDBBlock.m_gap.m_nLength;
        m_pRoutineThread->suspend(false);

        if(m_nWriteBufferSize > m_nWriteBufferLimited)
        {
            m_blockWriteBuffer.set(false);
        }

        return true;
    }


    void *RoutineManager::readRoutineBlock(const ID &id) const
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(const_cast<OpenThreads::Mutex &>(m_mtxRoutines));

        std::list<RoutineTask>::const_reverse_iterator itorFind = m_queueRoutines.rbegin();
        for( ; itorFind != m_queueRoutines.rend(); ++itorFind)
        {
            const RoutineTask &task = *itorFind;
            if(task.first == id)
            {
                break;
            }
        }
        if(itorFind == m_queueRoutines.rend())
        {
            return NULL;
        }

        const Routine &routine = itorFind->second;
        void *pDataBlock = NULL;
        if(routine.m_infoDBBlock.m_gap.m_nLength > 0u)
        {
            pDataBlock = malloc(routine.m_infoDBBlock.m_gap.m_nLength);
            memcpy(pDataBlock, routine.m_pDataBlock, routine.m_infoDBBlock.m_gap.m_nLength);
        }

        return pDataBlock;
    }

};

