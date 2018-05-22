#ifndef NAVIGATOR_MANAGER_H_77705C74_D155_4D80_A63D_1E662540E7E4_INCLUDE
#define NAVIGATOR_MANAGER_H_77705C74_D155_4D80_A63D_1E662540E7E4_INCLUDE

#include "INavigatorManager.h"
#include "NavigatorBase.h"
#include <vector>
#include <map>
#include <string>
#include <OpenSP/sp.h>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgGA/CameraManipulator>
#include <OpenThreads/Thread>
#include <queue>


#pragma warning (disable : 4250)

class NavigatorManager : public INavigatorManager, public osgGA::CameraManipulator
{
public:
    explicit NavigatorManager(ea::IEventAdapter *pEventAdapter = NULL);
             NavigatorManager(const NavigatorManager &navi);
    virtual ~NavigatorManager(void);

public:  // methods from INavigatorManager
    virtual bool                    initialize(void);

    virtual unsigned                getNavigators(std::vector<std::string> &vecNavigatorNames) const;

    virtual bool                    setActiveNavigator(const std::string &strName);
    virtual const std::string      &getActiveNavigator(void) const;

    virtual NavigationParam        *getNavigationParam(void);
    virtual const NavigationParam  *getNavigationParam(void) const;

    virtual void        resetCamera(void);
    virtual void        stopInertia(void);

    virtual void        getCameraPose(CameraPose &pose) const;
    virtual void        setCameraPose(const CameraPose &pose);

    virtual void        setEventOnPoseChanged(bool bPost);
    virtual bool        hasEventOnPoseChanged(void) const;

	virtual void        enableInertia(const std::string &strNavigatorName, bool bEnable);
	virtual void        enableUnderGroundViewMode(const std::string &strNavigatorName, bool bEnable);

public:  // Methods from osgGA::CameraManipulator
    virtual bool                handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual void                setByMatrix(const osg::Matrixd& matrix);
    virtual void                setByInverseMatrix(const osg::Matrixd& matrix);
    virtual osg::Matrixd        getMatrix(void) const;
    virtual osg::Matrixd        getInverseMatrix(void) const;
    virtual void                setTargetNode(osg::Node *pTargetNode){}
    virtual void                setNaviNode(osg::Node *pTargetNode);

protected:
    bool                            m_bInitialized;
    OpenSP::sp<NavigationParam>     m_pNavigationParam;
    std::string                     m_strActiveNavigator;
    volatile bool                   m_bHasEventOnPoseChanged;
    std::map<std::string, OpenSP::sp<NavigatorBase> >   m_mapAllNavigators;
    OpenSP::sp<ea::IEventAdapter>               m_pEventAdapter;

    class EventThread : public OpenThreads::Thread
    {
    public:
        explicit EventThread(ea::IEventAdapter *pEventAdapter) : m_pEventAdapter(pEventAdapter), m_nMaxQueueLength(5u), m_MissionFinished(0u)
        {
            setStackSize(16u * 1024u);
        }
        virtual ~EventThread(void)
        {        }

        void postEvent(const std::string &strEvent);
        void finishMission(void)
        {
            m_MissionFinished.exchange(1);
            join();
        }

    protected:
        virtual void run(void);

    protected:
        OpenSP::sp<ea::IEventAdapter>   m_pEventAdapter;
        OpenThreads::Atomic             m_MissionFinished;

        std::queue<std::string>     m_queueEvents;
        unsigned                    m_nMaxQueueLength;
        OpenThreads::Mutex          m_mtxQueueEvents;
    };

    EventThread    *m_pEventThread;
};


#endif
