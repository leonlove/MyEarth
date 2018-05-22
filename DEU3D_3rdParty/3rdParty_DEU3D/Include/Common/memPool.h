#ifndef MEM_POOL_H_3A4A8B98_66EF_4221_BF22_E1E4327310F6_INCLUDE
#define MEM_POOL_H_3A4A8B98_66EF_4221_BF22_E1E4327310F6_INCLUDE

#include <OpenThreads/Mutex>
#include <OpenThreads/Block>
#include "Export.h"
#include <vector>
#include <algorithm>
#include <fstream>

namespace cmm
{
#pragma warning(disable:4251)
#define MP_LOG_OUT  0

class CM_EXPORT MemPool
{
public:
    // MemPool must be constructed with a suitable 'nInitBlockCount', which can improve the performance a lot.
    // Please do not put your performance upon MemPool which has an unsuitable 'nInitBlockCount'.
    // For our system, the default value of 'nInitBlockCount' is 65536, I think this value is the best.
    explicit MemPool(unsigned nBlockSize, unsigned nInitBlockCount = 1024u * 512u, bool bMultiThread = true, const std::string &strPoolName = "")
        : m_nBlockSize(nBlockSize),
        m_nInitBlockCount(nInitBlockCount),
        m_bMultiThread(bMultiThread),
        m_strPoolName(strPoolName)
    {
        m_listBuckets.reserve(16u);

#if MP_LOG_OUT
        m_nMaxUsed = 0u;
        if(!strPoolName.empty())
        {
            std::string strPath = "c:\\";
            strPath += strPoolName;
            strPath += ".log";
            m_fileLog.open(strPath);//, std::ios_base::out);
        }
#endif
    }

    ~MemPool(void)
    {
        std::vector<Bucket *>::iterator itor = m_listBuckets.begin();
        for( ; itor != m_listBuckets.end(); ++itor)
        {
            delete *itor;
        }
    }

protected:
    class Bucket
    {
        friend class MemPool;

    protected:
        inline explicit Bucket(unsigned nBlockSize, unsigned nBlockCount)
            : m_nBlockSize(nBlockSize),
            m_nBlockCount(nBlockCount),
            m_nEmptyCount(nBlockCount)
        {
            const unsigned nBucketSize = nBlockSize * nBlockCount;
            m_pMemoryBufferBegin = new unsigned char[nBucketSize];
            m_pMemoryBufferEnd   = m_pMemoryBufferBegin + nBucketSize;
            m_vecEmptyBlock = new unsigned int [nBlockCount];
            for(unsigned n = 0u; n < nBlockCount; n++)
            {
                m_vecEmptyBlock[n] = n;
            }
        }

        inline ~Bucket(void)
        {
            delete[] m_pMemoryBufferBegin;
            delete[] m_vecEmptyBlock;
        }

    protected:
        inline void *alloc(void)
        {
            if(m_nEmptyCount < 1u)
            {
                return NULL;
            }

            const unsigned nOffset = m_vecEmptyBlock[0];
            unsigned char *pResult = m_pMemoryBufferBegin + nOffset * m_nBlockSize;

            m_nEmptyCount--;
            m_vecEmptyBlock[0] = m_vecEmptyBlock[m_nEmptyCount];

            return pResult;
        }

        inline void free(void *p)
        {
            unsigned char *pFree = (unsigned char *)p;
            const unsigned nOffset = (pFree - m_pMemoryBufferBegin) / m_nBlockSize;
            if(nOffset >= m_nBlockCount)
            {
                return;
            }
            m_vecEmptyBlock[m_nEmptyCount] = nOffset;
            m_nEmptyCount++;
        }

        inline bool inMyContent(void *p) const  {   return ((p >= m_pMemoryBufferBegin) && (p < m_pMemoryBufferEnd));   }
        inline bool hasRoom(void) const         {   return m_nEmptyCount > 0u;                                          }
        inline bool isEmpty(void) const         {   return m_nEmptyCount >= m_nBlockCount;                              }

    protected:
        const unsigned int  m_nBlockSize;
        const unsigned int  m_nBlockCount;
        unsigned char      *m_pMemoryBufferBegin;
        unsigned char      *m_pMemoryBufferEnd;
        unsigned int       *m_vecEmptyBlock;
        unsigned int        m_nEmptyCount;
    };

    struct LowerBound
    {
        inline bool operator()(const Bucket *pItor, const Bucket &val) const
        {
            return (pItor->m_pMemoryBufferBegin < val.m_pMemoryBufferBegin);
        }
        inline bool operator()(const Bucket *pItor, void *pVal) const 
        {
            return (pItor->m_pMemoryBufferEnd <= pVal);
        }
    };

public:
    inline void *alloc(void)
    {
        if(m_bMultiThread)
        {
            m_mtxListBuckets.lock();
        }

        Bucket *pBucket = NULL;
        for(std::vector<Bucket *>::iterator itor = m_listBuckets.begin(); itor != m_listBuckets.end(); ++itor)
        {
            Bucket *pFind = *itor;

            // find the first warehouse which has enough room
            if(pFind->hasRoom())
            {
                pBucket = pFind;
                break;
            }
        }

        if(pBucket == NULL)
        {
            pBucket = new Bucket(m_nBlockSize, m_nInitBlockCount);

            std::vector<Bucket *>::const_iterator itorPos = std::lower_bound(m_listBuckets.begin(), m_listBuckets.end(), *pBucket, LowerBound());
            m_listBuckets.insert(itorPos, pBucket);
        }

        void *p = pBucket->alloc();

#if MP_LOG_OUT
        unsigned nUsedCount = 0u;
        for(std::vector<Bucket *>::iterator itor = m_listBuckets.begin(); itor != m_listBuckets.end(); ++itor)
        {
            Bucket *pFind = *itor;
            const unsigned nUsed = pFind->m_nBlockCount - pFind->m_nEmptyCount;
            nUsedCount += nUsed;
        }
        if(nUsedCount > m_nMaxUsed)
        {
            m_nMaxUsed = nUsedCount;
            m_fileLog << "MaxUsed of \t\"" << m_strPoolName.c_str() << "\"\t=\t" << m_nMaxUsed << std::endl;
        }
#endif

        if(m_bMultiThread)
        {
            m_mtxListBuckets.unlock();
        }

        return p;
    }

    inline void free(void *p)
    {
        if(m_bMultiThread)
        {
            m_mtxListBuckets.lock();
        }

        std::vector<Bucket *>::const_iterator itorFind = std::lower_bound(m_listBuckets.begin(), m_listBuckets.end(), p, LowerBound());
        if(itorFind != m_listBuckets.end())
        {
            Bucket *pBucket = *itorFind;
            pBucket->free(p);
            if(pBucket->isEmpty() && m_listBuckets.size() > 1u)
            {
                delete pBucket;
                m_listBuckets.erase(itorFind);
            }
        }

        if(m_bMultiThread)
        {
            m_mtxListBuckets.unlock();
        }
    }

protected:
    const unsigned int          m_nBlockSize;
    const unsigned int          m_nInitBlockCount;
    const bool                  m_bMultiThread;

    std::vector<Bucket *>       m_listBuckets;
    OpenThreads::Mutex          m_mtxListBuckets;

    std::string                 m_strPoolName;

#if MP_LOG_OUT
    unsigned                    m_nMaxUsed;
    std::ofstream               m_fileLog;
#endif
};

}

#endif
