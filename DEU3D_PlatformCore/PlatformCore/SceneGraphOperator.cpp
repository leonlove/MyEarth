#include "SceneGraphOperator.h"
#include <osgUtil/FindNodeVisitor.h>

SceneGraphOperator::SceneGraphOperator(void)
{
    m_pTerrainRootNode = NULL;
    m_pCultureRootNode = NULL;
    m_pTempElementRootNode = NULL;
}


SceneGraphOperator::~SceneGraphOperator(void)
{
}


void SceneGraphOperator::pushOperation(SceneGraphOperationBase *pOperation)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxOperations);
    m_queueOperations.push_back(pOperation);
}


void SceneGraphOperator::feedbackOperation(SceneGraphOperationBase *pOperation)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxOperations);
    m_queueOperations.push_front(pOperation);
}


SceneGraphOperationBase *SceneGraphOperator::takeOperation(void)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxOperations);
    if(m_queueOperations.empty())
    {
        return NULL;
    }
    osg::ref_ptr<SceneGraphOperationBase> pOperation = m_queueOperations.front();
    m_queueOperations.pop_front();
    return pOperation.release();
}


bool SceneGraphOperator::initialize(osg::Node *pSceneRootNode)
{
    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder1 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
    pSceneRootNode->accept(*pFinder1);
    m_pTempElementRootNode = pFinder1->getTargetNode();

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder2 = new osgUtil::FindNodeByNameVisitor("CultureRootNode");
    pSceneRootNode->accept(*pFinder2);
    m_pCultureRootNode = pFinder2->getTargetNode();

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder3 = new osgUtil::FindNodeByNameVisitor("TerrainRootNode");
    pSceneRootNode->accept(*pFinder3);
    m_pTerrainRootNode = pFinder3->getTargetNode();

    return (m_pTempElementRootNode && m_pCultureRootNode && m_pTerrainRootNode);
}


void SceneGraphOperator::operator()(osg::Node *node, osg::NodeVisitor *pNodeVisitor)
{
    if (node->getNumChildrenRequiringUpdateTraversal()>0)
    {
        traverse(node, pNodeVisitor);
    }

    if(pNodeVisitor->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR)
    {
        return;
    }

    osg::ref_ptr<SceneGraphOperationBase> pOperation = takeOperation();
    if(!pOperation.valid())
    {
        return;
    }

    pOperation->doAction(this);
}

