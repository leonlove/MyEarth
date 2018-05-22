#include "Compass.h"
#include <iostream>
#include <cstdio>
#include <osgWidget/ViewerEventHandlers>
#include <osgViewer/ViewerEventHandlers>
#include <List>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/PolygonMode>
#include <osgUtil/FindNodeVisitor.h>
#include "NavigatorManager.h"
#include <common/Common.h>
#include <algorithm>
#include "CameraInfo.h"

const unsigned int MASK_2D = 0xF0000000;

osg::ref_ptr<osg::Image> Compass::m_pRotateNorthImg = NULL;
osg::ref_ptr<osg::Image> Compass::m_pRotateImg      = NULL;
osg::ref_ptr<osg::Image> Compass::m_pMoveImg        = NULL;
osg::ref_ptr<osg::Image> Compass::m_pZoomImg        = NULL;


class CompassWindow : public osgWidget::Box
{
public:
    CompassWindow(void)
    {
    }

protected:

};


//////////////////////////////////////////////////////////////////////////
//RoateWindow
//////////////////////////////////////////////////////////////////////////
class RotateWindow : public CompassWindow
{
public:
    RotateWindow(void)
    {

    }

    virtual void update(void)
    {
        osgWidget::Window::WindowList wl;
        getEmbeddedList(wl);
        for(osgWidget::Window::WindowList::iterator w = wl.begin(); w != wl.end(); w++) w->get()->update();

        osgWidget::matrix_type x  = _x;
        osgWidget::matrix_type y  = _y;
        osgWidget::XYCoord     xy = getAbsoluteOrigin();

        // We only honor ANCHOR requests on topmost Windows, not embedded ones.
        if((_vAnchor != VA_NONE || _hAnchor != HA_NONE) && !_parent && _wm) {
            if(_vAnchor == VA_TOP) y = _wm->getHeight() - _height.current;
            else if(_vAnchor == VA_CENTER) y = osg::round(_wm->getHeight() / 2.0f);
            else if(_vAnchor == VA_BOTTOM) y = 0.0f;

            if(_hAnchor == HA_LEFT) x = 0.0f;
            else if(_hAnchor == HA_CENTER) x = osg::round((_wm->getWidth() - _width.current)/ 2.0f);
            else if(_hAnchor == HA_RIGHT) x = _wm->getWidth() - _width.current + _visibleArea[2];

            xy.set(x, y);
        }

        osgWidget::matrix_type z = _z;

        // We can't do proper scissoring until we have access to our parent WindowManager, and
        // we need to determine the sorting method we want to use.
        if(_wm) {
            if(_wm->isUsingRenderBins()) {
                getOrCreateStateSet()->setRenderBinDetails(
                    static_cast<int>((1.0f - fabs(_z)) * osgWidget::OSGWIDGET_RENDERBIN_MOD),
                    "RenderBin"
                    );

                z = 0.0f;
            }

            int sx = static_cast<int>(xy.x());
            int sy = static_cast<int>(xy.y());
            int sw = static_cast<int>(_width.current);
            int sh = static_cast<int>(_height.current);

            // This sets the Scissor area to some offset defined by the user.
            if(_vis == VM_PARTIAL) {
                sw = static_cast<int>(_visibleArea[2]);
                sh = static_cast<int>(_visibleArea[3]);
            }

            // Otherwise, use the size of the WindowManager itself.
            else if(_vis == VM_ENTIRE) {
                sx = 0;
                sy = 0;
                sw = static_cast<int>(_wm->getWidth());
                sh = static_cast<int>(_wm->getHeight());
            }

            _scissor()->setScissor(sx, sy, sw, sh);
        }
    }
};


class ArrowWidget : public osgWidget::Widget
{
public:
    ArrowWidget(NavigatorManager *pManipulator, Compass *pCompass)
    {
        m_pManipulator  = pManipulator;
        m_pCompass      = pCompass;
        m_bMouseDown    = false;
    }

    virtual bool manipulater(void) = 0;

protected:
    NavigatorManager   *m_pManipulator;
    Compass            *m_pCompass;
    bool                m_bMouseDown;
    osgWidget::Point    m_ptClickPoint;
};


