// NavigatorManager.cpp : 定义 DLL 应用程序的导出函数。
//

#include "NavigatorManager.h"
#include "PlanetNavigator.h"
#include <iostream>

bool MatrixDifferent(const osg::Matrixd &mtx1, const osg::Matrixd &mtx2)
{
    double dbl = 0.0;
    for(unsigned i = 0u; i < 4u; i++)
    {
        for(unsigned j = 0u; j < 4u; j++)
        {
            dbl += fabs(mtx1(i, j) - mtx2(i, j));
        }
    }
    return (dbl > 0.0001);
}


void NavigatorManager::EventThread::postEvent(const std::string &strEvent)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxQueueEvents);
    m_queueEvents.push(strEvent);
    while(m_queueEvents.size() > m_nMaxQueueLength)
    {
        m_queueEvents.pop();
    }
}


void NavigatorManager::EventThread::run(void)
{
    if(!m_pEventAdapter.valid())    return;

    OpenThreads::Block  block;
    while((unsigned)m_MissionFinished == 0u)
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxQueueEvents);
            if(!m_queueEvents.empty())
            {
                const std::string str = m_queueEvents.front();
                m_queueEvents.pop();

                OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
                pEventObj->setAction(ea::ACTION_CAMERA_POSE_CHANGED);
                pEventObj->putExtra("CameraPoseChanged", str);
                m_pEventAdapter->sendBroadcast(pEventObj.get());
            }
        }
        block.block(100);
    }
}


NavigatorManager::NavigatorManager(ea::IEventAdapter *pEventAdapter/* = NULL*/)
    : m_pEventAdapter(pEventAdapter),
      m_bInitialized(false),
      m_bHasEventOnPoseChanged(false),
      m_pEventThread(NULL)
{
}


NavigatorManager::NavigatorManager(const NavigatorManager &navi)
    : osgGA::CameraManipulator(navi),
      m_bInitialized(false),
      m_bHasEventOnPoseChanged(false),
      m_pEventThread(NULL)
{
}


NavigatorManager::~NavigatorManager(void)
{
    if(m_pEventThread)
    {
        m_pEventThread->finishMission();
        delete m_pEventThread;
        m_pEventThread = NULL;
    }
}


bool NavigatorManager::initialize(void)
{
    if(m_bInitialized)  return true;

    m_pNavigationParam = new NavigationParam;

    NavigatorBase *pNavigator = NULL;

    // 1. Create PlanetNavigator
    pNavigator = new PlanetNavigator;
    pNavigator->initialize(m_pNavigationParam.get());
    m_mapAllNavigators["PlanetNavigator"] = pNavigator;

    m_strActiveNavigator = "PlanetNavigator";

    m_bInitialized = true;
    return true;
}


bool NavigatorManager::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    if(!m_bInitialized) return false;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return false;
    }

    NavigatorBase *pNavigator = itorFind->second.get();
    return pNavigator->handleEvent(ea, aa);
}


void NavigatorManager::setByMatrix(const osg::Matrixd& matrix)
{
    return;
}


void NavigatorManager::setByInverseMatrix(const osg::Matrixd& matrix)
{
    return;
}


osg::Matrixd NavigatorManager::getMatrix(void) const
{
    return osg::Matrixd::inverse(getInverseMatrix());
}


osg::Matrixd NavigatorManager::getInverseMatrix(void) const
{
    if(!m_bInitialized) return osg::Matrixd::identity();

    std::map<std::string, OpenSP::sp<NavigatorBase> >::const_iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return osg::Matrixd::identity();
    }

    NavigatorBase *pNavigator = itorFind->second;
    const osg::Matrixd mtx = pNavigator->getCameraMatrix();

    if(m_bHasEventOnPoseChanged && m_pEventAdapter.valid())
    {
        static osg::Matrixd  mtxLast;
        if(MatrixDifferent(mtxLast, mtx))
        {
            mtxLast = mtx;

            static std::string strData = "Navigated";
            m_pEventThread->postEvent(strData);
        }
    }
    return mtx;
}


