#include "TilingThread.h"
#include "TaskQueue.h"
#include "Common/Pyramid.h"
#include "Common/deuImage.h"
#include "Common/deuImage.h"
#include "EventAdapter/IEventObject.h"
#include "ReadWriteTiFF.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <strstream>
#include <vector>
#include <IDProvider/Definer.h>
#include "TileBuilderImpl.h"

const unsigned BYSIDE = 2u;
using namespace std;

TilingThread::TilingThread(TaskQueue* pTaskQueue, deudbProxy::IDEUDBProxy *pTargetDB)
: m_pTaskQueue(pTaskQueue)
, m_pTargetDB(pTargetDB)
{
    m_bMissionFinished.exchange(0);
}

TilingThread::~TilingThread(void)
{
}

void TilingThread::run(void)
{
    if (m_pTaskQueue == NULL || m_pTargetDB == NULL)
    {
        return;
    }

    //初始化切片光栅数据
    const unsigned int nTileSize   = m_pTaskQueue->getTileSize();
    const unsigned int nBandCount = (m_pTaskQueue->getHeightField() ? 1u : 4u);

    RasterMemory memDataChar(nTileSize, nBandCount);
    HeightFieldMemory memDataFloat(nTileSize);

    while((unsigned)m_bMissionFinished != 1)
    {
        m_blockSuspend.block();

        OpenSP::sp<TilingTask> task;
        const bool bTake = m_pTaskQueue->takeTask(task);
        switch ((unsigned int)m_bMissionFinished)
        {
        case 1:
        case 2:
            if (!bTake) return;
            break;
        default:
            if (!bTake) sleep();
            break;
        }

        if (!bTake) continue;

        if (m_pTaskQueue->getHeightField())
        {
            memDataFloat.reset();
        }
        else
        {
            memDataChar.reset();
        }

        // 逐个文件生成切片
        ID               id;
        bool             bMerge = false;
        cmm::math::Box2d box2d;
        memset(&id, 0, sizeof(id));
        id.TileID.m_nDataSetCode = m_pTaskQueue->getDataSetCode();
        id.TileID.m_nUniqueID    = m_pTaskQueue->getGlobeUniqueNumber();
        id.TileID.m_nLevel       = task->m_nLevel;
        id.TileID.m_nRow         = task->m_nRow;
        id.TileID.m_nCol         = task->m_nCol;
        id.TileID.m_nType        = (m_pTaskQueue->getHeightField() ? TERRAIN_TILE_HEIGHT_FIELD : TERRAIN_TILE_IMAGE);
        const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
        if (pPyramid == NULL)
        {
            return;
        }

        for (unsigned int i=0; i<task->m_vecSourceFiles.size(); i++)
        {
            if (!m_pTaskQueue->getHeightField() && !memDataChar.needAlpha())
            {
                break;
            }

            //首先进行四合一，如果4个子切片没有生成完全，那么就重新进行计算
            if (mergeWithChildren(id, nTileSize))
            {
                bMerge = true;
                break;
            }
            else
            {
                cmm::math::Point2d ptMin, ptMax;
                pPyramid->getTilePos(task->m_nLevel, task->m_nRow, task->m_nCol, ptMin.x(), ptMin.y(), ptMax.x(), ptMax.y());
                box2d.set(ptMin, ptMax);
                if (m_pTaskQueue->getHeightField())
                {
                    if (BYSIDE == 2u)
                    {
                        double xPixel = fabs(box2d.point0().x() - box2d.point1().x()) / (nTileSize-1);
                        double yPixel = fabs(box2d.point0().y() - box2d.point1().y()) / (nTileSize-1);

                        ptMin.x() -= xPixel;
                        ptMin.y() -= yPixel;
                        ptMax.x() += xPixel;
                        ptMax.y() += yPixel;
                        box2d.set(ptMin, ptMax);
                    }

                    task->m_vecSourceFiles[i]->readTileForHeightField(box2d, nTileSize + BYSIDE, &memDataFloat.getMemoryData());
                }
                else
                {
                    task->m_vecSourceFiles[i]->readTileForImage(box2d, nTileSize, memDataChar.getRasterData());
                }
            }
        }

        if (bMerge)
        {
            //为进度条设置信息
            unsigned int nResult = 1;
            unsigned int nState  = 2;
            OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
            if (pEventObject == NULL)
            {
                continue;
            }

            pEventObject->setAction(ea::ACTION_TILE_BUILDER);
            pEventObject->putExtra("STATE", nState);
            pEventObject->putExtra("Result", nResult);
            pEventObject->putExtra("Level", task->m_nLevel);
            pEventObject->putExtra("Row", task->m_nRow);
            pEventObject->putExtra("Col", task->m_nCol);
            m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());

            continue;
        }

        if (m_pTaskQueue->getHeightField())
        {
            //为进度条设置信息
            unsigned int nResult = 1;
            unsigned int nState  = 2;
            OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
            if (pEventObject == NULL)
            {
                continue;
            }

            pEventObject->setAction(ea::ACTION_TILE_BUILDER);
            pEventObject->putExtra("STATE", nState);
            pEventObject->putExtra("Level", task->m_nLevel);
            pEventObject->putExtra("Row", task->m_nRow);
            pEventObject->putExtra("Col", task->m_nCol);

            if (memDataFloat.empty())
            {
                nResult = 2;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());

                continue;
            }

            if (writeHeightFieldStreamToDB(id, memDataFloat.getMemoryData(), nTileSize, nTileSize))
            {
                nResult = 1;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());
            }
            else
            {
                nResult = 0;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());
            }
        }
        else
        {
            //为进度条设置信息 FAILED = -1 ; DROPPED = 0 ; SUCCESS = 1 ;
            unsigned int nResult = 1;
            unsigned int nState  = 2;
            OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
            if (pEventObject == NULL)
            {
                continue;
            }

            pEventObject->setAction(ea::ACTION_TILE_BUILDER);
            pEventObject->putExtra("STATE", nState);
            pEventObject->putExtra("Level", task->m_nLevel);
            pEventObject->putExtra("Row", task->m_nRow);
            pEventObject->putExtra("Col", task->m_nCol);

            //mergeInvalidColor(nBandCount, nTileSize, memDataChar.getRasterData());

            if (memDataChar.empty())
            {
                nResult = 2;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());

                continue;
            }

            if (!memDataChar.needAlpha())
            {
                memDataChar.deleteAlpha();
            }

            if (writeImageStreamToDB(id, memDataChar.getRasterData(), nTileSize, nTileSize, memDataChar.getRasterData().size()))
            {
                nResult = 1;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());
            }
            else
            {
                nResult = 0;
                pEventObject->putExtra("Result", nResult);
                m_pTaskQueue->getIEventAdapter()->sendBroadcast(pEventObject.get());
            }
        }
    }

    return;
}