//////////////////////////////////////////////////////////////////////////
//  RoateCompassBox
//////////////////////////////////////////////////////////////////////////
class NorthRotateWidget: public ArrowWidget 
{
public:
    NorthRotateWidget(NavigatorManager *pManipulator, Compass *pCompass)
     : ArrowWidget(pManipulator, pCompass)
    {
        setEventMask(osgWidget::EVENT_ALL & ~osgWidget::EVENT_MOUSE_OVER);
        m_bMouseDown = false;
        m_dblStateAngle = 0.0;
        m_dblStartNorthAngle = 0.0;
    }


    virtual bool manipulater(void)
    {
        return false;
    }

    virtual bool mouseDrag(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.x() += x;
        m_ptClickPoint.y() += y;

        if (!m_bMouseDown)
        {
            return false;
        }

        osgWidget::Window *pRoateWindow = getParent();

        osgWidget::Point centerPoint = pRoateWindow->getPosition();
        centerPoint.z() = 0.0;
        centerPoint.x() += m_pCompass->m_nRotateNorthSize * 0.5;
        centerPoint.y() += m_pCompass->m_nRotateNorthSize * 0.5;

        osgWidget::Point curPoint(m_ptClickPoint.x(), m_ptClickPoint.y(), centerPoint.z());

        osg::Vec3 curDir = curPoint - centerPoint;
        curDir.normalize();

        osg::Vec3 clickDir = m_StartPoint - centerPoint;
        clickDir.normalize();

        const double ac = curDir * clickDir;
        double fAngle = acos(ac);

        osg::Vec3 rightDir = clickDir ^ osg::Vec3(0.0f, 0.0f, 1.0f);
        rightDir.normalize();
        const double fRight = curDir * rightDir;
        if (fRight < 0.0)
        {
            fAngle = osg::PI + osg::PI - fAngle;
        }

        CameraPose pose;
        m_pManipulator->getCameraPose(pose);
        pose.m_dblAzimuthAngle = m_dblStartNorthAngle + fAngle /*+ osg::PI_2*/;
        m_pManipulator->setCameraPose(pose);
        return true;
    }


    virtual bool mousePush(double x, double y, const osgWidget::WindowManager *pWndM) 
    {
        m_StartPoint.set(x, y, 0.0);

        m_ptClickPoint = m_StartPoint;
        osgWidget::XYCoord xy = localXY(x,y);
        if (xy.x() < 0.0)
        {
            return false;
        }
        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001) 
        {
            return false;
        }

        osgWidget::Window *pRoateWindow = getParent();
        const osg::Matrixd mat = pRoateWindow->getMatrix();

        const osg::Quat  qtLastR = mat.getRotate();
        osg::Vec3d vecRoateDir;
        qtLastR.getRotate(m_dblStateAngle, vecRoateDir);

        m_bMouseDown = true;
        CameraPose pose;
        m_pManipulator->getCameraPose(pose);
        m_dblStartNorthAngle = pose.m_dblAzimuthAngle;
        return true; 
    }

    virtual bool mouseRelease(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_bMouseDown = false;
        return false;
    }

    virtual bool keyUp(int key, int keyMask, osgWidget::WindowManager*) 
    {
        return true;
    }

protected:
    double                  m_dblStartNorthAngle;
    double                  m_dblStateAngle;
    osgWidget::Point        m_StartPoint;
};


//////////////////////////////////////////////////////////////////////////
//RoateWidget
//////////////////////////////////////////////////////////////////////////
class RotateWidget: public ArrowWidget
{
public:
    RotateWidget(NavigatorManager *pManipulator, Compass *pCompass)
        : ArrowWidget(pManipulator, pCompass)
    {
        m_bMouseDown = false;
        setEventMask(osgWidget::EVENT_ALL& ~osgWidget::EVENT_MOUSE_OVER);
    }

