#include <osgGA/GUIEventHandler>
#include <osg/Group>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include "ICompass.h"

#pragma warning (disable : 4250)


class NavigatorManager;

class ArrowWidget;
class Compass : public ICompass, public osg::Group
{
public:
    explicit Compass(void);
    virtual ~Compass(void);

public:     // Methods from ICompass
    virtual void setLayoutType(const std::string &strLayout);
    virtual void setVisible(bool bVisible);
    virtual const std::string &getLayoutType(void) const{   return m_strLayoutType; }
    virtual bool  isVisible(void) const                 {   return m_bVisible;      }

    virtual void setNorthernRingVisible(bool bVisible);
    virtual bool getNorthernRingVisible(void) const;

    virtual void setRotationDiskVisible(bool bVisible);
    virtual bool getRotationDiskVisible(void) const;

    virtual void setTranslationDiskVisible(bool bVisible);
    virtual bool getTranslationDiskVisible(void) const;

    virtual void setElevationBarVisible(bool bVisible);
    virtual bool getElevationBarVisible(void) const;

public:
    void setNavigatorManager(NavigatorManager *pNavManager)
    {
        m_pNavigationManager = pNavManager;
    }

    bool initialize(void);
    bool handleEvent(osgViewer::View *pView);
    void setRefreshSpeed(unsigned nSpeed)   { m_nRefreshSpeed = osg::clampAbove(nSpeed, 1u);    }


protected:
    bool setupScene(osgViewer::View *pView);
    void resetPosition(osgViewer::View *pView);

protected:
    friend class NorthRotateWidget;
    friend class RotateWidget;
    friend class MoveWidget;
    friend class ZoomWidget;
protected:
    osg::ref_ptr<osgWidget::WindowManager>  m_pWindowManager;
    bool                                    m_bVisible;
    bool                                    m_bNorthernRingVisible;
    bool                                    m_bRotaionDiskVisible;
    bool                                    m_bTranslationDiskVisible;
    bool                                    m_bElevationBarVisible;

    osg::ref_ptr<osgWidget::Box>            m_pNorthRotateBox;
    osg::ref_ptr<osgWidget::Box>            m_pRotateBox;
    osg::ref_ptr<osgWidget::Box>            m_pMoveBox;
    osg::ref_ptr<osgWidget::Box>            m_pZoomBox;

    osg::ref_ptr<osg::Camera>               m_pOrth2DCamera;
    osg::ref_ptr<NavigatorManager>          m_pNavigationManager;

    osg::ref_ptr<ArrowWidget>               m_pNorthRoateWidget;
    osg::ref_ptr<ArrowWidget>               m_pRoateWidget;
    osg::ref_ptr<ArrowWidget>               m_pMoveWidget;
    osg::ref_ptr<ArrowWidget>               m_pZoomWidget;

    //std::vector<osg::ref_ptr<ArrowWidget> >  m_vecArrowWidgets;

    std::string   m_strLayoutType;
    unsigned      m_nRefreshSpeed;

    int           m_nRotateNorthSize;
    int           m_nRotateSize;
    int           m_nMoveSize;
    int           m_nZoomWidth;
    int           m_nZoomHeight;

    static osg::ref_ptr<osg::Image>         m_pRotateNorthImg;
    static osg::ref_ptr<osg::Image>         m_pRotateImg;
    static osg::ref_ptr<osg::Image>         m_pMoveImg;
    static osg::ref_ptr<osg::Image>         m_pZoomImg;
};

