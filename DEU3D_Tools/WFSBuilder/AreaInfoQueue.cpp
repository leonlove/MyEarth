#include "AreaInfoQueue.h"
#include "EventAdapter/IEventFilter.h"
#include "EventAdapter/IEventObject.h"
#include "WFSBuilder.h"

namespace wfsb
{

AreaInfoQueue::AreaInfoQueue(void)
{
}


AreaInfoQueue::~AreaInfoQueue(void)
{
}

bool AreaInfoQueue::initialize(ea::IEventAdapter* pIEventAdapter,
    IWFSBuilder *pWFSBuilder)
{
    m_pWFSBuilder = pWFSBuilder;
    m_pEventAdapter = pIEventAdapter;

    m_pBBoxReceiver = new AreaReceiver(this);
    OpenSP::sp<ea::IEventFilter> pEventFilter = ea::createEventFilter();
    pEventFilter->addAction(ea::ACTION_BBOX_INFO);
    m_pEventAdapter->registerReceiver(m_pBBoxReceiver.get(),pEventFilter.get());

    m_pBuilderThread = new BuilderThread(m_pWFSBuilder,this);
    m_pBuilderThread->startThread();

    m_bInitialized = true;
    return true;

}

void AreaInfoQueue::unInitialize()
{
    if(!m_bInitialized) return;

    if(m_pBuilderThread.valid())
    {
        m_pBuilderThread->finishMission();
        m_pBuilderThread = NULL;
    }

    m_pEventAdapter->unregisterReceiver(m_pBBoxReceiver.get());
    m_bInitialized = false;
}

bool AreaInfoQueue::getBBoxInfo(AreaInfo& bInfo)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxBBoxQueue);
    if (m_listBBox.empty())
    {
        return false;
    }

    bInfo = m_listBBox.front();
    m_listBBox.pop_front();
    return true;
}

bool AreaInfoQueue::addBBoxInfo(AreaInfo  bInfo)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scopeLock(m_mtxBBoxQueue);
    m_listBBox.push_back(bInfo);
    m_pBuilderThread->suspend(false);
    return true;
}

void AreaInfoQueue::AreaReceiver::onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject)
{
    if(pEventAdapter == NULL || pEventObject == NULL || !m_pBBoxQueue.valid())
    {
        return;
    }

    const std::string &strAction = pEventObject->getAction();

    if(strAction.compare("action_bbox_info") == 0)
    {
        cmm::variant_data var_xmin;
        pEventObject->getExtra("xmin", var_xmin);

        cmm::variant_data var_ymin;
        pEventObject->getExtra("ymin", var_ymin);

        cmm::variant_data var_xmax;
        pEventObject->getExtra("xmax", var_xmax);

        cmm::variant_data var_ymax;
        pEventObject->getExtra("ymax", var_ymax);

        AreaInfo bInfo;
        bInfo.dxmin = var_xmin;
        bInfo.dymin = var_ymin;
        bInfo.dxmax = var_xmax;
        bInfo.dymax = var_ymax;

        m_pBBoxQueue->addBBoxInfo(bInfo);
    }

    return;
}

void AreaInfoQueue::BuilderThread::run()
{
    while((unsigned)m_MissionFinished == 0u)
    {
        // 获取BBox信息
        AreaInfo bInfo;
        if(!m_pBBoxQueue->getBBoxInfo(bInfo))
        {
            suspend(true);
            m_block.block();
            continue;
        }
        // 下载数据
        m_pWFSBuilder->downloadFeature(bInfo);
    }
}

}