    virtual bool manipulater(void)
    {
        if(!m_bMouseDown)   return false;

        osgWidget::Window *pRotateWindow = getParent();

        osgWidget::Point centerPoint = pRotateWindow->getPosition();
        centerPoint.x() += m_pCompass->m_nRotateSize * 0.5;
        centerPoint.y() += m_pCompass->m_nRotateSize * 0.5;
        centerPoint.z() = 0.0;

        osgWidget::Point curPoint = centerPoint;
        curPoint.x() = m_ptClickPoint.x();
        curPoint.y() = m_ptClickPoint.y();

        osg::Vec3f curDir = curPoint - centerPoint;
        curDir.normalize();

        CameraPose pose;
        m_pManipulator->getCameraPose(pose);
        pose.m_dblAzimuthAngle += curDir.x() * 0.01;
        pose.m_dblPitchAngle   += curDir.y() * 0.01;
        pose.m_dblPitchAngle    = osg::clampBetween(pose.m_dblPitchAngle, 0.0, osg::DegreesToRadians(160.0));
        m_pManipulator->setCameraPose(pose);
        return true;
    }

    virtual bool mouseDrag(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.x() += x;
        m_ptClickPoint.y() += y;

        return m_bMouseDown;
    }

    virtual bool mousePush(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.set(x, y, 0.0);
        osgWidget::XYCoord xy = localXY(x,y);
        if (xy.x() < 0.0)
        {
            return false;
        }
        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001) 
        {
            return false;
        }
        m_bMouseDown = true;
        return true; 
    }

    virtual bool mouseRelease(double x, double y, const osgWidget::WindowManager *pWndM) 
    {
        m_bMouseDown = false;
        return false;
    }
};


//////////////////////////////////////////////////////////////////////////
//MoveWidget
//////////////////////////////////////////////////////////////////////////
class MoveWidget: public ArrowWidget
{
public:
    MoveWidget(NavigatorManager *pManipulator, Compass *pCompass)
        : ArrowWidget(pManipulator, pCompass)
    {
        setEventMask(osgWidget::EVENT_ALL & ~osgWidget::EVENT_MOUSE_OVER);
    }

    virtual bool manipulater(void)
    {
        if(!m_bMouseDown)   return false;

        osgWidget::Window *pRotateWindow = getParent();

        osgWidget::Point centerPoint = pRotateWindow->getPosition();
        centerPoint.z()  = 0.0;
        centerPoint.x() += m_pCompass->m_nMoveSize * 0.5;
        centerPoint.y() += m_pCompass->m_nMoveSize * 0.5;

        const osgWidget::Point curPoint(m_ptClickPoint.x(), m_ptClickPoint.y(), centerPoint.z());

        osg::Vec3 curDir = curPoint - centerPoint;
        curDir.normalize();

        double fAngle = curDir * osg::Vec3(1.0f, 0.0f, 0.0f);
        fAngle = acos(fAngle);
        if (curDir.y() < 0.0)
        {
            fAngle = -fAngle;
        }
        fAngle -= osg::PI_2;

        CameraPose pose;
        m_pManipulator->getCameraPose(pose);
        const double dx = cos(pose.m_dblAzimuthAngle + fAngle);
        const double dy = sin(pose.m_dblAzimuthAngle + fAngle);

        const double dCurHeihgt = osg::absolute(pose.m_dblHeight) / 1000000000.0;
        pose.m_dblPositionX += dx * dCurHeihgt;
        pose.m_dblPositionY += dy * dCurHeihgt;

        m_pManipulator->setCameraPose(pose);
        return true;
    }

    virtual bool mouseDrag(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.x() += x;
        m_ptClickPoint.y() += y;

        return m_bMouseDown;
    }

    virtual bool mousePush(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.set(x, y, 0.0);
        osgWidget::XYCoord xy = localXY(x, y);
        if (xy.x() < 0.0)
        {
            return false;
        }
        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001) 
        {
            return false;
        }
        m_bMouseDown = true;
        return true; 
    }

    virtual bool mouseRelease(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_bMouseDown = false;
        return false;
    }
};


//////////////////////////////////////////////////////////////////////////
//ZoomWidget
//////////////////////////////////////////////////////////////////////////
class ZoomWidget: public ArrowWidget
{
public:
    ZoomWidget(NavigatorManager *pManipulator, Compass *pCompass)
        :ArrowWidget(pManipulator, pCompass)
    {
        setEventMask(osgWidget::EVENT_ALL& ~osgWidget::EVENT_MOUSE_OVER);
    }

