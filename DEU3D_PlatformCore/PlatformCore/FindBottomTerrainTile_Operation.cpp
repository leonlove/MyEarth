#include "FindBottomTerrainTile_Operation.h"
#include "SceneGraphOperator.h"

FindBottomTerrainTile_Operation::FindBottomTerrainTile_Operation(void)
{
    m_pFinder = new BottomTerrainTileFinder;
    m_block.reset();
}


bool FindBottomTerrainTile_Operation::doAction(SceneGraphOperator *pOperator)
{
    osg::ref_ptr<osg::Node> pTerrainNode = getTerrainRootNode(pOperator);
    pTerrainNode->accept(*m_pFinder);
    m_block.release();
    return true;
}