void TilingThread::finishMission(bool bCancel)
{
    if(bCancel)
    {
        // 外部要求取消任务，那么这种情况下，不论任务队列中还有多少任务，都不予理睬，直接退出线程
        m_bMissionFinished.exchange(1);
    }
    else
    {
        // 外部要求将任务队列中的任务都做完才能退出
        m_bMissionFinished.exchange(2);
    }

    wakeup();

    return;
}

void TilingThread::sleep()
{
    m_blockSuspend.set(false);
}

void TilingThread::wakeup()
{
    m_blockSuspend.set(true);
}

void TilingThread::getChildren(const TileInfo& ParentInfo, TileInfoVec &ChildrenInfo)
{
    TileInfo childInfo;
    childInfo.m_nLevel = ParentInfo.m_nLevel + 1;

    childInfo.m_nRow = ParentInfo.m_nRow * 2;
    childInfo.m_nCol = ParentInfo.m_nCol * 2;
    ChildrenInfo.push_back(childInfo);

    childInfo.m_nRow = ParentInfo.m_nRow * 2;
    childInfo.m_nCol = ParentInfo.m_nCol * 2 + 1;
    ChildrenInfo.push_back(childInfo);

    childInfo.m_nRow = ParentInfo.m_nRow * 2 + 1;
    childInfo.m_nCol = ParentInfo.m_nCol * 2;
    ChildrenInfo.push_back(childInfo);

    childInfo.m_nRow = ParentInfo.m_nRow * 2 + 1;
    childInfo.m_nCol = ParentInfo.m_nCol * 2 + 1;
    ChildrenInfo.push_back(childInfo);

    return;
}

