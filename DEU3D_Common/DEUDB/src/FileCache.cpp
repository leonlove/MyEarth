// FileCache.cpp : 定义 DLL 应用程序的导出函数。
//

#include "FileCache.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <time.h>
#include <vector>
#include <OpenThreads/ScopedLock>
#include <OpenSP/sp.h>
#include "Common/Common.h"
#include "WorkingThreads.h"


namespace deudb
{

const std::string   g_strIndexFileExt = ".idx";
const std::string   g_strMirroFix = "_bak";
const unsigned char g_szIndexFileFlag[8] = {'D', 'E', 'U', 'D', 'B', '\0', '\0', '\0'};

struct IdxFileHeader
{
    unsigned char   m_szFlag[8];        // must be "DEUDB\0\0\0"
    unsigned char   m_szReserved[8];    // reserved
    unsigned int    m_nVersionNumber;   // begin from 1
    unsigned char   m_reserved[20];     // reserved
};


IDEUDB *createDEUDB(void)
{
    OpenSP::sp<FileCache> pFileCache = new FileCache;
    return pFileCache.release();
}


void freeMemory(void *pData)
{
    if(NULL == pData)
    {
        return;
    }

    free(pData);
}


void FileCache::RestrictMemThread::run(void)
{
    while(0u == (unsigned)m_MissionFinished)
    {
        m_block.block();
        const bool bTaskRunOut = m_pFileCache->restrictMemory();
        if(bTaskRunOut)
        {
            suspend(true);
        }
    }
}


FileCache::FileCache(void)
{
    resetInternalArgs();
}


FileCache::~FileCache(void)
{
    closeDataBase();
}


void FileCache::resetInternalArgs(void)
{
    m_nBufferSizeLimited = 256u * 1024u * 1024u;
    m_nCurrentMemorySize = 0u;

    m_pRestrictMemThread = NULL;
    m_blockBuffer.release();

    m_strDB.clear();

    m_bIsOpen = false;

    m_nCurPosInIndex = ~0u;
}


UINT_64 FileCache::getRefTime(void)
{
    UINT_64 nUpdateTime = 0u;
    cmm::genUniqueValue64(nUpdateTime);
    return nUpdateTime;
}


void FileCache::closeDataBase(void)
{
    if(!isOpen())   return;
    m_bIsOpen = false;

    m_pRestrictMemThread->finishMission();
    m_pRestrictMemThread->join();
    m_pRestrictMemThread = NULL;

    m_pRoutineManager = NULL;

    m_pDataBase->closeDB();
    m_pDataBase = NULL;

    for(std::map<UINT_64, DataBlock *>::iterator itor = m_mapBlockBuffer.begin(); itor != m_mapBlockBuffer.end(); ++itor)
    {
        DataBlock *pBlock = itor->second;
        free(pBlock->m_pMemory);
    }

    m_mapBlockBuffer.clear();
    m_mapDataBlocks.clear();
    m_strDB.clear();
    m_listGapsInIndex.clear();
    resetInternalArgs();
}


bool FileCache::openDB(const std::string &strDB, UINT_64 nReadBufferSize, UINT_64 nWriteBufferSize)
{
    closeDataBase();
    if(strDB.empty())
    {
        return false;
    }
    m_strDB = strDB;

    const std::string   strIndexFilePath = strDB + g_strIndexFileExt;
    const bool bDatabaseExist = cmm::isFileExist(strIndexFilePath);
    if(!bDatabaseExist)
    {
        if(!createNewIndexFile())
        {
            return false;
        }
    }

    if(!readIndexFile())
    {
        return false;
    }

    FILE_GAP_MAP mapWhiteGap;
    std::map<IDVersion, DataBlock>::const_iterator itorBlock = m_mapDataBlocks.begin();
    for( ; itorBlock != m_mapDataBlocks.end(); ++itorBlock)
    {
        const DataBlock &block = itorBlock->second;
        mapWhiteGap[block.m_infoDBBlock.m_nDBFile].push_back(block.m_infoDBBlock.m_gap);
    }

    m_blockBuffer.release();
    m_nBufferSizeLimited = nReadBufferSize;

    m_pDataBase = new DataBase;
    m_pDataBase->init(m_strDB, mapWhiteGap);

    m_pRoutineManager = new RoutineManager;
    m_pRoutineManager->init(strIndexFilePath, m_pDataBase.get(), nWriteBufferSize);

    m_pRestrictMemThread = new RestrictMemThread(this);
    m_pRestrictMemThread->startThread();

    m_bIsOpen = true;

    return true;
}


bool FileCache::createNewIndexFile(void) const
{
    if(m_strDB.empty()) return false;

    std::string strTargetFile = m_strDB;
    strTargetFile += g_strIndexFileExt;

    IdxFileHeader header;
    memset(&header, 0, sizeof(IdxFileHeader));
    header.m_szFlag[0] = 'D';
    header.m_szFlag[1] = 'E';
    header.m_szFlag[2] = 'U';
    header.m_szFlag[3] = 'D';
    header.m_szFlag[4] = 'B';

    header.m_nVersionNumber = 2u;

    FILE *pFile = fopen(strTargetFile.c_str(), "wb");
    if(!pFile)  return false;
    const unsigned nRet = fwrite(&header, sizeof(IdxFileHeader), 1, pFile);
    fclose(pFile);

    return (nRet == 1u);
}

bool FileCache::updateIndexFile(const std::vector<BlockInIdx_v1>& blockIdxVec)
{
    const std::string   strIndexFilePath = m_strDB + g_strIndexFileExt;

    FILE *pFile = fopen(strIndexFilePath.c_str(), "wb");
    if(NULL == pFile)
    {
        return false;
    }

    IdxFileHeader header;
    memset(&header, 0, sizeof(IdxFileHeader));
    header.m_szFlag[0] = 'D';
    header.m_szFlag[1] = 'E';
    header.m_szFlag[2] = 'U';
    header.m_szFlag[3] = 'D';
    header.m_szFlag[4] = 'B';

    header.m_nVersionNumber = 2u;

    const unsigned nRet = fwrite(&header, sizeof(IdxFileHeader), 1, pFile);
#ifdef WIN32    
    std::vector<BlockInIdx_v1>::const_iterator  cItor = blockIdxVec.cbegin();
    while(cItor != blockIdxVec.cend())
#else
    std::vector<BlockInIdx_v1>::const_iterator  cItor = blockIdxVec.begin();
    while(cItor != blockIdxVec.end())
#endif
    {
        fseek(pFile, cItor->m_nPosInIndex, SEEK_SET);
        fwrite(&cItor->m_blockInIdx, sizeof(BlockInIdx), 1, pFile);
        cItor++;
    }

    fclose(pFile);
    return true;
}

bool FileCache::readIndexFile(void)
{
    const std::string   strIndexFilePath = m_strDB + g_strIndexFileExt;

    FILE *pFile = fopen(strIndexFilePath.c_str(), "rb");
    if(NULL == pFile)      return false;

    const unsigned nFileLength = cmm::getFileLength(pFile);
    if(nFileLength < sizeof(IdxFileHeader))
    {
        fclose(pFile);
        return false;
    }

    std::vector<unsigned char>  vecFileBuffer(nFileLength);
    fread(vecFileBuffer.data(), vecFileBuffer.size(), 1, pFile);
    fclose(pFile);

    IdxFileHeader header;
    memcpy(&header, vecFileBuffer.data(), sizeof(IdxFileHeader));

    if(memcmp(header.m_szFlag, g_szIndexFileFlag, sizeof(g_szIndexFileFlag)) != 0)
    {
        // It is not a valid deudb index file
        return false;
    }

    bool bRead = false;
    if(header.m_nVersionNumber == 1u)
    {
        const unsigned char *pBuffer = vecFileBuffer.data() + sizeof(IdxFileHeader);
        bRead = readIndexFile_v1(pBuffer, vecFileBuffer.size() - sizeof(IdxFileHeader));
    }
    else if(header.m_nVersionNumber == 2u)
    {
        const unsigned char *pBuffer = vecFileBuffer.data() + sizeof(IdxFileHeader);
        bRead = readIndexFile_v2(pBuffer, vecFileBuffer.size() - sizeof(IdxFileHeader));
    }
    if(!bRead)  return false;

    return true;
}


bool FileCache::readIndexFile_v1(const unsigned char *pIndexBuffer, unsigned nBufLen)
{
    const unsigned nInfoOffset = sizeof(IdxFileHeader);
    const unsigned nBlockCount = nBufLen / sizeof(BlockInIdx);
    unsigned  nBufferPos = 0;
    unsigned  nRemove = 0;

    std::vector<BlockInIdx_v1> blockIdxVec;

    for(unsigned int n = 0u; n < nBlockCount; n++)
    {
        BlockInIdx    blockInIdx;
        memcpy(&blockInIdx, pIndexBuffer, sizeof(BlockInIdx));

        if(blockInIdx.m_bRemove)
        {
            m_listGapsInIndex.push_back(nBufferPos + nInfoOffset);
            nRemove++;
        }
        else
        { 
            unsigned nVersion = time(NULL);
            blockInIdx.m_nVersion = nVersion;
            //save version
            VersionList& vList = m_mapVersion[blockInIdx.m_id];
            if(vList.empty())
            {
                vList.push_back(blockInIdx.m_nVersion);
            }
            else
            {
                VersionList::iterator verItor = std::find(vList.begin(),vList.end(),blockInIdx.m_nVersion);
                if(verItor == vList.end())
                {
                    vList.push_back(blockInIdx.m_nVersion);
                    std::sort(vList.begin(),vList.end());
                }
            }

            //save datablock
            DataBlock blockItem;
            blockItem.m_infoDBBlock  = blockInIdx.m_infoDBBlock;
            blockItem.m_pMemory      = NULL;
            blockItem.m_nLastRefTime = 0u;
            blockItem.m_nPosInIndex  = nBufferPos + nInfoOffset;
            blockItem.m_nVersion     = blockInIdx.m_nVersion;

            IDVersion idVersion(blockInIdx.m_id,blockInIdx.m_nVersion);
            m_mapDataBlocks[idVersion] = blockItem;
        }

        BlockInIdx_v1 bV1;
        bV1.m_nPosInIndex = nBufferPos + nInfoOffset;
        bV1.m_blockInIdx = blockInIdx;
        blockIdxVec.push_back(bV1);

        pIndexBuffer += sizeof(BlockInIdx);
        nBufferPos   += sizeof(BlockInIdx);
    }

    if((m_mapDataBlocks.size() + nRemove) < nBlockCount)
    {
        std::cout << "Warning: some errors exist in the .idx file." << std::endl;
    }

    //update index file
    updateIndexFile(blockIdxVec);

    m_nCurPosInIndex = nBlockCount*sizeof(BlockInIdx) + nInfoOffset;

    return true;
}

bool FileCache::readIndexFile_v2(const unsigned char *pIndexBuffer, unsigned nBufLen)
{
    const unsigned nInfoOffset = sizeof(IdxFileHeader);
    const unsigned nBlockCount = nBufLen / sizeof(BlockInIdx);
    unsigned  nBufferPos = 0;
    unsigned  nRemove = 0;

    for(unsigned int n = 0u; n < nBlockCount; n++)
    {
        BlockInIdx    blockInIdx;
        memcpy(&blockInIdx, pIndexBuffer, sizeof(BlockInIdx));

        if(blockInIdx.m_bRemove)
        {
            m_listGapsInIndex.push_back(nBufferPos + nInfoOffset);
            nRemove++;
        }
        else
        { 
            //save version
            VersionList& vList = m_mapVersion[blockInIdx.m_id];
            if(vList.empty())
            {
                vList.push_back(blockInIdx.m_nVersion);
            }
            else
            {
                VersionList::iterator verItor = std::find(vList.begin(),vList.end(),blockInIdx.m_nVersion);
                if(verItor == vList.end())
                {
                    vList.push_back(blockInIdx.m_nVersion);
                    std::sort(vList.begin(),vList.end());
                }
            }

            //save datablock
            DataBlock blockItem;
            blockItem.m_infoDBBlock  = blockInIdx.m_infoDBBlock;
            blockItem.m_pMemory      = NULL;
            blockItem.m_nLastRefTime = 0u;
            blockItem.m_nPosInIndex  = nBufferPos + nInfoOffset;
            blockItem.m_nVersion     = blockInIdx.m_nVersion;

            IDVersion idVersion(blockInIdx.m_id,blockInIdx.m_nVersion);
            m_mapDataBlocks[idVersion] = blockItem;
        }

        pIndexBuffer += sizeof(BlockInIdx);
        nBufferPos   += sizeof(BlockInIdx);
    }

    if((m_mapDataBlocks.size() + nRemove) < nBlockCount)
    {
        std::cout << "Warning: some errors exist in the .idx file." << std::endl;
    }

    m_nCurPosInIndex = nBlockCount*sizeof(BlockInIdx) + nInfoOffset;

    return true;
}

void FileCache::closeDB(void)
{
    closeDataBase();
}


bool FileCache::isOpen(void) const
{
    return m_bIsOpen;
}


bool FileCache::isExist(const ID &id) const
{
    if(!isOpen())   return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(const_cast<OpenThreads::Mutex &>(m_mtxDataBlocks));
    std::map<ID, VersionList>::const_iterator itorFind = m_mapVersion.find(id);
    if(itorFind == m_mapVersion.end())
    {
        return false;
    }
    return true;
}


std::vector<unsigned> FileCache::getVersion(const ID &id) const
{
    VersionList vList;
    if(!isOpen())   return vList;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(const_cast<OpenThreads::Mutex &>(m_mtxDataBlocks));
    std::map<ID, VersionList>::const_iterator itorFind = m_mapVersion.find(id);
    if(itorFind == m_mapVersion.end())
    {
        return vList;
    }

    return itorFind->second;
}


bool FileCache::readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion)
{
    if(!isOpen())   return false;

    m_blockBuffer.block();

    {
        // 1. Check if such id cannot be found
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockSlice(m_mtxDataBlocks);
        std::map<ID, VersionList>::iterator itorVersion = m_mapVersion.find(id);
        if(itorVersion == m_mapVersion.end())
        {
            nLength = 0u;
            pBuffer = NULL;
            return false;
        }

        // 2. Get proper version
        unsigned curVersion;
        if(nVersion == 0)
        {
            curVersion = itorVersion->second[itorVersion->second.size()-1];
        }
        else
        {
            VersionList vList = itorVersion->second;
            if(nVersion < vList[0])
            {
                return false;
            }
            else if(nVersion >= vList[vList.size() - 1])
            {
                curVersion = vList[vList.size() - 1];
            }
            else
            {
                for(unsigned n = 0;n < vList.size() - 1;n++)
                {
                    if(vList[n] <= nVersion && nVersion < vList[n+1])
                    {
                        curVersion = vList[n];
                    }
                }
            }
            
        }
        IDVersion idVersion(id,curVersion);

        // 3. Such id is found
        std::map<IDVersion,DataBlock>::iterator itorBlock = m_mapDataBlocks.find(idVersion);
        if(itorBlock == m_mapDataBlocks.end())
        {
            nLength = 0u;
            pBuffer = NULL;
            return false;
        }
        DataBlock &block = itorBlock->second;
        if(block.m_infoDBBlock.m_gap.m_nLength == 0u)
        {
            // it is an zero-length block, so it does not need to read
            pBuffer = NULL;
            nLength = 0u;
            return true;
        }

        // 4. It is not in the cache, so we must read it from db file or routine manager
        if(!block.m_pMemory)
        {
            // 3.1. read from routine manager
            block.m_pMemory = m_pRoutineManager->readRoutineBlock(id);

            // 3.2. read from db file
            if(!block.m_pMemory)
            {
                block.m_pMemory = m_pDataBase->readBlock(block.m_infoDBBlock);
                if(!block.m_pMemory)
                {
                    // so bad, disk error !!
                    nLength = 0u;
                    pBuffer = NULL;
                    return false;
                }
            }
        }

        // 4. Update the reference-time
        const bool bShouldErase = (block.m_nLastRefTime != 0u);
        if(bShouldErase)
        {
            m_mapBlockBuffer.erase(block.m_nLastRefTime);
        }
        block.m_nLastRefTime = getRefTime();
        m_mapBlockBuffer[block.m_nLastRefTime] = &block;

        // 5. Alloc memory
        pBuffer = malloc(block.m_infoDBBlock.m_gap.m_nLength);
        memcpy(pBuffer, block.m_pMemory, block.m_infoDBBlock.m_gap.m_nLength);
        nLength = block.m_infoDBBlock.m_gap.m_nLength;

        if(!bShouldErase)
        {
            m_nCurrentMemorySize += nLength;
        }

        if(m_nCurrentMemorySize > m_nBufferSizeLimited)
        {
            m_blockBuffer.set(false);
            m_pRestrictMemThread->suspend(false);
        }
    }

    return true;
}


