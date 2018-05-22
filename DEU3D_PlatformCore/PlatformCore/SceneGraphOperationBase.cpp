#include "SceneGraphOperationBase.h"
#include "SceneGraphOperator.h"

osg::Node *SceneGraphOperationBase::getTerrainRootNode(SceneGraphOperator *pOperator)
{
    return pOperator->m_pTerrainRootNode;
}


osg::Node *SceneGraphOperationBase::getTempElementRootNode(SceneGraphOperator *pOperator)
{
    return pOperator->m_pTempElementRootNode;
}


osg::Node *SceneGraphOperationBase::getCultureRootNode(SceneGraphOperator *pOperator)
{
    return pOperator->m_pCultureRootNode;
}