bool TilingThread::mergeWithChildren(const ID &parentID, unsigned nTileSize)
{
    if (m_pTaskQueue->getHeightField() || m_pTaskQueue == NULL || m_pTargetDB == NULL)
    {
        return false;
    }

    ID          id;
    TileInfo    ParentInfo;
    TileInfoVec ChildrenInfo;
    ParentInfo.m_nLevel = parentID.TileID.m_nLevel;
    ParentInfo.m_nRow   = parentID.TileID.m_nRow;
    ParentInfo.m_nCol   = parentID.TileID.m_nCol;
    getChildren(ParentInfo, ChildrenInfo);

    memset(&id, 0, sizeof(id));
    id.TileID.m_nDataSetCode = m_pTaskQueue->getDataSetCode();
    id.TileID.m_nUniqueID    = m_pTaskQueue->getGlobeUniqueNumber();
    id.TileID.m_nType        = TERRAIN_TILE_IMAGE;
    for (unsigned int iChild=0; iChild<ChildrenInfo.size(); iChild++)
    {
        id.TileID.m_nLevel = ChildrenInfo[iChild].m_nLevel;
        id.TileID.m_nRow   = ChildrenInfo[iChild].m_nRow;
        id.TileID.m_nCol   = ChildrenInfo[iChild].m_nCol;
        if (!m_pTargetDB->isExist(id))
        {
            return false;
        }
    }

    //分别取出四张图片的光栅数据
    unsigned int   nBland  = 0u;
    unsigned int   nBland1 = 0u;
    unsigned int   nBland2 = 0u;
    unsigned int   nBland3 = 0u;
    unsigned int   nBland4 = 0u;
    unsigned char* pRaster1 = NULL;
    unsigned char* pRaster2 = NULL;
    unsigned char* pRaster3 = NULL;
    unsigned char* pRaster4 = NULL;
    bool           bNeedAlpha = false;
    void*          pImage1 = NULL;
    void*          pImage2 = NULL;
    void*          pImage3 = NULL;
    void*          pImage4 = NULL;
    unsigned int nLen = 0;

    //图片1
    id.TileID.m_nLevel = ChildrenInfo[0].m_nLevel;
    id.TileID.m_nRow   = ChildrenInfo[0].m_nRow;
    id.TileID.m_nCol   = ChildrenInfo[0].m_nCol;
    m_pTargetDB->readBlock(id, pImage1, nLen);
    if (!readTIFStreamFromDB(pImage1, nLen, pRaster1, nBland1))
    {
        deudbProxy::freeMemory(pImage1);
        deudbProxy::freeMemory(pImage2);
        deudbProxy::freeMemory(pImage3);
        deudbProxy::freeMemory(pImage4);

        if (pRaster1 != NULL) delete pRaster1;
        if (pRaster2 != NULL) delete pRaster2;
        if (pRaster3 != NULL) delete pRaster3;
        if (pRaster4 != NULL) delete pRaster4;

        return false;
    }
    if (nBland1 == 4)
    {
        bNeedAlpha = true;
    }

    //图片2
    id.TileID.m_nLevel = ChildrenInfo[1].m_nLevel;
    id.TileID.m_nRow   = ChildrenInfo[1].m_nRow;
    id.TileID.m_nCol   = ChildrenInfo[1].m_nCol;
    m_pTargetDB->readBlock(id, pImage2, nLen);
    if (!readTIFStreamFromDB(pImage2, nLen, pRaster2, nBland2))
    {
        deudbProxy::freeMemory(pImage1);
        deudbProxy::freeMemory(pImage2);
        deudbProxy::freeMemory(pImage3);
        deudbProxy::freeMemory(pImage4);

        if (pRaster1 != NULL) delete pRaster1;
        if (pRaster2 != NULL) delete pRaster2;
        if (pRaster3 != NULL) delete pRaster3;
        if (pRaster4 != NULL) delete pRaster4;

        return false;
    }
    if (nBland2 == 4)
    {
        bNeedAlpha = true;
    }

    //图片3
    id.TileID.m_nLevel = ChildrenInfo[2].m_nLevel;
    id.TileID.m_nRow   = ChildrenInfo[2].m_nRow;
    id.TileID.m_nCol   = ChildrenInfo[2].m_nCol;
    m_pTargetDB->readBlock(id, pImage3, nLen);
    if (!readTIFStreamFromDB(pImage3, nLen, pRaster3, nBland3))
    {
        deudbProxy::freeMemory(pImage1);
        deudbProxy::freeMemory(pImage2);
        deudbProxy::freeMemory(pImage3);
        deudbProxy::freeMemory(pImage4);

        if (pRaster1 != NULL) delete pRaster1;
        if (pRaster2 != NULL) delete pRaster2;
        if (pRaster3 != NULL) delete pRaster3;
        if (pRaster4 != NULL) delete pRaster4;

        return false;
    }
    if (nBland3 == 4)
    {
        bNeedAlpha = true;
    }

    //图片4
    id.TileID.m_nLevel = ChildrenInfo[3].m_nLevel;
    id.TileID.m_nRow   = ChildrenInfo[3].m_nRow;
    id.TileID.m_nCol   = ChildrenInfo[3].m_nCol;
    m_pTargetDB->readBlock(id, pImage4, nLen);
    if (!readTIFStreamFromDB(pImage4, nLen, pRaster4, nBland4))
    {
        deudbProxy::freeMemory(pImage1);
        deudbProxy::freeMemory(pImage2);
        deudbProxy::freeMemory(pImage3);
        deudbProxy::freeMemory(pImage4);

        if (pRaster1 != NULL) delete pRaster1;
        if (pRaster2 != NULL) delete pRaster2;
        if (pRaster3 != NULL) delete pRaster3;
        if (pRaster4 != NULL) delete pRaster4;

        return false;
    }
    if (nBland4 == 4)
    {
        bNeedAlpha = true;
    }

    //合并四张图片，格式RGBARGBARGBARGBARGBARGBA
    char* pMergeRaster = NULL;
    if (bNeedAlpha)
    {
        nBland = 4;
        pMergeRaster = new char[nTileSize*nTileSize*4*4];
        memset(pMergeRaster, 0, nTileSize*nTileSize*4*4);
        for (unsigned i=0u; i<nTileSize; i++)
        {
            for (unsigned j=0u; j<nTileSize; j++)
            {
                //合并图片1
                memcpy_s(pMergeRaster+(i*(2*nTileSize)+j)*4, 3, pRaster1+(i*nTileSize+j)*nBland1, 3);
                if (nBland1 == 3)
                {
                    *(pMergeRaster+(i*(2*nTileSize)+j)*4+3) = (char)255;
                }
                else if (nBland1 == 4)
                {
                    memcpy_s(pMergeRaster+(i*(2*nTileSize)+j)*4+3, 1, pRaster1+(i*nTileSize+j)*nBland1+3, 1);
                }
                //合并图片2
                memcpy_s(pMergeRaster+nTileSize*(i+1)*4+(i*nTileSize+j)*4, 3, pRaster2+(i*nTileSize+j)*nBland2, 3);
                if (nBland2 == 3)
                {
                    *(pMergeRaster+nTileSize*(i+1)*4+(i*nTileSize+j)*4+3) = (char)255;
                }
                else if (nBland2 == 4)
                {
                    memcpy_s(pMergeRaster+nTileSize*(i+1)*4+(i*nTileSize+j)*4+3, 1, pRaster2+(i*nTileSize+j)*nBland2+3, 1);
                }
                //合并图片3
                memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*4+(i*(2*nTileSize)+j)*4, 3, pRaster3+(i*nTileSize+j)*nBland3, 3);
                if (nBland3 == 3)
                {
                    *(pMergeRaster+(nTileSize*nTileSize*2)*4+(i*(2*nTileSize)+j)*4+3) = (char)255;
                }
                else if (nBland3 == 4)
                {
                    memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*4+(i*(2*nTileSize)+j)*4+3, 1, pRaster3+(i*nTileSize+j)*nBland3+3, 1);
                }
                //合并图片4
                memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*4+nTileSize*(i+1)*4+(i*nTileSize+j)*4, 3, pRaster4+(i*nTileSize+j)*nBland4, 3);
                if (nBland4 == 3)
                {
                    *(pMergeRaster+(nTileSize*nTileSize*2)*4+nTileSize*(i+1)*4+(i*nTileSize+j)*4+3) = (char)255;
                }
                else if (nBland4 == 4)
                {
                    memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*4+nTileSize*(i+1)*4+(i*nTileSize+j)*4+3, 1, pRaster4+(i*nTileSize+j)*nBland4+3, 1);
                }
            }
        }
    }
    else
    {
        nBland = 3;
        pMergeRaster = new char[nTileSize*nTileSize*3*4];
        memset(pMergeRaster, 0, nTileSize*nTileSize*3*4);
        for (unsigned i=0; i<nTileSize; i++)
        {
            for (unsigned j=0; j<nTileSize; j++)
            {
                //合并图片1
                memcpy_s(pMergeRaster+(i*(2*nTileSize)+j)*3, 3, pRaster1+(i*nTileSize+j)*nBland1, 3);
                //合并图片2
                memcpy_s(pMergeRaster+nTileSize*(i+1)*3+(i*nTileSize+j)*3, 3, pRaster2+(i*nTileSize+j)*nBland2, 3);
                //合并图片3
                memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*3+(i*(2*nTileSize)+j)*3, 3, pRaster3+(i*nTileSize+j)*nBland3, 3);
                //合并图片4
                memcpy_s(pMergeRaster+(nTileSize*nTileSize*2)*3+nTileSize*(i+1)*3+(i*nTileSize+j)*3, 3, pRaster4+(i*nTileSize+j)*nBland4, 3);
            }
        }
    }

    //缩放
    cmm::image::PixelFormat format;
    if (nBland == 3)
    {
        format = cmm::image::PF_RGB;
    }
    else if (nBland == 4)
    {
        format = cmm::image::PF_RGBA;
    }

    cmm::image::Image img;
    img.attach((void*)pMergeRaster, nTileSize*2, nTileSize*2, format);
    OpenSP::sp<cmm::image::IDEUImage> pNewImg = img.scaleImage(nTileSize, nTileSize);

    if (!pNewImg->hasAlpha())
    {
        pNewImg->deleteAlpha();

        nBland = 3;
        format = cmm::image::PF_RGB;
    }

    writeMergeImageStreamToDB(parentID, (char*)pNewImg->data(), nTileSize, nTileSize, nBland);

    pNewImg = NULL;

    deudbProxy::freeMemory(pImage1);
    deudbProxy::freeMemory(pImage2);
    deudbProxy::freeMemory(pImage3);
    deudbProxy::freeMemory(pImage4);

    if (pRaster1 != NULL)     delete pRaster1;
    if (pRaster2 != NULL)     delete pRaster2;
    if (pRaster3 != NULL)     delete pRaster3;
    if (pRaster4 != NULL)     delete pRaster4;
    if (pMergeRaster != NULL) delete pMergeRaster;

    return true;
}