bool FileCache::removeBlock(const ID &id)
{
    if(!isOpen())   return false;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(m_mtxDataBlocks);
        std::map<ID,VersionList>::iterator itorVersion = m_mapVersion.find(id);
        if(itorVersion == m_mapVersion.end())
        {
            return true;
        }

        VersionList vList = itorVersion->second;
        
        for(unsigned n = 0;n < vList.size();n++)
        {
            DBBlockInfo infoDBBlockOld;
            void    *pMemDelete = NULL;
            Routine routine;

            IDVersion idVersion(id,vList[n]);

            std::map<IDVersion, DataBlock>::iterator itorFind = m_mapDataBlocks.find(idVersion);
            if(itorFind == m_mapDataBlocks.end())
            {
                continue;
            }

            const DataBlock &block = itorFind->second;
            infoDBBlockOld = block.m_infoDBBlock;
            pMemDelete     = block.m_pMemory;

            m_listGapsInIndex.push_back(block.m_nPosInIndex);
            routine.m_nPosInIndex = block.m_nPosInIndex;

            const size_t nOldSize  = m_mapBlockBuffer.size();
            m_mapBlockBuffer.erase(block.m_nLastRefTime);
            m_mapDataBlocks.erase(itorFind);

            if(m_mapBlockBuffer.size() < nOldSize)
            {
                m_nCurrentMemorySize -= infoDBBlockOld.m_gap.m_nLength;
                if(m_nCurrentMemorySize <= m_nBufferSizeLimited)
                {
                    m_blockBuffer.release();
                }
            }
            routine.m_eRoutineType = Routine::RT_REMOVE;
            routine.m_pDataBlock = NULL;
            m_pRoutineManager->addRoutine(id, routine);
            m_pDataBase->releaseBlock(infoDBBlockOld);
            if(pMemDelete)
            {
                free(pMemDelete);
            }
        }

        m_mapVersion.erase(itorVersion);
    }

    return true;
}