    virtual bool manipulater(void)
    {
        if(!m_bMouseDown)
        {
            return false;
        }

        osgWidget::Window *pRoateWindow = getParent();

        osgWidget::Point centerPoint = pRoateWindow->getPosition();
        centerPoint.z() = 0.0;
        centerPoint.x() += m_pCompass->m_nZoomWidth * 0.5;
        centerPoint.y() += m_pCompass->m_nZoomHeight * 0.5;

        const osgWidget::Point curPoint(m_ptClickPoint.x(), m_ptClickPoint.y(), centerPoint.z());
        osg::Vec3 curDir = curPoint - centerPoint;
        curDir.normalize();

        CameraPose pose;
        m_pManipulator->getCameraPose(pose);
        pose.m_dblHeight -= curDir.y() * pose.m_dblHeight * 0.005;
        m_pManipulator->setCameraPose(pose);

        return true;
    }

    virtual bool mouseDrag(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.x() += x;
        m_ptClickPoint.y() += y;

        return m_bMouseDown;
    }

    virtual bool mousePush(double x, double y, const osgWidget::WindowManager*pWndM) 
    {
        m_ptClickPoint.set(x, y, 0.0);
        osgWidget::XYCoord xy = localXY(x,y);
        if (xy.x() < 0)
        {
            return false;
        }
        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001f) 
        {
            return false;
        }
        m_bMouseDown = true;
        return true; 
    }

    virtual bool mouseRelease(double x, double y, const osgWidget::WindowManager *pWndM)
    {
        m_bMouseDown = false;
        return false;
    }
};


class CompassEventAdapter : public osgGA::GUIEventHandler
{
public:
    explicit CompassEventAdapter(Compass *pCompass)
    {
        m_pCompass = pCompass;
    }

protected:
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
    {
        const osgGA::GUIEventAdapter::EventType eType = ea.getEventType();
        if(eType != osgGA::GUIEventAdapter::FRAME)
        {
            return false;
        }

        osgViewer::View *pView = dynamic_cast<osgViewer::View *>(&aa);
        if(!pView)  return false;

        return m_pCompass->handleEvent(pView);
    }

    Compass    *m_pCompass;
};


class WindowMouseHandler : public osgWidget::MouseHandler
{
public:
    WindowMouseHandler(osgWidget::WindowManager *pWindowManager) : osgWidget::MouseHandler(pWindowManager)
    {
        m_bInitialized = false;
    }

    bool initSceneGraph(osg::Node *pSceneRootNode)
    {
        m_pElementRootNode = NULL;
        m_pTempElementRootNode = NULL;
        m_bInitialized = false;

        if(!pSceneRootNode)     return false;

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder1 = new osgUtil::FindNodeByNameVisitor("ElementRootGroup");
        pSceneRootNode->accept(*pFinder1);
        m_pElementRootNode = pFinder1->getTargetNode();

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder2 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
        pSceneRootNode->accept(*pFinder2);
        m_pTempElementRootNode = pFinder2->getTargetNode();

        if(!m_pElementRootNode.valid() || !m_pTempElementRootNode.valid())
        {
            m_pElementRootNode = NULL;
            m_pTempElementRootNode = NULL;
            m_bInitialized = false;
            return false;
        }

        m_bInitialized = true;
        m_bMouseDown = false;
        return true;
    }

protected:
    bool    m_bInitialized;
    osg::ref_ptr<osg::Node>     m_pElementRootNode;
    osg::ref_ptr<osg::Node>     m_pTempElementRootNode;
    bool    m_bMouseDown;