bool TilingThread::readTIFStreamFromDB(const void* pBuffer, unsigned int nLen, unsigned char*&pRaster, unsigned &nBland)
{
    if (pBuffer == NULL || nLen < 1)
    {
        return false;
    }

    std::strstream ss((char *)pBuffer, nLen);
    PixelType      nPixelType;
    ReadWriteTiFF  rtiff;
    pRaster = rtiff.readTIFStream(ss, nPixelType);

    switch (nPixelType)
    {
    case PixelType_LUMINANCE:
    case PixelType_LUMINANCE_ALPHA:
    case PixelType_ALPHA:
        nBland = 1;
        break;
    case PixelType_RGB:
        nBland = 3;
        break;
    case PixelType_RGBA:
        nBland = 4;
        break;
    }

    return true;
}

//GDAL读取文件后的光栅处理
bool TilingThread::writeImageStreamToDB(const ID &id, std::vector<char*> &pBufVec, unsigned nXsize, unsigned nYsize, unsigned nBland)
{
    //图像倒置
    char* pTmpRasterR = NULL;
    char* pTmpRasterG = NULL;
    char* pTmpRasterB = NULL;
    char* pTmpRasterA = NULL;
    switch (nBland)
    {
    case 1:
        pTmpRasterA = new char[nXsize*nYsize];

        for (unsigned nRow=0; nRow<nYsize; nRow++)
        {
            memcpy_s(pTmpRasterA+nRow*nXsize, nXsize, pBufVec[0]+(nYsize-nRow-1)*nXsize, nXsize);
        }

        break;
    case 3:
        pTmpRasterR = new char[nXsize*nYsize];
        pTmpRasterG = new char[nXsize*nYsize];
        pTmpRasterB = new char[nXsize*nYsize];

        for (unsigned nRow=0; nRow<nYsize; nRow++)
        {
            memcpy_s(pTmpRasterR+nRow*nXsize, nXsize, pBufVec[0]+(nYsize-nRow-1)*nXsize, nXsize);
            memcpy_s(pTmpRasterG+nRow*nXsize, nXsize, pBufVec[1]+(nYsize-nRow-1)*nXsize, nXsize);
            memcpy_s(pTmpRasterB+nRow*nXsize, nXsize, pBufVec[2]+(nYsize-nRow-1)*nXsize, nXsize);
        }

        break;
    case 4:
        pTmpRasterR = new char[nXsize*nYsize];
        pTmpRasterG = new char[nXsize*nYsize];
        pTmpRasterB = new char[nXsize*nYsize];
        pTmpRasterA = new char[nXsize*nYsize];

        for (unsigned nRow=0; nRow<nYsize; nRow++)
        {
            memcpy_s(pTmpRasterR+nRow*nXsize, nXsize, pBufVec[0]+(nYsize-nRow-1)*nXsize, nXsize);
            memcpy_s(pTmpRasterG+nRow*nXsize, nXsize, pBufVec[1]+(nYsize-nRow-1)*nXsize, nXsize);
            memcpy_s(pTmpRasterB+nRow*nXsize, nXsize, pBufVec[2]+(nYsize-nRow-1)*nXsize, nXsize);
            memcpy_s(pTmpRasterA+nRow*nXsize, nXsize, pBufVec[3]+(nYsize-nRow-1)*nXsize, nXsize);
        }

        break;
    }

    //将RRRRRRGGGGGGBBBBBBAAAAAA格式转换成RGBARGBARGBARGBARGBARGBA
    char* pBuf = new char[nXsize*nYsize*nBland];
    memset(pBuf, 0, nXsize*nYsize*nBland);

    PixelType nPixelType;
    switch (nBland)
    {
    case 1:
        nPixelType = PixelType_LUMINANCE_ALPHA;
        break;
    case 3:
        nPixelType = PixelType_RGB;
        break;
    case 4:
        nPixelType = PixelType_RGBA;
        break;
    }

    int nDest = 0;
    for (unsigned i=0; i<nXsize*nYsize; i++)
    {
        switch (nBland)
        {
        case 1:
            pBuf[nDest++] = pTmpRasterA[i];
            break;
        case 3:
            pBuf[nDest++] = pTmpRasterR[i];
            pBuf[nDest++] = pTmpRasterG[i];
            pBuf[nDest++] = pTmpRasterB[i];
            break;
        case 4:
            pBuf[nDest++] = pTmpRasterR[i];
            pBuf[nDest++] = pTmpRasterG[i];
            pBuf[nDest++] = pTmpRasterB[i];
            pBuf[nDest++] = pTmpRasterA[i];
            break;
        }
    }

    if (pTmpRasterR != NULL) delete pTmpRasterR;
    if (pTmpRasterG != NULL) delete pTmpRasterG;
    if (pTmpRasterB != NULL) delete pTmpRasterB;
    if (pTmpRasterA != NULL) delete pTmpRasterA;

    std::ostrstream ss;
    ReadWriteTiFF   wtiff;
    wtiff.writeTIFStream(ss, pBuf, nPixelType, GDTType_Byte, nXsize, nYsize, true);

    ss.seekp(0, std::ios::end);
    unsigned int nLen = ss.tellp();

#if 0
    /*                                                        0222222222222222222220
    22222222222222222222                                      2222222222222222222222
    21111111111111111112                                      2211111111111111111122
    21111111111111111112                扩 边                 2211111111111111111122
    21111111111111111112          =================>>         2211111111111111111122
    21111111111111111112                                      2211111111111111111122
    21111111111111111112                                      2211111111111111111122
    22222222222222222222                                      2222222222222222222222
                                                              0222222222222222222220
    */
    if ((id.TileID.m_nLevel == 6 && id.TileID.m_nRow == 18 && id.TileID.m_nCol == 33) ||
        (id.TileID.m_nLevel == 6 && id.TileID.m_nRow == 18 && id.TileID.m_nCol == 34) ||
        (id.TileID.m_nLevel == 6 && id.TileID.m_nRow == 17 && id.TileID.m_nCol == 33) ||
        (id.TileID.m_nLevel == 6 && id.TileID.m_nRow == 17 && id.TileID.m_nCol == 34))
    {
        char* pBuf258 = NULL;

        if (nBland == 3)
        {
            pBuf258 = new char[258*(258*nBland+2)];
            memset(pBuf258, 0, 258*(258*nBland+2));
        }
        else if (nBland == 4)
        {
            pBuf258 = new char[258*258*nBland];
            memset(pBuf258, 0, 258*258*nBland);
        }

        //第一行
        if (nBland == 3)
        {
            memcpy_s(pBuf258+3, 256*3, pBuf, 256*3);
        }
        else if (nBland == 4)
        {
            memcpy_s(pBuf258+4, 256*4, pBuf, 256*4);
        }

        //中间行
        for (int i=0; i<256; i++)
        {
            if (nBland == 3)
            {
                pBuf258[(i+1)*(258*3+2)]   = pBuf[i*256*3];
                pBuf258[(i+1)*(258*3+2)+1] = pBuf[i*256*3+1];
                pBuf258[(i+1)*(258*3+2)+2] = pBuf[i*256*3+2];

                pBuf258[(i+1)*(258*3+2)+257*3]   = pBuf[i*256*3+255*3];
                pBuf258[(i+1)*(258*3+2)+257*3+1] = pBuf[i*256*3+255*3+1];
                pBuf258[(i+1)*(258*3+2)+257*3+2] = pBuf[i*256*3+255*3+2];

                memcpy_s(pBuf258+(i+1)*(258*3+2)+3, 256*3, pBuf+i*256*3, 256*3);
            }
            else if (nBland == 4)
            {
                pBuf258[(i+1)*258*4]   = pBuf[i*256*4];
                pBuf258[(i+1)*258*4+1] = pBuf[i*256*4+1];
                pBuf258[(i+1)*258*4+2] = pBuf[i*256*4+2];
                pBuf258[(i+1)*258*4+3] = pBuf[i*256*4+3];

                pBuf258[(i+1)*258*4+257*4]   = pBuf[i*256*4+255*4];
                pBuf258[(i+1)*258*4+257*4+1] = pBuf[i*256*4+255*4+1];
                pBuf258[(i+1)*258*4+257*4+2] = pBuf[i*256*4+255*4+2];
                pBuf258[(i+1)*258*4+257*4+3] = pBuf[i*256*4+255*4+3];

                memcpy_s(pBuf258+(i+1)*258*4+4, 256*4, pBuf+i*256*4, 256*4);
            }
        }

        //最后一行
        if (nBland == 3)
        {
            memcpy_s(pBuf258+257*(258*3+2)+3, 256*3, pBuf+255*256*3, 256*3);
        }
        else if (nBland == 4)
        {
            memcpy_s(pBuf258+257*258*4+4, 256*4, pBuf+255*256*4, 256*4);
        }

        std::ostringstream oss;
        oss << "E:\\Picture\\Test\\";
        oss << (unsigned)id.TileID.m_nLevel << '_'
            << (unsigned)id.TileID.m_nRow << '_'
            << (unsigned)id.TileID.m_nCol << "_256"
            << ".bmp";

        cmm::image::PixelFormat format = cmm::image::PF_RGBA;
        if (nBland == 3)
        {
            format = cmm::image::PF_RGB;
        }

        cmm::image::Image img;
        img.attach((void*)pBuf, 256, 256, format);
        img.saveToFile(oss.str());

        std::ostringstream oss2;
        oss2 << "E:\\Picture\\Test\\";
        oss2 << (unsigned)id.TileID.m_nLevel << '_'
             << (unsigned)id.TileID.m_nRow << '_'
             << (unsigned)id.TileID.m_nCol << "_258"
             << ".bmp";

        cmm::image::Image img2;
        img2.attach((void*)pBuf258, 258, 258, format);
        img2.saveToFile(oss2.str());
    }

    FILE* pFile = NULL;
    fopen_s(&pFile, "E:\\Picture\\Test\\11.tif", "wb");
    fwrite(ss.str(), nLen, 1, pFile);
    fclose(pFile);
#endif

    //*/输出调试用图片
    /*
    std::ostringstream oss;
    oss << "E:\\Picture\\"
        << (unsigned)id.TileID.m_nLevel << "\\"
        << (unsigned)id.TileID.m_nLevel << '_'
        << (unsigned)id.TileID.m_nRow << '_'
        << (unsigned)id.TileID.m_nCol << "_256"
        << ".tif";
    std::string oss1 = oss.str();
    FILE* pFile = NULL;
    fopen_s(&pFile, oss1.c_str(), "wb");
    fwrite(ss.str(), nLen, 1, pFile);
    fclose(pFile);
    */

    m_pTargetDB->addBlock(id, ss.str(), nLen);
    ss.freeze(false);

    m_pTaskQueue->addTopLevelID(id);

    if (pBuf != NULL) delete pBuf;

    return true;
}

