#include "DatabaseFile.h"
#include <OpenThreads/ScopedLock>
#include <algorithm>
#include <vector>
#ifdef WIN32
#include <io.h>
#endif
#include "Common/Common.h"

#include "WorkingThreads.h"

#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif

namespace deudb
{

const UINT_64 __DB_ONE_BILLION_ = 1024u * 1024u * 1024u;
const UINT_64 DatabaseFile::m_nFileSizeLimited = __DB_ONE_BILLION_ * 20u;

DatabaseFile::DatabaseFile(void)
{
    m_pFile            = NULL;
    m_nCurrentFileSize = 0u;
#ifdef WIN32
    m_nAllocFileSize = 16*1024*1024i64;
#else
    m_nAllocFileSize = 16*1024*1024LL;
#endif
}


DatabaseFile::~DatabaseFile(void)
{
    closeFile();
}


struct GapSorter
{
    bool operator()(const FileGap &left, const FileGap &right)
    {
        if(left.m_nPosition < right.m_nPosition)   
        {
            return true;
        }
        else if(left.m_nPosition > right.m_nPosition)   
        {
            return false;
        }
        else
        {
            if(left.m_nLength < right.m_nLength) 
            {
                return true;
            }
            return false;
        }
    }
};

UINT_64 DatabaseFile::getFileLength()
{
#if defined (WIN32) || defined (WIN64)
    _fseeki64(m_pFile, 0, SEEK_END);
    const UINT_64 nLength = _ftelli64(m_pFile);
    _fseeki64(m_pFile, 0, SEEK_SET);
#else
    fseeko(m_pFile, 0, SEEK_END);
    const UINT_64 nLength = ftello(m_pFile);
    fseeko(m_pFile, 0, SEEK_SET);
    
#endif
    return nLength;
}

bool DatabaseFile::init(const std::string &strFilePath, const std::vector<FileGap> &vecWhiteGap)
{
    const bool bIsFileExist = cmm::isFileExist(strFilePath);
    if(bIsFileExist)
    {
        m_pFile            = fopen(strFilePath.c_str(), "rb+");
        m_nCurrentFileSize = getFileLength();
        //m_nCurrentFileSize = 0u;//getFileLength(m_pFile);
    }
    else
    {
        m_pFile = fopen(strFilePath.c_str(), "wb");
        if(NULL == m_pFile)
        {
            return false;
        }
        fclose(m_pFile);

        m_pFile = fopen(strFilePath.c_str(), "rb+");
        m_nCurrentFileSize = 0u;
    }
    if(NULL == m_pFile)
    {
        return false;
    }

    if(bIsFileExist && !vecWhiteGap.empty())
    {
        std::vector<FileGap>  vecGaps(vecWhiteGap.begin(), vecWhiteGap.end());
        std::sort(vecGaps.begin(), vecGaps.end(), GapSorter());
        //listGaps.sort(GapSorter());

        const FileGap &lastGap = vecGaps.back();
        UINT_64 nLast = lastGap.m_nPosition + lastGap.m_nLength;

        std::vector<FileGap>::const_iterator itorGap0 = vecGaps.begin();
        std::vector<FileGap>::const_iterator itorGap1 = vecGaps.begin();

        if(itorGap0->m_nPosition != 0)
        {
            FileGap gap;
            gap.m_nPosition = 0;
            gap.m_nLength = itorGap0->m_nPosition;
            m_listBlackGap.push_back(gap);
        }

        for(++itorGap1 ; itorGap1 != vecGaps.end(); ++itorGap0, ++itorGap1)
        {
            const FileGap &gap0 = *itorGap0;
            const FileGap &gap1 = *itorGap1;

            const UINT_64 nEndPosition = gap0.m_nPosition + gap0.m_nLength;
            if(nEndPosition >= gap1.m_nPosition)
            {
                continue;
            }

            FileGap gap;
            gap.m_nPosition = nEndPosition;
            gap.m_nLength   = unsigned(gap1.m_nPosition - nEndPosition);
            m_listBlackGap.push_back(gap);
        }

        if(m_nCurrentFileSize > nLast)
        {
            FileGap tempGap;
            tempGap.m_nPosition = nLast;
            tempGap.m_nLength = m_nCurrentFileSize - nLast;
            m_listBlackGap.push_back(tempGap);
        }

    }

    return true;
}

bool DatabaseFile::chFileSize(UINT_64 nSize)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxFile);
#ifdef WIN32
    if(_chsize_s(fileno(m_pFile),nSize) == 0)
        return true;
    else
        return false;
#else
    if(fseek(m_pFile,nSize-1,SEEK_SET) != 0)
        return false;
    fwrite("a",1,1,m_pFile);
    if(fflush(m_pFile) == 0)
        return true;
    else
        return false;
