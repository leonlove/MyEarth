#include "ViewState.h"


#include <osgUtil/CullVisitor>
#include <osgViewer/View>

ViewState::OutofViewCallBack::OutofViewCallBack(const unsigned int nViews)
{
    m_nViews = nViews;
}


ViewState::OutofViewCallBack::~OutofViewCallBack(void)
{
}

void ViewState::OutofViewCallBack::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    if(!pCullVisitor)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osg::RenderInfo &info = pCullVisitor->getRenderInfo();
    osgViewer::View *pView = dynamic_cast<osgViewer::View *>(info.getView());
    unsigned int nIndex = atoi(pView->getName().c_str());
    if((m_nViews & nIndex) != 0)
    {
        traverse(pNode, pNodeVisitor);
    }
    return;
}

ViewState::ViewState(const std::string &strName) : StateBase(strName)
{
}


ViewState::~ViewState(void)
{
}

bool ViewState::applyState(osg::Node *pNode, bool bApply)
{
    if(pNode == NULL && !m_pCallBack.valid())
    {
        return false;
    }

    if(bApply)
    {
        pNode->addCullCallback(m_pCallBack);
    }
    else
    {
        pNode->removeCullCallback(m_pCallBack);
    }

    return true;
}