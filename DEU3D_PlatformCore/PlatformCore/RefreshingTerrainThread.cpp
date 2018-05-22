#include "RefreshingTerrainThread.h"
#include <assert.h>
#include <osgUtil/IncrementalCompileOperation>
#include <osg/SharedObjectPool>
#include <algorithm>

#include "ReplaceChildren_Operation.h"
#include "FindBottomTerrainTile_Operation.h"

RefreshingTerrainThread::RefreshingTerrainThread(void)
{
}


RefreshingTerrainThread::~RefreshingTerrainThread(void)
{
}


void RefreshingTerrainThread::initialize(osgViewer::ViewerBase *pViewer, osg::Node *pTerrainNode, const FileReadInterceptor *pTileReader, SceneGraphOperator *pSceneGraphOperator)
{
    m_pViewer      = pViewer;
    m_pTerrainNode = pTerrainNode;
    m_pTileReader  = pTileReader;
    m_pSceneGraphOperator = pSceneGraphOperator;
}


void RefreshingTerrainThread::doRefresh(void)
{
    if(isRunning())
    {
        dropWork();
        join();
    }

    setSchedulePriority(THREAD_PRIORITY_HIGH);
    startThread();
}


void RefreshingTerrainThread::dropWork(void)
{
    m_bDropped.exchange(1u);
}


void RefreshingTerrainThread::run(void)
{
    if(!m_pTerrainNode.valid())         return;
    if(!m_pTileReader.valid())          return;
    if(!m_pSceneGraphOperator.valid())  return;

    m_bDropped.exchange(0u);

    osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
    pPool->clearObjectByDataset(2u);

    OpenSP::sp<FindBottomTerrainTile_Operation> pBottomTileFinder = new FindBottomTerrainTile_Operation;
    m_pSceneGraphOperator->pushOperation(pBottomTileFinder.get());
    pBottomTileFinder->waitResult();

    FindBottomTerrainTile_Operation::TerrainTileMap &mapObsoleteTerrainTiles = pBottomTileFinder->getTerrainTiles();
    if(mapObsoleteTerrainTiles.empty())
    {
        return;
    }

    const unsigned nMode = osgUtil::GLObjectsVisitor::COMPILE_DISPLAY_LISTS | osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES | osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
    osg::ref_ptr<osgUtil::StateToCompile>   pState2Compile = new osgUtil::StateToCompile(nMode);

    const TerrainModificationManager *pTerrainModificationManager = dynamic_cast<const TerrainModificationManager *>(m_pTileReader->getTerrainModificationManager());

    FindBottomTerrainTile_Operation::TerrainTileMap::reverse_iterator riotr = mapObsoleteTerrainTiles.rbegin();
    for( ; riotr != mapObsoleteTerrainTiles.rend() && (unsigned)m_bDropped == 0u; ++riotr)
    {
        osg::ref_ptr<ReplaceChildren_Operation> pReplaceOperation = new ReplaceChildren_Operation;
        FindBottomTerrainTile_Operation::TerrainTileList &listTerrainTiles = riotr->second;
        FindBottomTerrainTile_Operation::TerrainTileList::iterator itor = listTerrainTiles.begin();
        for(itor = riotr->second.begin(); itor != riotr->second.end() && (unsigned)m_bDropped == 0u; ++itor)
        {
            osgTerrain::TerrainTile *pTile = (*itor).get();

            const ID &id = pTile->getID();

            osgDB::ReaderWriter::ReadResult rr = m_pTileReader->readSimpleTileByID(id, NULL);
            if(!rr.success())   continue;

            osg::ref_ptr<osgTerrain::TerrainTile> pNewTile = dynamic_cast<osgTerrain::TerrainTile *>(rr.getNode());
            if(!pNewTile.valid())       continue;

            pTerrainModificationManager->modifyTerrainTile(pNewTile.get());
            pNewTile->dirtyBound();
            pNewTile->accept(*pState2Compile);
            pReplaceOperation->addReplacePair(pTile, pNewTile);

            //Á¢¼´ÊÍ·Å
            *itor = NULL;
        }

        m_pSceneGraphOperator->pushOperation(pReplaceOperation.get());
        riotr->second.clear();
    }
}