bool FileCache::addBlock(const ID &id, const void *pBuffer, unsigned nBufLen)
{
    if(!isOpen())   return false;

    unsigned curVersion = time(NULL);
    if(curVersion == -1)
    {
        return false;
    }

    Routine routine;
    routine.m_eRoutineType = Routine::RT_ADD;
    routine.m_nVersion = curVersion;

    m_blockBuffer.block();

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(m_mtxDataBlocks);

        std::map<ID, VersionList>::const_iterator itorFind = m_mapVersion.find(id);
        if(itorFind != m_mapVersion.end())
        {
            // It already exist in data base
            return false;
        }

        unsigned  nDBFileIndex = 0u;
        UINT_64   nPosition    = 0u;
        const bool bEmptyBlock = (NULL == pBuffer || 0u == nBufLen);
        if(!bEmptyBlock)
        {
            if(!m_pDataBase->allocBlock(nBufLen, nDBFileIndex, nPosition))
            {
                // Disk error, so bad !! unable to find a gap to fit the new block!!
                return false;
            }
        }

        VersionList &vList = m_mapVersion[id];
        vList.push_back(curVersion);

        IDVersion idVersion(id,curVersion);

        DataBlock &block                      = m_mapDataBlocks[idVersion];
        block.m_nLastRefTime                  = getRefTime();
        block.m_infoDBBlock.m_nDBFile         = nDBFileIndex;
        block.m_infoDBBlock.m_gap.m_nPosition = nPosition;
        block.m_nVersion                      = curVersion;

        if(bEmptyBlock)
        {
            block.m_infoDBBlock.m_gap.m_nLength = 0u;
            block.m_pMemory = NULL;
        }
        else
        {
            block.m_infoDBBlock.m_gap.m_nLength = nBufLen;
            block.m_pMemory = malloc(nBufLen);
            memcpy(block.m_pMemory, pBuffer, nBufLen);
        }
        if(m_listGapsInIndex.empty())
        {
            block.m_nPosInIndex = m_nCurPosInIndex;
            m_nCurPosInIndex += sizeof(BlockInIdx);
        }
        else
        {
            block.m_nPosInIndex = m_listGapsInIndex.front();
            m_listGapsInIndex.erase(m_listGapsInIndex.begin());
        }

        m_mapBlockBuffer[block.m_nLastRefTime] = &block;

        m_nCurrentMemorySize += block.m_infoDBBlock.m_gap.m_nLength;
        if(m_nCurrentMemorySize > m_nBufferSizeLimited)
        {
            m_blockBuffer.set(false);
            m_pRestrictMemThread->suspend(false);
        }

        routine.m_nPosInIndex = block.m_nPosInIndex;
        routine.m_infoDBBlock.m_nDBFile = nDBFileIndex;
        routine.m_infoDBBlock.m_gap     = block.m_infoDBBlock.m_gap;

        if(NULL == pBuffer || 0u == nBufLen)
        {
            routine.m_pDataBlock = NULL;
        }
        else
        {
            routine.m_pDataBlock = malloc(nBufLen);
            memcpy(routine.m_pDataBlock, pBuffer, nBufLen);
        }
    }

    m_pRoutineManager->addRoutine(id, routine);

    return 1;
}