    virtual bool handle(
        const osgGA::GUIEventAdapter& gea,
        osgGA::GUIActionAdapter&      gaa,
        osg::Object*                  obj,
        osg::NodeVisitor*             nv)
    {
        if(!m_bInitialized) return false;

        const osgGA::GUIEventAdapter::EventType ev = gea.getEventType();
        if(ev == osgGA::GUIEventAdapter::PUSH)
        {
            m_bMouseDown = true;
        }
        else if(ev == osgGA::GUIEventAdapter::RELEASE)
        {
            m_bMouseDown = false;
        }
        else if(ev == osgGA::GUIEventAdapter::MOVE || ev == osgGA::GUIEventAdapter::DRAG)
        {
            if(!m_bMouseDown)
            {
                return false;
            }
        }

        MouseAction ma = _isMouseEvent(ev);
        if(ma)
        {
            const osg::Node::NodeMask eMaskElement = m_pElementRootNode->getNodeMask();
            const osg::Node::NodeMask eMaskTempElement = m_pTempElementRootNode->getNodeMask();
            m_pElementRootNode->setNodeMask(0u);
            m_pTempElementRootNode->setNodeMask(0u);

            // If we're scrolling, we need to inform the WindowManager of that.
            _wm->setScrollingMotion(gea.getScrollingMotion());

            const bool bRetVal = (this->*ma)(gea.getX(), gea.getY(), gea.getButton());

            m_pElementRootNode->setNodeMask(eMaskElement);
            m_pTempElementRootNode->setNodeMask(eMaskTempElement);
            return bRetVal;
        }
    
        return false;
    }
};


//////////////////////////////////////////////////////////////////////////
//Compass
//////////////////////////////////////////////////////////////////////////
Compass::Compass(void)
{
    m_bVisible          = true;
    m_bNorthernRingVisible = true;
    m_bRotaionDiskVisible = true;
    m_bTranslationDiskVisible = true;
    m_bElevationBarVisible = true;
    m_pWindowManager    = NULL;
    m_pNorthRotateBox   = NULL;
    m_pRotateBox        = NULL;
    m_pMoveBox          = NULL;
    m_pZoomBox          = NULL;
    m_pOrth2DCamera     = NULL;
    m_nRefreshSpeed     = 5u;
    m_strLayoutType     = "right";

    m_nRotateNorthSize  = 142;
    m_nRotateSize       = 80;
    m_nMoveSize         = 100;
    m_nZoomWidth        = 30;
    m_nZoomHeight       = 160;

    if(!m_pRotateNorthImg.valid())
    {
        std::string strNorthRoateImgFile = cmm::genResourceFileDir() + "north.png";
        m_pRotateNorthImg = osgDB::readImageFile(strNorthRoateImgFile);
    }

    if(!m_pRotateImg.valid())
    {
        std::string strRoateImgFile = cmm::genResourceFileDir() + "roate.png";
        m_pRotateImg = osgDB::readImageFile(strRoateImgFile);
    }

    if(!m_pMoveImg.valid())
    {
        std::string strMoveImgFile = cmm::genResourceFileDir() + "move.png";
        m_pMoveImg = osgDB::readImageFile(strMoveImgFile);
    }

    if(!m_pZoomImg.valid())
    {
        std::string strZoomImgFile = cmm::genResourceFileDir() + "zoom.png";
        m_pZoomImg = osgDB::readImageFile(strZoomImgFile);
    }
}


Compass::~Compass(void)
{
    clearEventCallback();
}


bool Compass::initialize(void)
{
    if(!m_pRotateNorthImg.valid() || !m_pRotateImg.valid() || !m_pMoveImg.valid() ||!m_pZoomImg.valid())
    {
        return false;
    }

    osg::ref_ptr<CompassEventAdapter>    pEventHandler = new CompassEventAdapter(this);
    addEventCallback(pEventHandler.get());

    return true;
}


void Compass::setLayoutType(const std::string &strLayout)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }

    if(strLayout.compare("right") == 0)
    {
        m_strLayoutType = strLayout;
    }
    else if(strLayout.compare("left") == 0)
    {
        m_strLayoutType = strLayout;
    }
}


void Compass::setVisible(bool bVisible)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }

    if (bVisible)
    {
        m_pWindowManager->setAllChildrenOn();
    }
    else
    {
        m_pWindowManager->setAllChildrenOff();
    }
}


void Compass::setNorthernRingVisible(bool bVisible)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }
    m_pWindowManager->setChildValue(m_pNorthRotateBox.get(), bVisible);
}


bool Compass::getNorthernRingVisible(void) const
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return false;
    }
    return m_pWindowManager->getChildValue(m_pNorthRotateBox.get());
}


void Compass::setRotationDiskVisible(bool bVisible)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }
    m_pWindowManager->setChildValue(m_pRotateBox.get(), bVisible);
}


