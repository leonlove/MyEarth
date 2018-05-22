#include "ReciverManager.h"

#include <Common/Actions.h>
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventObject.h>
#include <EventAdapter/IEventFilter.h>

ReciverManager::ReciverManager(void)
{
    m_pHidenController      = new HidenController;
    //m_pHighlightController   = new HighlightController;
}


ReciverManager::~ReciverManager(void)
{
}


void ReciverManager::setRootNode(osg::Node *pNode)
{
    m_pRootNode = pNode;
    m_pHidenController->setRootNode(m_pRootNode);
    //m_pHighlightController->setRootNode(m_pRootNode);
}

void ReciverManager::onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject)
{
    std::string strAction = pEventObject->getAction();
    if(strAction == cmm::ACTION_HIDEN_INSTANCES)
    {
        cmm::variant_data var;
        pEventObject->getExtra("Hiden", var);
        bool bHiden = var;

        cmm::variant_data varInsts;
        pEventObject->getExtra("Instances", varInsts);
        std::vector<ID> vecInstance = varInsts;

        m_pHidenController->hiden(vecInstance, bHiden);
    }
    else if(strAction == cmm::ACTION_HIGHLIGHT_INSTANCES)
    {
    }
}