bool FileCache::replaceBlock(const ID &id, const void *pNewBuffer, unsigned nNewBufLen)
{
    if(!isOpen())   return false;

    unsigned curVersion = time(NULL);
    if(curVersion == -1)
    {
        return false;
    }

    unsigned nDBFileForNew  = 0u;
    UINT_64 nPositionForNew = 0u;
    const bool bEmptyBlock  = (NULL == pNewBuffer || 0u == nNewBufLen);
    if(!bEmptyBlock)
    {
        if(!m_pDataBase->allocBlock(nNewBufLen, nDBFileForNew, nPositionForNew))
        {
            // Disk error, so bad !! unable to find a gap for the new data!!
            return false;
        }
    }

    void   *pMemoryNew = NULL;
    Routine routine;
    routine.m_infoDBBlock.m_gap.m_nPosition = nPositionForNew;
    routine.m_infoDBBlock.m_gap.m_nLength   = nNewBufLen;
    routine.m_infoDBBlock.m_nDBFile         = nDBFileForNew;
    routine.m_eRoutineType = Routine::RT_UPDATE;
    routine.m_nVersion     = curVersion;
    routine.m_nPosInIndex  = ~0u;
    if(bEmptyBlock)
    {
        routine.m_pDataBlock  = NULL;
    }
    else
    {
        pMemoryNew = malloc(nNewBufLen);
        routine.m_pDataBlock  = malloc(nNewBufLen);
        memcpy(routine.m_pDataBlock, pNewBuffer, nNewBufLen);
        memcpy(pMemoryNew, pNewBuffer, nNewBufLen);
    }

    m_blockBuffer.block();


    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(m_mtxDataBlocks);

        VersionList& vList = m_mapVersion[id];

        for(unsigned n = 0;n < vList.size();n++)
        {
            DBBlockInfo infoDBBlockOld;
            void    *pMemDelete = NULL;
            Routine removeRoutine;

            IDVersion idVersion(id,vList[n]);

            std::map<IDVersion, DataBlock>::iterator itorFind = m_mapDataBlocks.find(idVersion);
            if(itorFind == m_mapDataBlocks.end())
            {
                continue;
            }

            const DataBlock &block = itorFind->second;
            infoDBBlockOld = block.m_infoDBBlock;
            pMemDelete     = block.m_pMemory;

            m_listGapsInIndex.push_back(block.m_nPosInIndex);
            removeRoutine.m_nPosInIndex = block.m_nPosInIndex;

            const size_t nOldSize  = m_mapBlockBuffer.size();
            m_mapBlockBuffer.erase(block.m_nLastRefTime);
            m_mapDataBlocks.erase(itorFind);

            if(m_mapBlockBuffer.size() < nOldSize)
            {
                m_nCurrentMemorySize -= infoDBBlockOld.m_gap.m_nLength;
                if(m_nCurrentMemorySize <= m_nBufferSizeLimited)
                {
                    m_blockBuffer.release();
                }
            }
            removeRoutine.m_eRoutineType = Routine::RT_REMOVE;
            removeRoutine.m_pDataBlock = NULL;
            m_pRoutineManager->addRoutine(id, removeRoutine);
            m_pDataBase->releaseBlock(infoDBBlockOld);
            if(pMemDelete)
            {
                free(pMemDelete);
            }
        }
        vList.clear();
        vList.push_back(curVersion);
        IDVersion idVersion(id,curVersion);
        if(m_listGapsInIndex.empty())
        {
            routine.m_nPosInIndex = m_nCurPosInIndex;
            m_nCurPosInIndex += sizeof(BlockInIdx);
        }
        else
        {
            routine.m_nPosInIndex = m_listGapsInIndex.front();
            m_listGapsInIndex.erase(m_listGapsInIndex.begin());
        }
        routine.m_eRoutineType = Routine::RT_ADD;

        DataBlock* pBlock = &m_mapDataBlocks[idVersion];
        pBlock->m_nVersion     = routine.m_nVersion;
        pBlock->m_nPosInIndex  = routine.m_nPosInIndex;
        pBlock->m_pMemory      = pMemoryNew;
        pBlock->m_nLastRefTime = 0u;
        pBlock->m_infoDBBlock.m_nDBFile = nDBFileForNew;
        pBlock->m_infoDBBlock.m_gap.m_nPosition = nPositionForNew;
        pBlock->m_infoDBBlock.m_gap.m_nLength   = 0u;

        if(!bEmptyBlock)
        {
            pBlock->m_nLastRefTime = getRefTime();

            pBlock->m_infoDBBlock.m_gap.m_nLength    = nNewBufLen;
            m_mapBlockBuffer[pBlock->m_nLastRefTime] = pBlock;

            m_nCurrentMemorySize  += pBlock->m_infoDBBlock.m_gap.m_nLength;
            if(m_nCurrentMemorySize > m_nBufferSizeLimited)
            {
                m_blockBuffer.set(false);
                m_pRestrictMemThread->suspend(false);
            }
        }
        m_pRoutineManager->addRoutine(id, routine);
    }
    return true;
}