bool Compass::getRotationDiskVisible(void) const
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return false;
    }

    return m_pWindowManager->getChildValue(m_pRotateBox.get());
}


void Compass::setTranslationDiskVisible(bool bVisible)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }
    m_pWindowManager->setChildValue(m_pMoveBox.get(), bVisible);
}


bool Compass::getTranslationDiskVisible(void) const
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return false;
    }
    return m_pWindowManager->getChildValue(m_pMoveBox.get());
}


void Compass::setElevationBarVisible(bool bVisible)
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return;
    }
    m_pWindowManager->setChildValue(m_pZoomBox.get(), bVisible);
}


bool Compass::getElevationBarVisible(void) const
{
    if(!m_pWindowManager.valid())
    {
        assert(false);
        return false;
    }
    return m_pWindowManager->getChildValue(m_pZoomBox.get());
}


bool Compass::handleEvent(osgViewer::View *pView)
{
    if(!m_pWindowManager.valid())
    {
        std::cout << "setupScene In Compass." << std::endl;
        if(!setupScene(pView))
        {
            return false;
        }
    }

    m_pNorthRoateWidget->manipulater();
    m_pRoateWidget->manipulater();
    m_pMoveWidget->manipulater();
    m_pZoomWidget->manipulater();

    CameraPose pose;
    m_pNavigationManager->getCameraPose(pose);

    const osgWidget::Quad  visibleArea = m_pNorthRotateBox->getVisibleArea();
    const osgWidget::Point point       = m_pNorthRotateBox->getPosition();
    const osg::Matrixd ts   = osg::Matrixd::translate(-0.5 * m_nRotateNorthSize, -0.5 * m_nRotateNorthSize, 0.0);
    const osg::Matrixd tss  = osg::Matrixd::translate( 0.5 * m_nRotateNorthSize,  0.5 * m_nRotateNorthSize, 0.0);
    const osg::Matrixd r    = osg::Matrixd::rotate(osg::PI_2 - pose.m_dblAzimuthAngle, osg::Vec3d(0.0, 0.0, 1.0));
    const osg::Matrixd t    = osg::Matrixd::translate(point.x() - visibleArea[0], point.y() - visibleArea[1], point.z());

    m_pNorthRotateBox->setMatrix(ts * r * tss * t);

    if(pView->getFrameStamp()->getFrameNumber() % m_nRefreshSpeed == 0u)
    {
        resetPosition(pView);
    }

    return false;
}


