#ifndef _TILINGTHREAD_H
#define _TILINGTHREAD_H

#include "OpenThreads/Thread"
#include "OpenSP/sp.h"
#include "OpenThreads/Block"
#include "OpenThreads/Atomic"
#include "EventAdapter/EventAdapter.h"
#include <DEUDBProxy/IDEUDBProxy.h>

class TaskQueue;

struct TileInfo
{
    int m_nLevel;
    int m_nRow;
    int m_nCol;
};

typedef std::vector<TileInfo> TileInfoVec;
extern const unsigned BYSIDE;

class HeightFieldMemory
{
public:
    HeightFieldMemory(unsigned int nTileSize)
    {
        m_nTileSize = nTileSize + BYSIDE;

        m_pData = new float[m_nTileSize * m_nTileSize];
        for (unsigned int i=0; i<m_nTileSize*m_nTileSize; i++)
        {
            m_pData[i] = -1000000.0;
        }
    }

    ~HeightFieldMemory()
    {
        delete [] m_pData;
    }

public:
    float* &getMemoryData()
    {
        return m_pData;
    }

    void reset()
    {
        for (unsigned int i=0; i<m_nTileSize*m_nTileSize; i++)
        {
            m_pData[i] = -1000000.0;
        }
    }

    bool empty()
    {
        float* pBuf = new float[m_nTileSize * m_nTileSize];
        for (unsigned int i=0; i<m_nTileSize*m_nTileSize; i++)
        {
            pBuf[i] = -1000000.0;
        }

        if (memcmp(m_pData, pBuf, m_nTileSize*m_nTileSize*sizeof(float)) != 0)
        {
            delete [] pBuf;
            return false;
        }

        delete [] pBuf;

        return true;
    }

private:
    unsigned int m_nTileSize;
    float*       m_pData;
};

class RasterMemory
{
public:
    RasterMemory(unsigned int nTileSize, unsigned int nBandCount)
    {
        m_nTileSize  = nTileSize;
        m_nBandCount = nBandCount;

        m_pAlphaData = new char[m_nTileSize * m_nTileSize];
        memset(m_pAlphaData, 255, m_nTileSize*m_nTileSize);

        for (unsigned int i=0; i<m_nBandCount; i++)
        {
            char* pBuf = new char[m_nTileSize * m_nTileSize];
            memset(pBuf, 0, m_nTileSize*m_nTileSize);
            m_pRasterDataVec.push_back(pBuf);
        }
    }

    ~RasterMemory()
    {
        delete [] m_pAlphaData;

        for (unsigned int i=0; i<m_nBandCount; i++)
        {
            char* pBuf = (char*)m_pRasterDataVec[i];
            delete [] pBuf;
        }

        m_pRasterDataVec.clear();
    }

public:
    std::vector<char*>& getRasterData()
    {
        return m_pRasterDataVec;
    }

    bool needAlpha()
    {
        if (m_nBandCount != 4)
        {
            return false;
        }

        if (memcmp(m_pRasterDataVec[3], m_pAlphaData, m_nTileSize*m_nTileSize) == 0)
        {
            return false;
        }

        return true;
    }

    bool checkAlpha()
    {
        if (m_nBandCount == 3)
        {
            return true;
        }

        for (unsigned int i = 0; i < m_nTileSize*m_nTileSize; i++)
        {
            if (m_pRasterDataVec[3][i] > 0)
            {
                m_pRasterDataVec[3][i] = (char)255;
            }
        }

        return true;
    }

    bool deleteAlpha()
    {
        if (m_nBandCount == 3)
        {
            return true;
        }

        delete [] m_pRasterDataVec[3];
        m_pRasterDataVec.pop_back();

        m_nBandCount--;

        return true;
    }

    void reset()
    {
        for (unsigned int i=0; i<m_pRasterDataVec.size(); i++)
        {
            memset(m_pRasterDataVec[i], 0, m_nTileSize*m_nTileSize);
        }

        if (m_nBandCount == 3)
        {
            char* pBuf = new char[m_nTileSize * m_nTileSize];
            memset(pBuf, 0, m_nTileSize*m_nTileSize);
            m_pRasterDataVec.push_back(pBuf);

            m_nBandCount++;
        }
    }

    bool empty()
    {
        std::vector<char>   vecBuf(m_nTileSize * m_nTileSize);
        memset(vecBuf.data(), 0, vecBuf.size());

        if (m_nBandCount == 4 && memcmp(m_pRasterDataVec[3], vecBuf.data(), vecBuf.size()) == 0)
        {
            return true;
        }

        for (unsigned int i=0; i<m_pRasterDataVec.size(); i++)
        {
            if (memcmp(m_pRasterDataVec[i], vecBuf.data(), vecBuf.size()) != 0)
            {
                return false;
            }
        }

        return true;
    }

private:
    char*           m_pAlphaData;
    unsigned int    m_nTileSize;
    unsigned int    m_nBandCount;
    std::vector<char*> m_pRasterDataVec;
};

class TilingThread : public OpenThreads::Thread
{
public:
    TilingThread(TaskQueue* pTaskQueue, deudbProxy::IDEUDBProxy *pTargetDB);
    ~TilingThread(void);

public:
    void run(void);
    void finishMission(bool bCancel);
    void sleep();
    void wakeup();

private:
    void getChildren(const TileInfo& ParentInfo, TileInfoVec &ChildrenInfo);
    bool mergeWithChildren(const ID &parentID, unsigned nTileSize);
    bool readTIFStreamFromDB(const void* pBuffer, unsigned int nLen, unsigned char*&pRaster, unsigned &nBland);
    bool writeImageStreamToDB(const ID &id, std::vector<char*> &pBufVec, unsigned nXsize, unsigned nYsize, unsigned nBland);
    bool writeMergeImageStreamToDB(const ID &id, char* pBuf, unsigned nXsize, unsigned nYsize, unsigned nBland);
    bool writeHeightFieldStreamToDB(const ID &id, float* pBuf, unsigned nXsize, unsigned nYsize);
    bool writeMergeHeightFieldStreamToDB(const ID &id, float* pBuf, unsigned nXsize, unsigned nYsize);
    void mergeInvalidColor(const int nBandCount, const unsigned int nTileSize, std::vector<char*> &RasterDataVec);

private:
    TaskQueue*               m_pTaskQueue;
    deudbProxy::IDEUDBProxy* m_pTargetDB;
    OpenThreads::Block       m_blockSuspend;
    OpenThreads::Atomic      m_bMissionFinished;
    OpenThreads::Mutex       m_mtxSaveFile;
};

#endif