void NavigatorManager::setNaviNode(osg::Node *pTargetNode)
{
    if(!m_bInitialized) return;
    std::map<std::string, OpenSP::sp<NavigatorBase> >::const_iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return;
    }

    NavigatorBase *pNavigator = itorFind->second;
    return pNavigator->setNaviNode(pTargetNode);
}


void NavigatorManager::resetCamera(void)
{
    if(!m_bInitialized) return;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return;
    }

    NavigatorBase *pNavigator = itorFind->second;
    pNavigator->resetCamera();
}


void NavigatorManager::stopInertia(void)
{
    if(!m_bInitialized) return;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return;
    }

    NavigatorBase *pNavigator = itorFind->second;
    pNavigator->stopInertia();
}


void NavigatorManager::getCameraPose(CameraPose &pose) const
{
    if(!m_bInitialized) return;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::const_iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return;
    }

    const NavigatorBase *pNavigator = itorFind->second;
    pNavigator->getCameraPose(pose);
}


void NavigatorManager::setCameraPose(const CameraPose &pose)
{
    if(!m_bInitialized) return;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itorFind = m_mapAllNavigators.find(m_strActiveNavigator);
    if(itorFind == m_mapAllNavigators.end())
    {
        return;
    }

    NavigatorBase *pNavigator = itorFind->second;
    pNavigator->setCameraPose(pose);
}


unsigned NavigatorManager::getNavigators(std::vector<std::string> &vecNavigatorNames) const
{
    vecNavigatorNames.clear();
    if(!m_bInitialized) return 0u;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::const_iterator itorNavigator = m_mapAllNavigators.begin();
    for( ; itorNavigator != m_mapAllNavigators.end(); ++itorNavigator)
    {
        const std::string &strName = itorNavigator->first;
        vecNavigatorNames.push_back(strName);
    }
    return (unsigned)vecNavigatorNames.size();
}


bool NavigatorManager::setActiveNavigator(const std::string &strName)
{
    if(!m_bInitialized) return false;

    std::map<std::string, OpenSP::sp<NavigatorBase> >::const_iterator itorFind = m_mapAllNavigators.find(strName);
    if(itorFind == m_mapAllNavigators.end())
    {
        return false;
    }
    m_strActiveNavigator = strName;
    return true;
}


const std::string &NavigatorManager::getActiveNavigator(void) const
{
    return m_strActiveNavigator;
}


const NavigationParam *NavigatorManager::getNavigationParam(void) const
{
    return m_pNavigationParam.get();
}


NavigationParam *NavigatorManager::getNavigationParam(void)
{
    return m_pNavigationParam.get();
}


void NavigatorManager::setEventOnPoseChanged(bool bPost)
{
    m_bHasEventOnPoseChanged = bPost;
    if(bPost)
    {
        if(!m_pEventThread)
        {
            m_pEventThread = new EventThread(m_pEventAdapter.get());
            m_pEventThread->startThread();
        }
    }
    else
    {
        if(m_pEventThread)
        {
            m_pEventThread->finishMission();
            delete m_pEventThread;
            m_pEventThread = NULL;
        }
    }
}


bool NavigatorManager::hasEventOnPoseChanged(void) const
{
    return m_bHasEventOnPoseChanged;
}

void NavigatorManager::enableInertia(const std::string &strNavigatorName, bool bEnable)
{
	if(!m_bInitialized) return;

	std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itr = m_mapAllNavigators.find(strNavigatorName);
	if(itr == m_mapAllNavigators.end())
	{
		return;
	}

	NavigatorBase *pNavigator = itr->second.get();
	pNavigator->enableInertia(bEnable);
}

void NavigatorManager::enableUnderGroundViewMode(const std::string &strNavigatorName, bool bEnable)
{
	if(!m_bInitialized) return;

	std::map<std::string, OpenSP::sp<NavigatorBase> >::iterator itr = m_mapAllNavigators.find(strNavigatorName);
	if(itr == m_mapAllNavigators.end())
	{
		return;
	}

	NavigatorBase *pNavigator = itr->second.get();
	pNavigator->enableUnderGroundViewMode(bEnable);
}