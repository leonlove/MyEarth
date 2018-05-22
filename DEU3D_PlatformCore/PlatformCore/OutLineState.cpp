#include "OutLineState.h"

#include <assert.h>

OutLineState::OutLineState(const std::string &strName) : StateBase(strName)
{
    m_pOutLineLayouter = new OutlineLayouter;
}

OutLineState::~OutLineState(void)
{
    m_pOutLineLayouter = NULL;
}

double OutLineState::getLineWidth()
{
    assert(m_pOutLineLayouter != NULL);
    return 0.0;//m_pOutLineLayouter->getWidth();
}

void OutLineState::setLineWidth(double nLineWidth)
{
    assert(m_pOutLineLayouter != NULL);
    m_pOutLineLayouter->setWidth(nLineWidth);
}

const cmm::FloatColor OutLineState::getColor() const
{
    cmm::FloatColor clr;
    clr.m_fltR = m_color[0];
    clr.m_fltG = m_color[1];
    clr.m_fltB = m_color[2];
    clr.m_fltA = m_color[3];

    return clr;
}

void OutLineState::setColor(const cmm::FloatColor &clr)
{
    m_color.set(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);

    assert(m_pOutLineLayouter != NULL);
    m_pOutLineLayouter->setColor(m_color);
}

bool OutLineState::applyState(osg::Node *pNode, bool bApply)
{
    assert(m_pOutLineLayouter != NULL);
    if(pNode == NULL)
    {
        return false;
    }

    if(bApply)
    {
        pNode->addCullCallback(m_pOutLineLayouter);
    }
    else
    {
        pNode->removeCullCallback(m_pOutLineLayouter);
    }

    return true;
}