//TIFFLib读取后的光栅数据处理
bool TilingThread::writeMergeImageStreamToDB(const ID &id, char* pBuf, unsigned nXsize, unsigned nYsize, unsigned nBland)
{
    if (m_pTaskQueue == NULL || m_pTargetDB == NULL)
    {
        return false;
    }

    PixelType nPixelType;
    switch (nBland)
    {
    case 1:
        nPixelType = PixelType_LUMINANCE_ALPHA;
        break;
    case 3:
        nPixelType = PixelType_RGB;
        break;
    case 4:
        nPixelType = PixelType_RGBA;
        break;
    }

    std::ostrstream ss;
    ReadWriteTiFF   wtiff;
    wtiff.writeTIFStream(ss, pBuf, nPixelType, GDTType_Byte, nXsize, nYsize, true);

    ss.seekp(0, std::ios::end);
    unsigned int nLen = ss.tellp();

#if 0
    //if (id.TileID.m_nLevel == 12 || id.TileID.m_nLevel == 8)
    {
        std::ostringstream oss;
        oss << "E:\\Picture\\Test\\";
        oss << (unsigned)id.TileID.m_nLevel << '_'
            << (unsigned)id.TileID.m_nRow << '_'
            << (unsigned)id.TileID.m_nCol
            << ".bmp";

        cmm::image::PixelFormat format = cmm::image::PF_RGBA;
        if (nBland == 3)
        {
            format = cmm::image::PF_RGB;
        }

        cmm::image::Image img;
        img.attach((void*)pBuf, nXsize, nYsize, format);
        img.saveToFile(oss.str());
    }
#endif

    /*
    std::ostringstream oss;
    oss << "E:\\Picture\\"
        << (unsigned)id.TileID.m_nLevel << "\\"
        << (unsigned)id.TileID.m_nLevel << '_'
        << (unsigned)id.TileID.m_nRow << '_'
        << (unsigned)id.TileID.m_nCol << "_256"
        << ".tif";
    std::string oss1 = oss.str();
    FILE* pFile = NULL;
    fopen_s(&pFile, oss1.c_str(), "wb");
    fwrite(ss.str(), nLen, 1, pFile);
    fclose(pFile);
    */

    m_pTargetDB->addBlock(id, ss.str(), nLen);
    ss.freeze(false);

    m_pTaskQueue->addTopLevelID(id);

    return true;
}