bool FileCache::updateBlock(const ID &id, const void *pNewBuffer, unsigned nNewBufLen)
{
    if(!isOpen())   return false;

    unsigned curVersion = time(NULL);
    if(curVersion == -1)
    {
        return false;
    }

    unsigned nDBFileForNew  = 0u;
    UINT_64 nPositionForNew = 0u;
    const bool bEmptyBlock  = (NULL == pNewBuffer || 0u == nNewBufLen);
    if(!bEmptyBlock)
    {
        if(!m_pDataBase->allocBlock(nNewBufLen, nDBFileForNew, nPositionForNew))
        {
            // Disk error, so bad !! unable to find a gap for the new data!!
            return false;
        }
    }

    void   *pMemoryNew = NULL;
    Routine routine;
    routine.m_infoDBBlock.m_gap.m_nPosition = nPositionForNew;
    routine.m_infoDBBlock.m_gap.m_nLength   = nNewBufLen;
    routine.m_infoDBBlock.m_nDBFile         = nDBFileForNew;
    routine.m_eRoutineType = Routine::RT_UPDATE;
    routine.m_nVersion     = curVersion;
    routine.m_nPosInIndex  = ~0u;
    if(bEmptyBlock)
    {
        routine.m_pDataBlock  = NULL;
    }
    else
    {
        pMemoryNew = malloc(nNewBufLen);
        routine.m_pDataBlock  = malloc(nNewBufLen);
        memcpy(routine.m_pDataBlock, pNewBuffer, nNewBufLen);
        memcpy(pMemoryNew, pNewBuffer, nNewBufLen);
    }

    DBBlockInfo infoOldDBBlock;
    bool        bOldBlockExist = false;
   
    
    m_blockBuffer.block();

    
    void *pMemDelete = NULL;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(m_mtxDataBlocks);

        IDVersion idVersion(id,curVersion);
        std::map<IDVersion, DataBlock>::iterator itorFind = m_mapDataBlocks.find(idVersion);
        DataBlock *pBlock = NULL;
        if(itorFind != m_mapDataBlocks.end())
        {
            pBlock = &itorFind->second;

            infoOldDBBlock = pBlock->m_infoDBBlock;

            std::map<UINT_64, DataBlock *>::iterator itorBuffer = m_mapBlockBuffer.find(pBlock->m_nLastRefTime);
            if(itorBuffer != m_mapBlockBuffer.end())
            {
                pMemDelete = pBlock->m_pMemory;
                m_mapBlockBuffer.erase(itorBuffer);
                m_nCurrentMemorySize -= pBlock->m_infoDBBlock.m_gap.m_nLength;
            }
            routine.m_nPosInIndex = pBlock->m_nPosInIndex;
            bOldBlockExist = true;
        }
        else
        {
            if(m_listGapsInIndex.empty())
            {
                routine.m_nPosInIndex = m_nCurPosInIndex;
                m_nCurPosInIndex += sizeof(BlockInIdx);
            }
            else
            {
                routine.m_nPosInIndex = m_listGapsInIndex.front();
                m_listGapsInIndex.erase(m_listGapsInIndex.begin());
            }
            routine.m_eRoutineType = Routine::RT_ADD;
        }

        VersionList& vList = m_mapVersion[id];
        if(vList.empty())
        {
            vList.push_back(curVersion);
        }
        else
        {
            VersionList::iterator verItor = std::find(vList.begin(),vList.end(),curVersion);
            if(verItor == vList.end())
            {
                vList.push_back(curVersion);
                std::sort(vList.begin(),vList.end());
            }
        }

        if(!pBlock)
        {
            pBlock = &m_mapDataBlocks[idVersion];
        }

        pBlock->m_nVersion     = routine.m_nVersion;
        pBlock->m_nPosInIndex  = routine.m_nPosInIndex;
        pBlock->m_pMemory      = pMemoryNew;
        pBlock->m_nLastRefTime = 0u;
        pBlock->m_infoDBBlock.m_nDBFile = nDBFileForNew;
        pBlock->m_infoDBBlock.m_gap.m_nPosition = nPositionForNew;
        pBlock->m_infoDBBlock.m_gap.m_nLength   = 0u;

        if(!bEmptyBlock)
        {
            pBlock->m_nLastRefTime = getRefTime();

            pBlock->m_infoDBBlock.m_gap.m_nLength    = nNewBufLen;
            m_mapBlockBuffer[pBlock->m_nLastRefTime] = pBlock;

            m_nCurrentMemorySize  += pBlock->m_infoDBBlock.m_gap.m_nLength;
            if(m_nCurrentMemorySize > m_nBufferSizeLimited)
            {
                m_blockBuffer.set(false);
                m_pRestrictMemThread->suspend(false);
            }
        }
    }

    if(bOldBlockExist)
    {
        m_pDataBase->releaseBlock(infoOldDBBlock);
    }
    if(pMemDelete)
    {
        free(pMemDelete);
    }

    m_pRoutineManager->addRoutine(id, routine);
    return true;
}


