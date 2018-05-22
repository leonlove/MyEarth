#include "DataBase.h"
#include <algorithm>
#include <sstream>
#include <OpenThreads/ScopedLock>
#include "Common/Common.h"
#include "RoutineManager.h"

#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif

namespace deudb
{

const std::string DataBase::ms_strDBFileExt = ".db";

DataBase::DataBase(void)
{
}


DataBase::~DataBase(void)
{
}


void DataBase::init(const std::string &strDatabase, const FILE_GAP_MAP &mapWhiteGap)
{
    m_strDataBase = strDatabase;
    std::replace(m_strDataBase.begin(), m_strDataBase.end(), '\\', '/');
    for(unsigned n = 0u; ; n++)
    {
        std::stringstream ss;
        ss << '_' << n << ms_strDBFileExt;
        const std::string strDBFilePath = m_strDataBase + ss.str();
        if(n > 0u && !cmm::isFileExist(strDBFilePath))
        {
            // I think I should improve the algorithm here
            break;
        }

        DatabaseFile *pFile = new DatabaseFile;
        FILE_GAP_MAP::const_iterator itorGapList = mapWhiteGap.find(n);
        if(itorGapList != mapWhiteGap.end())
        {
            const std::vector<FileGap> &vecFileGaps = itorGapList->second;
            pFile->init(strDBFilePath, vecFileGaps);
        }
        else
        {
            pFile->init(strDBFilePath);
        }

        m_mapDatabaseFiles[n] = pFile;
    }
}


void DataBase::closeDB(void)
{
    std::map<unsigned, OpenSP::sp<DatabaseFile> >::iterator itorFile = m_mapDatabaseFiles.begin();
    for( ; itorFile != m_mapDatabaseFiles.end(); ++itorFile)
    {
        DatabaseFile *pFile = itorFile->second;
        pFile->closeFile();
    }
}


void *DataBase::readBlock(const DBBlockInfo &infoDBBlock)
{
    DatabaseFile *pFile = NULL;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockDB(m_mtxDataBase);
        std::map<unsigned, OpenSP::sp<DatabaseFile> >::iterator itorFile = m_mapDatabaseFiles.find(infoDBBlock.m_nDBFile);
        if(itorFile == m_mapDatabaseFiles.end())
        {
            return NULL;
        }
        pFile = itorFile->second;
    }

    return pFile->readBlock(infoDBBlock.m_gap);
}


bool DataBase::writeBlock(const DBBlockInfo &infoDBBlock, const void *pDataBlock)
{
    DatabaseFile *pFile = NULL;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDataBase);
        std::map<unsigned, OpenSP::sp<DatabaseFile> >::iterator itorFile = m_mapDatabaseFiles.find(infoDBBlock.m_nDBFile);
        if(itorFile != m_mapDatabaseFiles.end())
        {
            pFile = itorFile->second;
        }
    }

    if(NULL == pFile)
    {
        return false;
    }

    return pFile->writeBlock(infoDBBlock.m_gap, pDataBlock);
}


bool DataBase::allocBlock(unsigned nLength, unsigned &nDBFile, UINT_64 &nPosition)
{
    if(nLength == 0u)
    {
        nDBFile   = 0u;
        nPosition = 0u;
        return true;
    }

    unsigned nExpectantFileIndex = ~0u;
#if defined (WIN32) || defined (WIN64)
    UINT_64 nExpectantPosition   = ~0ui64;
#else
    UINT_64 nExpectantPosition   = ~0uLL;
#endif

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDataBase);

        std::map<unsigned, OpenSP::sp<DatabaseFile> >::iterator itorFile = m_mapDatabaseFiles.begin();
        for( ; itorFile != m_mapDatabaseFiles.end(); ++itorFile)
        {
            DatabaseFile *pFile = itorFile->second;
            nExpectantPosition = pFile->allocBlock(nLength);

#if defined (WIN32) || defined (WIN64)
            if(nExpectantPosition == ULLONG_MAX - 1)//磁盘空间满
            {
                return false;
            }
            else if(nExpectantPosition != ULLONG_MAX)//分配空间成功
#else
            if(nExpectantPosition == ULLONG_MAX - 1)
            {
                return false;
            }
            else if(nExpectantPosition != ULLONG_MAX)
#endif
            {
                nExpectantFileIndex = itorFile->first;
                nDBFile   = nExpectantFileIndex;
                nPosition = nExpectantPosition;
                return true;
            }
        }

        // current database files cannot fit this memory block, so create an new database file now

        unsigned nNewFileIndex = 0u;
        for(itorFile = m_mapDatabaseFiles.begin(); itorFile != m_mapDatabaseFiles.end(); ++itorFile)
        {
            const unsigned &nFileIndex = itorFile->first;
            if(nNewFileIndex != nFileIndex)
            {
                break;
            }
            nNewFileIndex++;
        }

        std::stringstream ss;
        ss << '_' << nNewFileIndex << ms_strDBFileExt.c_str();

        DatabaseFile *pNewFile = new DatabaseFile;
        if(!pNewFile->init(m_strDataBase + ss.str()))
        {
            delete pNewFile;
            return false;
        }

        nExpectantFileIndex = nNewFileIndex;
        nExpectantPosition  = pNewFile->allocBlock(nLength);
#if defined (WIN32) || defined (WIN64)
        if(nExpectantPosition == ULLONG_MAX || nExpectantPosition == ULLONG_MAX - 1)
#else 
        if(nExpectantPosition == ULLONG_MAX || nExpectantPosition == ULLONG_MAX - 1)
#endif
        {
            pNewFile->closeFile();
            delete pNewFile;
            return false;
        }

        m_mapDatabaseFiles[nNewFileIndex] = pNewFile;

        nDBFile   = nExpectantFileIndex;
        nPosition = nExpectantPosition;
    }
    return true;
}


void DataBase::releaseBlock(const DBBlockInfo &infoDBBlock)
{
    DatabaseFile *pFile = NULL;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDataBase);
        std::map<unsigned, OpenSP::sp<DatabaseFile> >::iterator itorFile = m_mapDatabaseFiles.find(infoDBBlock.m_nDBFile);
        if(itorFile != m_mapDatabaseFiles.end())
        {
            pFile = itorFile->second;
        }
    }

    if(NULL != pFile)
    {
        pFile->releaseBlock(infoDBBlock.m_gap);
    }
}


}

