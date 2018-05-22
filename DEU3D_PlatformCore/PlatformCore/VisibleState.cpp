#include "VisibleState.h"


VisibleState::VisibleState(const std::string &strName) : StateBase(strName)
{
}


VisibleState::~VisibleState(void)
{
}

bool VisibleState::applyState(osg::Node *pNode, bool bApply)
{
    pNode->setNodeMask(bApply ? 0x00000000 : 0xFFFFFFFF);
    return true;
}