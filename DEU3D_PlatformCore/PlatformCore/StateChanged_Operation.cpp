#include "StateChanged_Operation.h"
#include <common/StateDefiner.h>
#include <osgUtil/FindNodeVisitor.h>
#include "SmartLOD.h"
#include "VirtualCubeNode.h"
#include <IDProvider/Definer.h>
#include <set>
#include "StateBase.h"


StateChanged_Operation::StateChanged_Operation(void) : m_bState(false)
{
}


StateChanged_Operation::~StateChanged_Operation(void)
{
}


void StateChanged_Operation::setState(StateManager *pStateManager, const std::string &strState, bool bState)
{
    m_strState = strState;
    m_bState = bState;
    m_pStateManager = pStateManager;
}


void StateChanged_Operation::setChangedIDs(const IDList &listIDs)
{
    m_listIDs.insert(listIDs.begin(), listIDs.end());
}


bool StateChanged_Operation::doAction(SceneGraphOperator *pOperator)
{
    osg::Node *pCultureRootNode = getCultureRootNode(pOperator);
    if(m_strState == cmm::STATE_VISIBLE)
    {
        applyVisibleState(pCultureRootNode);
    }
    else
    {
        applyNonVisibleState(pCultureRootNode);
    }

    return true;
}


void StateChanged_Operation::applyVisibleState(osg::Node *pCultureRootNode)
{
    osg::ref_ptr<osgUtil::FindNodesByIDTypeVisitor>  pFinder = new osgUtil::FindNodesByIDTypeVisitor(V_CUBE_FRAGMENT_NODE_ID);
    pCultureRootNode->accept(*pFinder);

    const osgUtil::FindNodesByIDTypeVisitor::NodeList &listNodes = pFinder->getTargetNodes();
    osgUtil::FindNodesByIDListVisitor::NodeList::const_iterator itorNode = listNodes.begin();
    for( ; itorNode != listNodes.end(); ++itorNode)
    {
        VCubeFragmentNode *pVCubeFragmentNode = dynamic_cast<VCubeFragmentNode *>((*itorNode).get());
        if(!pVCubeFragmentNode)    continue;

        IDList listIDs;
        pVCubeFragmentNode->getObjects(listIDs);
        if(listIDs.empty()) continue;

        std::set<ID> listIDs2;
        for(IDList::const_iterator itor = listIDs.begin(); itor != listIDs.end(); ++itor)
        {
            const ID &id = *itor;
            std::set<ID>::const_iterator itorFind = m_listIDs.find(id);
            if(itorFind != m_listIDs.end())
            {
                listIDs2.insert(id);
                m_listIDs.erase(itorFind);
                if(m_listIDs.empty())   break;
            }
        }
        if(listIDs2.empty())    continue;

        pVCubeFragmentNode->setVisible(listIDs2, m_bState);
        if(m_listIDs.empty())   break;
    }
}


void StateChanged_Operation::applyNonVisibleState(osg::Node *pCultureRootNode)
{
    StateBase *pState = dynamic_cast<StateBase *>(m_pStateManager->getRenderState(m_strState));
    if(!pState) return;

    osg::ref_ptr<osgUtil::FindNodesByIDListVisitor> pFinder = new osgUtil::FindNodesByIDListVisitor(m_listIDs);
    pCultureRootNode->accept(*pFinder);

    const osgUtil::FindNodesByIDListVisitor::NodeList &listNodes = pFinder->getTargetNodes();
    osgUtil::FindNodesByIDListVisitor::NodeList::const_iterator itorNode = listNodes.begin();

    for( ; itorNode != listNodes.end(); ++itorNode)
    {
        osg::Node *pNode = const_cast<osg::Node *>((*itorNode).get());
        pState->applyState(pNode, m_bState);
    }
}

