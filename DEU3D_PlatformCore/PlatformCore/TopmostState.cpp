#include "TopmostState.h"


TopmostState::TopmostState(const std::string &strName) : StateBase(strName)
{
}


TopmostState::~TopmostState(void)
{
}

bool TopmostState::applyState(osg::Node *pNode, bool bApply)
{
    //对应参数化做修改
    if(bApply)
    {
        pNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    }
    else
    {
        pNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    }

    return true;
}