bool Compass::setupScene(osgViewer::View *pView)
{
    osg::Camera *pCamera = pView->getCamera();
    osg::Viewport *pViewport = pCamera->getViewport();

    m_pWindowManager = new osgWidget::WindowManager(
        pView,
        pViewport->width(),
        pViewport->height(),
        MASK_2D,
        osgWidget::WindowManager::WM_USE_RENDERBINS);
    m_pWindowManager->setPointerFocusMode(osgWidget::WindowManager::PFM_SLOPPY);

    osg::ref_ptr<WindowMouseHandler> pMouseHandler = new WindowMouseHandler(m_pWindowManager.get());
    pMouseHandler->initSceneGraph(pView->getSceneData());
    pView->addEventHandler(pMouseHandler.get());

    {
        m_pNorthRotateBox = new RotateWindow();
        m_pNorthRotateBox->getBackground()->setColor(osg::Vec4f(0,1,0,0));

        m_pNorthRoateWidget = new NorthRotateWidget(m_pNavigationManager.get(), this);
        m_pNorthRoateWidget->setImage(m_pRotateNorthImg.get());
        m_pNorthRoateWidget->setColor(osg::Vec4f(1.0f,1.0f,1.0f,0.7f));
        m_pNorthRoateWidget->setTexCoord(0.0f,0.0f,osgWidget::Widget::LOWER_LEFT);
        m_pNorthRoateWidget->setTexCoord(1.0f,0.0f,osgWidget::Widget::LOWER_RIGHT);
        m_pNorthRoateWidget->setTexCoord(1.0f,1.0f,osgWidget::Widget::UPPER_RIGHT);
        m_pNorthRoateWidget->setTexCoord(0.0f,1.0f,osgWidget::Widget::UPPER_LEFT);

        m_pNorthRotateBox->addWidget(m_pNorthRoateWidget);
        m_pNorthRotateBox->attachMoveCallback();
        m_pWindowManager->addChild(m_pNorthRotateBox);
    }

    {
        m_pRotateBox = new CompassWindow();
        m_pRotateBox->getBackground()->setColor(osg::Vec4f(0,1,0,0));

        m_pRoateWidget = new RotateWidget(m_pNavigationManager.get(), this);
        m_pRoateWidget->setImage(m_pRotateImg);
        m_pRoateWidget->setColor(osg::Vec4f(1.0f,1.0f,1.0f,0.7f));
        m_pRoateWidget->setTexCoord(0.0f,0.0f,osgWidget::Widget::LOWER_LEFT);
        m_pRoateWidget->setTexCoord(1.0f,0.0f,osgWidget::Widget::LOWER_RIGHT);
        m_pRoateWidget->setTexCoord(1.0f,1.0f,osgWidget::Widget::UPPER_RIGHT);
        m_pRoateWidget->setTexCoord(0.0f,1.0f,osgWidget::Widget::UPPER_LEFT);

        m_pRotateBox->addWidget(m_pRoateWidget);
        m_pWindowManager->addChild(m_pRotateBox);
    }

    {
        m_pMoveBox = new CompassWindow();

        m_pMoveBox->getBackground()->setColor(osg::Vec4f(0,1,0,0));
        m_pMoveWidget = new MoveWidget(m_pNavigationManager.get(), this);
        m_pMoveWidget->setImage(m_pMoveImg.get());
        m_pMoveWidget->setColor(osg::Vec4f(1.0f,1.0f,1.0f,0.7f));
        m_pMoveWidget->setTexCoord(0.0f,0.0f,osgWidget::Widget::LOWER_LEFT);
        m_pMoveWidget->setTexCoord(1.0f,0.0f,osgWidget::Widget::LOWER_RIGHT);
        m_pMoveWidget->setTexCoord(1.0f,1.0f,osgWidget::Widget::UPPER_RIGHT);
        m_pMoveWidget->setTexCoord(0.0f,1.0f,osgWidget::Widget::UPPER_LEFT);

        m_pMoveBox->addWidget(m_pMoveWidget);
        m_pWindowManager->addChild(m_pMoveBox);
    }

    {
        m_pZoomBox = new CompassWindow();

        m_pZoomBox->getBackground()->setColor(osg::Vec4f(0,1,0,0));
        m_pZoomWidget = new ZoomWidget(m_pNavigationManager.get(), this);
        m_pZoomWidget->setImage(m_pZoomImg);
        m_pZoomWidget->setColor(osg::Vec4f(1.0f,1.0f,1.0f,0.7f));
        m_pZoomWidget->setTexCoord(0.0f,0.0f,osgWidget::Widget::LOWER_LEFT);
        m_pZoomWidget->setTexCoord(1.0f,0.0f,osgWidget::Widget::LOWER_RIGHT);
        m_pZoomWidget->setTexCoord(1.0f,1.0f,osgWidget::Widget::UPPER_RIGHT);
        m_pZoomWidget->setTexCoord(0.0f,1.0f,osgWidget::Widget::UPPER_LEFT);

        m_pZoomBox->addWidget(m_pZoomWidget);
        m_pWindowManager->addChild(m_pZoomBox);
    }

    m_pOrth2DCamera = m_pWindowManager->createParentOrthoCamera();
    m_pOrth2DCamera->setName("Compass_Camera");
    resetPosition(pView);

    addChild(m_pOrth2DCamera.get());

    osg::StateSet *pStateset = this->getOrCreateStateSet();
    pStateset->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL));

    return true;
}