#endif
}

UINT_64 DatabaseFile::allocBlock(unsigned nLength)
{
    if(nLength < 1u)
    {
        return 0u;
    }

    UINT_64 nExpectantPosition = ULLONG_MAX;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlack(m_mtxBlackGap);
        std::list<FileGap>::iterator itorGap = m_listBlackGap.begin();
        for( ; itorGap != m_listBlackGap.end(); ++itorGap)
        {
            const FileGap &gap = *itorGap;
            if(gap.m_nLength >= nLength)
            {
                break;
            }
        }

        if(itorGap == m_listBlackGap.end())
        {
            if(m_nCurrentFileSize + nLength > m_nFileSizeLimited)
            {
                // the memory block will not fit in this file
                return ULLONG_MAX;
            }
            while(m_nAllocFileSize < nLength)
            {
                m_nAllocFileSize = m_nAllocFileSize*2;
            }

            if(m_nCurrentFileSize + m_nAllocFileSize > m_nFileSizeLimited)
            {
                m_nAllocFileSize = m_nFileSizeLimited - m_nCurrentFileSize;
                if(m_nAllocFileSize < nLength)
                {
                    return ULLONG_MAX;
                }
            }

            if(m_listBlackGap.empty())
            {
                //alloc memory
                while(m_nAllocFileSize >= nLength)
                {
                    if(chFileSize(m_nCurrentFileSize + m_nAllocFileSize))
                    {
                        nExpectantPosition = m_nCurrentFileSize;
                        m_nCurrentFileSize += m_nAllocFileSize;

                        if((m_nAllocFileSize - nLength) > 0)
                        {

                            FileGap gapTarget;
                            gapTarget.m_nPosition = nExpectantPosition + nLength;
                            gapTarget.m_nLength = m_nAllocFileSize - nLength;
                            m_listBlackGap.push_back(gapTarget);
                        }

                        if(m_nAllocFileSize < 1024*1024*1024)
                            m_nAllocFileSize = m_nAllocFileSize*2;
                        return nExpectantPosition;
                    }
                    else
                    {
                        m_nAllocFileSize = m_nAllocFileSize / 2;
                        continue;
                    }
                }
                return ULLONG_MAX-1;
            }
            else
            {
                FileGap& lastGap = m_listBlackGap.back();
                UINT_64 nLastPos = lastGap.m_nPosition;
                UINT_64 nLastLength = lastGap.m_nLength;

                //alloc memory
                if((nLastPos + nLastLength) == m_nCurrentFileSize)
                {
                    while((m_nAllocFileSize + nLastLength) >= nLength)
                    {
                        if(chFileSize(m_nCurrentFileSize + m_nAllocFileSize))
                        {
                            nExpectantPosition = nLastPos;
                            m_nCurrentFileSize += m_nAllocFileSize;

                            if((m_nAllocFileSize + nLastLength - nLength) > 0)
                            {
                                lastGap.m_nPosition = nExpectantPosition + nLength;
                                lastGap.m_nLength = m_nAllocFileSize + nLastLength - nLength;
                            }

                            if(m_nAllocFileSize < 1024*1024*1024)
                                m_nAllocFileSize = m_nAllocFileSize*2;
                            return nExpectantPosition;
                        }
                        else
                        {
                            m_nAllocFileSize = m_nAllocFileSize / 2;
                            continue;
                        }
                    }
                }
                else
                {
                    while(m_nAllocFileSize >= nLength)
                    {
                        if(chFileSize(m_nCurrentFileSize + m_nAllocFileSize))
                        {
                            nExpectantPosition = m_nCurrentFileSize;
                            m_nCurrentFileSize += m_nAllocFileSize;

                            if((m_nAllocFileSize - nLength) > 0)
                            {

                                FileGap gapTarget;
                                gapTarget.m_nPosition = nExpectantPosition + nLength;
                                gapTarget.m_nLength = m_nAllocFileSize - nLength;
                                m_listBlackGap.push_back(gapTarget);
                            }

                            if(m_nAllocFileSize < 1024*1024*1024)
                                m_nAllocFileSize = m_nAllocFileSize*2;
                            return nExpectantPosition;
                        }
                        else
                        {
                            m_nAllocFileSize = m_nAllocFileSize / 2;
                            continue;
                        }
                    }
                }
                return ULLONG_MAX-1;
            }
        }
        else
        {
            FileGap &gapTarget = *itorGap;

            nExpectantPosition = gapTarget.m_nPosition;

            if(gapTarget.m_nLength == nLength)
            {
                m_listBlackGap.erase(itorGap);
            }
            else
            {
                gapTarget.m_nPosition += nLength;
                gapTarget.m_nLength   -= nLength;
            }
        }
    }

    return nExpectantPosition;
}


