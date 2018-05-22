#include "HidenController.h"

#include "DEUProxyNode.h"


void HidenController::HidenThread::run(void)
{
    m_bStop.exchange();
    hiden(m_pRootNode);
}

void HidenController::HidenThread::hiden(osg::Node *pNode)
{
    if((unsigned)m_bStop > 0)
    {
        return;
    }

    osg::Group *pGroup = pNode->asGroup();
    if(NULL == pGroup)
    {
        return;
    }

    DEUProxyNode *pDEUProxyNode = dynamic_cast<DEUProxyNode *>(pNode);
    if(NULL != pDEUProxyNode)
    {
        unsigned int nNum = pDEUProxyNode->getNumFileNames();

        for(unsigned int i = 0; i < nNum; i++)
        {
            if(m_idList.count(pDEUProxyNode->getFileID(i)) > 0u)
            {
                pDEUProxyNode->setVisible(i, false);
            }
        }

        return;
    }

    for(unsigned int i = 0; i < pGroup->getNumChildren(); i++)
    {
        hiden(pGroup->getChild(i));
    }
}

HidenController::HidenController(void) :
    m_pRootNode(NULL),
    m_pHidenThread(NULL)
{
}


HidenController::~HidenController(void)
{
    destroyHidenThread();
}

void HidenController::destroyHidenThread()
{
    if(m_pHidenThread != NULL)
    {
        if(m_pHidenThread->isRunning())
        {
            m_pHidenThread->stop();
            m_pHidenThread->join();
        }
        delete m_pHidenThread;
        m_pHidenThread = NULL;
    }
}

void HidenController::hiden(const std::vector<ID> &vecHidenList, bool bHiden)
{
    destroyHidenThread();

    std::set<ID> idList(vecHidenList.begin(), vecHidenList.end());
    m_pHidenThread = new HidenThread(m_pRootNode, idList, bHiden);
    m_pHidenThread->startThread();
}