void OutImage(const float *pImageData, int nSizeX, int nSizeY, const ID &id)
{
    std::ostringstream oss;
    oss << "e:\\outTile\\"
        << "L_" << (unsigned)id.TileID.m_nLevel
        << "_R_" << (unsigned)id.TileID.m_nRow
        << "_C_" << (unsigned)id.TileID.m_nCol
        << ".txt";
    const std::string strFilePath = oss.str();

    std::ofstream file(strFilePath);

    for(int y = 0; y < nSizeY; y++)
    {
        const float *pLineData = pImageData + y * nSizeX;
        for(int x = 0; x < nSizeX; x++)
        {
            const float *pData = pLineData + x;
            file << *pData << '\t';
        }
        file << '\n';
    }
    file.close();
}


//GDAL读取文件后的光栅处理
bool TilingThread::writeHeightFieldStreamToDB(const ID &id, float* pBuf, unsigned nXsize, unsigned nYsize)
{
    if (m_pTaskQueue == NULL || m_pTargetDB == NULL)
    {
        return false;
    }

    //图像倒置
    float* pTmpRaster = new float[(nXsize+BYSIDE)*(nYsize+BYSIDE)];
    memset(pTmpRaster, 0, (nXsize+BYSIDE)*(nYsize+BYSIDE)*sizeof(float));
    for (unsigned nRow=0; nRow<nYsize+BYSIDE; nRow++)
    {
        memcpy_s(pTmpRaster+nRow*(nXsize+BYSIDE), (nXsize+BYSIDE)*sizeof(float),
            pBuf+(nYsize+BYSIDE-nRow-1)*(nXsize+BYSIDE), (nXsize+BYSIDE)*sizeof(float));
    }

    std::ostrstream ss;
    ReadWriteTiFF   wtiff;
    wtiff.writeTIFStream(ss, (char*)pTmpRaster, PixelType_LUMINANCE, GDTType_Float32, nXsize+BYSIDE, nYsize+BYSIDE, false);

    ss.seekp(0, std::ios::end);
    unsigned int nLen = ss.tellp();

#if 0
    {
        std::ostringstream oss1;
        oss1 << "E:\\Picture\\Test\\";
        oss1 << (unsigned)id.TileID.m_nLevel << '_'
        << (unsigned)id.TileID.m_nRow << '_'
        << (unsigned)id.TileID.m_nCol
        << ".bmp";

        cmm::image::PixelFormat format = format = cmm::image::PF_LUMINANCE;;
        cmm::image::Image img;

        float *pActRaster = pTmpRaster;
        float fltMin = -200.0f, fltMax = 1000.0f;
        //for(unsigned y = 0; y < nYsize + BYSIDE; y++)
        //{
        //    for(unsigned x = 0; x < nXsize + BYSIDE; x++)
        //    {
        //        if(fltMin > *pActRaster)
        //        {
        //            fltMin = *pActRaster;
        //        }
        //        if(fltMax < *pActRaster)
        //        {
        //            fltMax = *pActRaster;
        //        }

        //        ++pActRaster;
        //    }
        //}
        float fltDelta = fltMax - fltMin;
        if(fltDelta < FLT_EPSILON)
        {
            fltDelta = 1.0f;
        }

        const unsigned nImgSize = (nXsize + BYSIDE) * (nYsize + BYSIDE);
        unsigned char *pPixel = new unsigned char[nImgSize * 4];
        unsigned char *pActPixel = pPixel;
        pActRaster = pTmpRaster;
        for(unsigned y = 0; y < nYsize + BYSIDE; y++)
        {
            for(unsigned x = 0; x < nXsize + BYSIDE; x++)
            {
                float fltValue = (*pActRaster - fltMin) / fltDelta;
                fltValue *= 255;//0x00FFFFFF;
                pActPixel[0] = (unsigned)(fltValue + 0.5f);
                pActPixel[1] = (unsigned)(fltValue + 0.5f);
                pActPixel[2] = (unsigned)(fltValue + 0.5f);
                pActPixel[3] = 255;
                ++pActRaster;
                pActPixel += 4;
            }
        }


        img.attach((void*)pPixel, nXsize+BYSIDE, nYsize+BYSIDE, cmm::image::PF_RGBA);
        img.saveToFile(oss1.str());
        delete[] pPixel;
    }
#endif

    m_pTargetDB->addBlock(id, ss.str(), nLen);
    ss.freeze(false);

    m_pTaskQueue->addTopLevelID(id);

//    OutImage(pTmpRaster, nXsize + BYSIDE, nYsize + BYSIDE, id);

    if (pTmpRaster != NULL) delete pTmpRaster;

    return true;
}

//TIFFLib读取后的光栅数据处理
bool TilingThread::writeMergeHeightFieldStreamToDB(const ID &id, float* pBuf, unsigned nXsize, unsigned nYsize)
{
    return true;
}


void TilingThread::mergeInvalidColor(const int nBandCount, const unsigned int nTileSize, std::vector<char*> &RasterDataVec)
{
    if (nBandCount != 4)
    {
        return;
    }

    std::vector<INVALIDCOLOR> vecInvalidColor = m_pTaskQueue->getInvalidColor();

    for (unsigned int i = 0; i < nTileSize*nTileSize; i++)
    {
        for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
        {
            if (*(RasterDataVec[0]+i) == vecInvalidColor[j].r && *(RasterDataVec[1]+i) == vecInvalidColor[j].g && *(RasterDataVec[2]+i) == vecInvalidColor[j].b)
            {
                *(RasterDataVec[3]+i) = (char)0;
                break;
            }
        }
    }
}