unsigned FileCache::getBlockCount(void) const
{
    if(!isOpen())   return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(const_cast<OpenThreads::Mutex &>(m_mtxDataBlocks));
    return m_mapVersion.size();
}


void FileCache::getIndices(std::vector<ID> &vecIndices, unsigned nOffset/* = 0u*/, unsigned nCount/* = ~0u*/) const
{
    if(!isOpen())   return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlock(const_cast<OpenThreads::Mutex &>(m_mtxDataBlocks));

    vecIndices.clear();
    if(nCount == ~0u)
    {
        if(nOffset >= m_mapVersion.size())
        {
            return;
        }
        vecIndices.reserve(m_mapVersion.size() - nOffset);
    }
    else if(nCount == 0u)
    {
        return;
    }
    else
    {
        vecIndices.reserve(nCount);
    }

    std::map<ID, VersionList>::const_iterator itorBlock = m_mapVersion.begin();
    for(unsigned nIndex = 0u; itorBlock != m_mapVersion.end(); ++itorBlock, ++nIndex)
    {
        if(nIndex < nOffset)            continue;
        if(vecIndices.size() >= nCount) break;

        vecIndices.push_back(itorBlock->first);
    }
}


bool FileCache::restrictMemory(void)
{
    void *pMemDelete = NULL;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDataBlocks);
        if(m_nCurrentMemorySize <= m_nBufferSizeLimited)
        {
            m_blockBuffer.release();
            return true;
        }
        assert(!m_mapBlockBuffer.empty());

        DataBlock *pBlock      = m_mapBlockBuffer.begin()->second;
        pMemDelete             = pBlock->m_pMemory;
        pBlock->m_nLastRefTime = 0u;
        pBlock->m_pMemory      = NULL;

        assert(m_nCurrentMemorySize >= pBlock->m_infoDBBlock.m_gap.m_nLength);

        m_mapBlockBuffer.erase(m_mapBlockBuffer.begin());

        m_nCurrentMemorySize -= pBlock->m_infoDBBlock.m_gap.m_nLength;
    }

    free(pMemDelete);
    return false;
}

}
