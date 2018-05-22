#include "HighlightController.h"

#include "DEUProxyNode.h"

void HighlightController::HighlightThread::run(void)
{
    m_bStop.exchange();
    highlight(m_pRootNode);
}

void HighlightController::HighlightThread::highlight(osg::Node *pNode)
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
            if(m_idList.count(pDEUProxyNode->getFileID(i)))
            {
                pDEUProxyNode->hightlight(i, true);
            }
            else
            {
                pDEUProxyNode->hightlight(i, false);
            }
        }

        return;
    }

    for(unsigned int i = 0; i < pGroup->getNumChildren(); i++)
    {
        highlight(pGroup->getChild(i));
    }
}

HighlightController::HighlightController(void) :
    m_pRootNode(NULL)
{
}

HighlightController::~HighlightController(void)
{
}

void HighlightController::highlight(const IDList &vecHighlightList)
{
    std::set<ID> idList(vecHighlightList.begin(), vecHighlightList.end());
    m_pHidenThread = new HighlightThread(m_pRootNode, idList);

    m_pHidenThread->startThread();
}