void Compass::resetPosition(osgViewer::View *pView)
{
    if(!m_pOrth2DCamera.valid()) 
    {
        return;
    }

    osg::Viewport *pOrth2DViewport  = m_pOrth2DCamera->getViewport();
    if(!pOrth2DViewport)
    {
        m_pOrth2DCamera->setViewport(0, 0, 1, 1);
        pOrth2DViewport = m_pOrth2DCamera->getViewport();
    }

    osg::Camera *pCamera     = pView->getCamera();
    osg::Viewport *pViewport = pCamera->getViewport();

    if(pOrth2DViewport->x() == pViewport->x()
        && pOrth2DViewport->y() == pViewport->y()
        && pOrth2DViewport->width() == pViewport->width()
        && pOrth2DViewport->height() == pViewport->height())
    {
        return;
    }

    unsigned int x = pViewport->x();
    unsigned int y = pViewport->y();
    unsigned int w = pViewport->x() + pViewport->width();
    unsigned int h = pViewport->y() + pViewport->height();

    pOrth2DViewport->setViewport(x, y, pViewport->width(), pViewport->height());
    m_pOrth2DCamera->setProjectionMatrixAsOrtho2D(x, w, y, h);

    unsigned int nSize  = pViewport->height() / 7;
    nSize               = nSize < 40 ? 40 : nSize;

    m_nRotateNorthSize  = nSize;
    m_nRotateSize       = (int)(nSize * 0.8);
    m_nMoveSize         = (int)(nSize * 0.8);
    m_nZoomWidth        = (int)(nSize * 0.2);
    m_nZoomHeight       = (int)(m_nZoomWidth * 5);

    const int maxWidth = osg::maximum(m_nRotateNorthSize,osg::maximum(m_nMoveSize,m_nZoomWidth));
    const int SplitHeight = 5;
    int curHeight = h - m_nRotateNorthSize - SplitHeight;

    const bool bLayoutRight = (m_strLayoutType.compare("right") == 0);

    osgWidget::Point point = m_pNorthRotateBox->getPosition();
    point.z() = 0;
    if(bLayoutRight)
    {
        point.x() = w - m_nRotateNorthSize - ((maxWidth - m_nRotateNorthSize)/2.0);
    }
    else
    {
        point.x() =  ((maxWidth - m_nRotateNorthSize)/2.0);
    }
    point.y() = curHeight;
    m_pNorthRotateBox->setPosition(point);
    //m_pNorthRotateBox->update();

    point = m_pNorthRotateBox->getPosition();
    point.z() = 0;
    if(bLayoutRight)
    {
        point.x() = w - m_nRotateSize - ((maxWidth - m_nRotateSize)/2.0);
    }
    else
    {
        point.x() = ((maxWidth - m_nRotateSize) / 2.0);
    }
    point.y() = curHeight + (m_nRotateNorthSize - m_nRotateSize)/2.0;
    m_pRotateBox->setPosition(point);
    //m_pRotateBox->update();

    curHeight -= m_nMoveSize;
    curHeight -= SplitHeight;
    point = m_pMoveBox->getPosition();
    point.z() = 0;
    if(bLayoutRight)
    {
        point.x() = w - m_nMoveSize - ((maxWidth - m_nMoveSize)/2.0);
    }
    else
    {
        point.x() = ((maxWidth - m_nMoveSize)/2.0);
    }
    point.y() = curHeight;
    m_pMoveBox->setPosition(point);
    //m_pMoveBox->update();

    curHeight -= m_nZoomHeight;
    curHeight -= SplitHeight;
    point = m_pZoomBox->getPosition();
    point.z() = 0;
    if(bLayoutRight)
    {
        point.x() = w - m_nZoomWidth - ((maxWidth - m_nZoomWidth)/2.0);
    }
    else
    {
        point.x() = ((maxWidth - m_nZoomWidth)/2.0);
    }
    point.y() = curHeight;
    m_pZoomBox->setPosition(point);
    //m_pZoomBox->update();

    m_pNorthRoateWidget->setSize(osgWidget::XYCoord(m_nRotateNorthSize, m_nRotateNorthSize));
    m_pRoateWidget->setSize(osgWidget::XYCoord(m_nRotateSize, m_nRotateSize));
    m_pMoveWidget->setSize(osgWidget::XYCoord(m_nMoveSize, m_nMoveSize));
    m_pZoomWidget->setSize(osgWidget::XYCoord(m_nZoomWidth, m_nZoomHeight));

    m_pWindowManager->setSize(w, h);
    m_pWindowManager->setWindowSize(w, h);
    m_pWindowManager->resizeAllWindows();
}