void DatabaseFile::releaseBlock(const FileGap &gap)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lockBlack(m_mtxBlackGap);

    if(m_listBlackGap.empty())
    {
        m_listBlackGap.push_back(gap);
        return;
    }
    std::list<FileGap>::iterator itor0 = m_listBlackGap.begin();
    while(itor0 != m_listBlackGap.end())
    {
        FileGap &gap0 = *itor0;
        if((gap.m_nPosition + gap.m_nLength) < gap0.m_nPosition)
        {
            m_listBlackGap.insert(itor0,gap);
            return;
        }
        else if((gap0.m_nPosition + gap0.m_nLength) < gap.m_nPosition)
        {
            itor0++;
            continue;
        }
        else if((gap0.m_nPosition <= (gap.m_nPosition + gap.m_nLength)) && ((gap.m_nPosition + gap.m_nLength) <= (gap0.m_nPosition + gap0.m_nLength)))
        {
            gap0.m_nLength = (gap0.m_nPosition + gap0.m_nLength) - std::min(gap0.m_nPosition,gap.m_nPosition);
            gap0.m_nPosition = std::min(gap0.m_nPosition,gap.m_nPosition);
            return;
        }
        else if((gap.m_nPosition <= (gap0.m_nPosition + gap0.m_nLength)) && ((gap.m_nPosition + gap.m_nLength) >= (gap0.m_nPosition + gap0.m_nLength)))
        {
            std::list<FileGap>::iterator itor1 = ++itor0;
            while(itor1 != m_listBlackGap.end())
            {
                FileGap &gap1 = *itor1;
                if((gap.m_nPosition + gap.m_nLength) < gap1.m_nPosition)
                {
                    gap0.m_nPosition = std::min(gap0.m_nPosition,gap.m_nPosition);
                    gap0.m_nLength = gap.m_nPosition + gap.m_nLength - gap0.m_nPosition;
                    return;
                }
                else if(((gap.m_nPosition + gap.m_nLength) >=  gap1.m_nPosition) && ((gap.m_nPosition + gap.m_nLength) <= (gap1.m_nPosition + gap1.m_nLength)))
                {
                    gap0.m_nPosition = std::min(gap0.m_nPosition,gap.m_nPosition);
                    gap0.m_nLength = gap1.m_nPosition + gap1.m_nLength - gap0.m_nPosition;
                    m_listBlackGap.erase(itor1);
                    return;
                }
                else
                {
                    itor1 = m_listBlackGap.erase(itor1);
                    continue;
                }
            }
            gap0.m_nPosition = std::min(gap0.m_nPosition,gap.m_nPosition);
            gap0.m_nLength = gap.m_nPosition + gap.m_nLength - gap0.m_nPosition;
            return;
        }
    }
    m_listBlackGap.push_back(gap);
    return;
}


void *DatabaseFile::readBlock(const FileGap &gap)
{
    if(gap.m_nLength < 1u || gap.m_nPosition >= m_nFileSizeLimited)
    {
        return NULL;
    }

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lockFile(m_mtxFile);
#if defined (WIN32) || defined (WIN64)
        if(_fseeki64(m_pFile, gap.m_nPosition, SEEK_SET) != 0)
#else
        if(fseeko(m_pFile, gap.m_nPosition, SEEK_SET) != 0)
#endif
        {
            return NULL;
        }

        void *pMemory = malloc(gap.m_nLength);
        if(fread(pMemory, gap.m_nLength, 1, m_pFile) != 1)
        {
            free(pMemory);
            return NULL;
        }
        return pMemory;
    }

    return NULL;
}


void DatabaseFile::closeFile(void)
{
    if(NULL != m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    m_listBlackGap.clear();
}


bool DatabaseFile::writeBlock(const FileGap &gap, const void *pDataBlock)
{
    if(gap.m_nLength <= 0 || !pDataBlock)
    {
        return true;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxFile);
#if defined (WIN32) || defined (WIN64)
    if(gap.m_nPosition == ~0ui64)
    {
        //_fseeki64(m_pFile, 0, SEEK_END);
        return false;
    }
    else
    {
        _fseeki64(m_pFile, gap.m_nPosition, SEEK_SET);
    }
#else
    if(gap.m_nPosition == ~0uLL)
    {
        //fseeko(m_pFile, 0, SEEK_END);
        return false;
    }
    else
    {
        fseeko(m_pFile, gap.m_nPosition, SEEK_SET);
    }
#endif
    const size_t nRet = fwrite(pDataBlock, gap.m_nLength, 1, m_pFile);
#ifndef WIN32
    fflush(m_pFile);
#endif
    return (nRet == 1);
}


}
