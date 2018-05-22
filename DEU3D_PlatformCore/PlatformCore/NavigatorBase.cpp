#include "NavigatorBase.h"
#include "common/Common.h"


NavigatorBase::NavigatorBase(void)
{
}


NavigatorBase::~NavigatorBase(void)
{
}


bool NavigatorBase::initialize(NavigationParam *pNavParam)
{
    m_pNavigationParam = pNavParam;

    calculateHomePose();
    return true;
}


bool NavigatorBase::handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}


