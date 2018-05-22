#include "StatusBar.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>
#include <Common/Common.h>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/CullVisitor>
#include <osg/PolygonMode>
#include <osg/CoordinateSystemNode>
#include <osgViewer/View>
#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgUtil/Radial.h>
#include <osg/Depth>
#include <osgText/Text>
#include <osg/AutoTransform>
#include <osgUtil/FindNodeVisitor.h>
#include <queue>

#include <IDProvider/Definer.h>

#include "HudLayouter.h"
#include "Registry.h"
#include "Utility.h"
#include "CameraInfo.h"

class StatusBarEventHandler : public osgGA::GUIEventHandler
{
public:
    explicit StatusBarEventHandler(StatusBar *pStatusBar) : m_pStatusBar(pStatusBar)
    {
        assert(m_pStatusBar != NULL);
        m_pTerrainPicker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, 0.0, 0.0);
    }

protected:
    virtual ~StatusBarEventHandler(void)    {    }
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    double getFPS(void) const;
    void generateStatusInfo(osgViewer::View &view, const osg::Vec2d &ptMousePos);

    void generateTextTip(osgViewer::View &view, const osg::Vec2d &ptMousePos);

protected:
    osg::ref_ptr<osgUtil::LineSegmentIntersector> m_pTerrainPicker;
    StatusBar          *m_pStatusBar;

    typedef std::pair<unsigned, double>     FrameInfo;
    std::queue<FrameInfo>   m_queueFrameInfo;
};


double StatusBarEventHandler::getFPS(void) const
{
    if(m_queueFrameInfo.size() < 2u)
    {
        return 0.0;
    }
    const FrameInfo &info0 = m_queueFrameInfo.front();
    const FrameInfo &info1 = m_queueFrameInfo.back();
    const unsigned nFrameDelta = info1.first - info0.first;
    const double dblTimeDelta = info1.second - info0.second;
    double dblFPS = nFrameDelta / dblTimeDelta;
    dblFPS = floor(dblFPS * 100.0) * 0.01;
    return dblFPS;
}


void StatusBarEventHandler::generateStatusInfo(osgViewer::View &view, const osg::Vec2d &ptMousePos)
{
	int nFPS = 0;
	const char *ptr = ::getenv("DEU_DEBUG_FPS");
	if(ptr)
	{
		nFPS = ::atoi(ptr);
		if(nFPS == 0x01)
		{
			m_pStatusBar->setFPS(getFPS());
		}
	}

    if(!m_pStatusBar->m_pTargetNode.valid())
    {
        return;
    }

    const osg::Camera *pCamera = view.getCamera();
    const double dblCameraHeight = CameraInfo::instance()->getCameraPose(0u).m_dblHeight;

    osg::Vec3d ptHitPos, ptMouseTerrainPos;
    if(computeIntersection(m_pStatusBar->m_pTargetNode, pCamera, ptMousePos, ptHitPos))
    {
        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        double dblLatitude = 0.0, dblLongitude = 0.0, dblHeight = 0.0;
        pEllipsoidModel->convertXYZToLatLongHeight(ptHitPos.x(), ptHitPos.y(), ptHitPos.z(), dblLatitude, dblLongitude, dblHeight);

        dblLatitude  = osg::RadiansToDegrees(dblLatitude);
        dblLongitude = osg::RadiansToDegrees(dblLongitude);

        const std::pair<double, double>   HeightGap(10000.0, 30000.0);
        const double dblMidHeightGap  = (HeightGap.first + HeightGap.second) * 0.5;
        const double dblRatioOfHeight = (dblCameraHeight - dblMidHeightGap) * 0.001;
        const double dblRatio = 1.0 - cmm::math::logsig(dblRatioOfHeight);
        dblHeight *= dblRatio;

        ptMouseTerrainPos.set(dblLongitude, dblLatitude, dblHeight);

        if(m_pStatusBar->m_pDemHelper.valid())
        {
            ptMouseTerrainPos.z() = m_pStatusBar->m_pDemHelper->getDem(ptMouseTerrainPos.x(), ptMouseTerrainPos.y());
        }
    }
    else
    {
        memset(ptMouseTerrainPos._v, 0xFF, sizeof(ptMouseTerrainPos._v));
    }

    m_pStatusBar->setInfo(ptMouseTerrainPos, dblCameraHeight);
}


void StatusBarEventHandler::generateTextTip(osgViewer::View &view, const osg::Vec2d &ptMousePos)
{
    static osg::ref_ptr<osgText::Text> pText = NULL;
    static osg::ref_ptr<osg::MatrixTransform> pTextTip = NULL;
    static osg::Group* pTempElementRootGroup = NULL;
    static osg::Group* pCultureRootNode = NULL;
    if (pTempElementRootGroup == NULL || pCultureRootNode == NULL)
    {
        osg::ref_ptr<osg::Node> pRootNode = view.getSceneData();
        osg::ref_ptr<osgUtil::FindNodeByNameVisitor> pFinder1 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
        pRootNode->accept(*pFinder1);
        pTempElementRootGroup = dynamic_cast<osg::Group*>(pFinder1->getTargetNode());

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor> pFinder2 = new osgUtil::FindNodeByNameVisitor("CultureRootNode");
        pRootNode->accept(*pFinder2);
        pCultureRootNode = dynamic_cast<osg::Group*>(pFinder2->getTargetNode());

        if (pTempElementRootGroup == NULL || pCultureRootNode == NULL)
        {
            return;
        }

        //创建一个文字节点
        if (pTextTip == NULL)
        {
            pText = new osgText::Text();
            const std::wstring strStringW = cmm::ANSIToUnicode("DeuExplorer");
            osg::ref_ptr<osgText::Font> pTextFont = osgText::readFontFile("SIMSUN.TTC");
            pText->setText(osgText::String(strStringW.c_str()));
            pText->setFont(pTextFont);
            pText->setEnableDepthWrites(false);
            pText->setLayout(osgText::TextBase::LEFT_TO_RIGHT);
            pText->setAlignment(osgText::Text::LEFT_BOTTOM);
            pText->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
            pText->setAutoRotateToScreen(false);
            pText->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
            pText->setCharacterSize(25.0);
            pText->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pText->setBackdropColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
            pText->setBackdropType(osgText::Text::OUTLINE);
            pText->getStateSet()->setRenderBinToInherit();

            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            pGeode->addDrawable(pText.get());
            pGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);
            pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

            osg::ref_ptr<osg::AutoTransform> pAutoTranseform = new osg::AutoTransform;
            pAutoTranseform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
            pAutoTranseform->setAutoScaleToScreen(true);
            pAutoTranseform->setCullingActive(false);
            pAutoTranseform->addChild(pGeode.get());

            pTextTip = new osg::MatrixTransform;
            pTextTip->addChild(pAutoTranseform.get());
            pTextTip->setNodeMask(0);

            pTempElementRootGroup->addChild(pTextTip.get());
        }
    }

    if (pTextTip != NULL)
    {
        osg::Node* pInterNode = NULL;
        osg::Vec3d ptHitPos, ptMouseTerrainPos;
        computeIntersection(pCultureRootNode, view.getCamera(), ptMousePos, ptHitPos, &pInterNode);
        if (pInterNode == NULL)
        {
            pTextTip->setNodeMask(0);
            return;
        }

        pText->setText(osgText::String(pInterNode->getName()));

        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        osg::Matrixd mtx;
        pEllipsoidModel->computeLocalToWorldTransformFromXYZ(ptHitPos.x(), ptHitPos.y(), ptHitPos.z(), mtx);
        pTextTip->setMatrix(mtx);
        pTextTip->setNodeMask(~0);
    }

    return;
}


bool StatusBarEventHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::FRAME)
    {
        return false;
    }

    osgViewer::View &view = dynamic_cast<osgViewer::View &>(aa);
    const osg::FrameStamp *pFrameStamp = view.getFrameStamp();
    FrameInfo info;
    info.first = pFrameStamp->getFrameNumber();
    info.second = pFrameStamp->getReferenceTime();
    m_queueFrameInfo.push(info);
    if(m_queueFrameInfo.size() > 60u)
    {
        m_queueFrameInfo.pop();
    }

    osg::Camera *pHudCamera = m_pStatusBar->m_pHudCamera.get();

    osg::Camera *pCamera = view.getCamera();
    osg::Viewport *pViewport = pCamera->getViewport();
    osg::Viewport *pHudViewport = pHudCamera->getViewport();
    if(pViewport->compare(*pHudViewport) != 0)
    {
        pHudCamera->setViewport(pViewport->x(), pViewport->y(), pViewport->width(), pViewport->height());
        pHudCamera->setProjectionMatrixAsOrtho2D(pViewport->x(), pViewport->x() + pViewport->width(), pViewport->y(), pViewport->y() + pViewport->height());

        osg::Vec3Array *pVertexArray = dynamic_cast<osg::Vec3Array *>(m_pStatusBar->m_pPaneGeom->getVertexArray());
        pVertexArray->at(0).set(pViewport->x(),                         pViewport->y(),                             0.0f);
        pVertexArray->at(1).set(pViewport->x() + pViewport->width(),    pViewport->y(),                             0.0f);
        pVertexArray->at(2).set(pViewport->x() + pViewport->width(),    pViewport->y() + m_pStatusBar->m_nBarWidth, 0.0f);
        pVertexArray->at(3).set(pViewport->x(),                         pViewport->y() + m_pStatusBar->m_nBarWidth, 0.0f);

        m_pStatusBar->m_pFPSTextGeom->setPosition(osg::Vec3(pViewport->x() + 5, pViewport->y() + pViewport->height() - m_pStatusBar->m_dblTextSize / 2.0, 0.0));

        m_pStatusBar->m_pInfoTextGeom->setPosition(osg::Vec3(pViewport->x() + pViewport->width() - 5, pViewport->y() + (m_pStatusBar->m_dblTextSize / 2), 0.0));
    }

    generateStatusInfo(view, osg::Vec2d(ea.getXnormalized(), ea.getYnormalized()));

    //generateTextTip(view, osg::Vec2d(ea.getXnormalized(), ea.getYnormalized()));

    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
StatusBar::StatusBar(void)
    : m_strTextFont("SIMSUN.TTC")
{
    m_clrTextColor.m_fltR = 0.4f;
    m_clrTextColor.m_fltG = 1.0f;
    m_clrTextColor.m_fltB = 0.3f;
    m_clrTextColor.m_fltA = 1.0f;

    m_clrPaneColor.m_fltR = 0.2f;
    m_clrPaneColor.m_fltG = 0.2f;
    m_clrPaneColor.m_fltB = 0.3f;
    m_clrPaneColor.m_fltA = 0.6f;

    m_bCreated    = false;
    m_bVisible    = true;
    m_nBarWidth   = 20u;
    m_dblTextSize = 18;
}

StatusBar::~StatusBar(void)
{
}

bool StatusBar::initialize(void)
{
    if(m_bCreated)  return true;

    m_pHudCamera = new osg::Camera;
    m_pHudCamera->setCullingActive(false);
    m_pHudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
    m_pHudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pHudCamera->setViewMatrix(osg::Matrix::identity());
    m_pHudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    m_pHudCamera->setRenderOrder(osg::Camera::POST_RENDER);
    m_pHudCamera->setAllowEventFocus(false);
    m_pHudCamera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    m_pHudCamera->setViewport(0, 0, 1, 1);

    m_pHudCamera->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL));

    osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile(m_strTextFont);

    const osg::Vec4  clrBackdrop(0.0, 0.0, 0.0, 1.0);

    osg::ref_ptr<osg::Geode>    pFPSGeode  = new osg::Geode;
    pFPSGeode->setCullingActive(false);
    m_pFPSTextGeom = new osgText::Text;
    m_pFPSTextGeom->setDataVariance(osg::Object::DYNAMIC);
    m_pFPSTextGeom->setFont(pFont.get());
    m_pFPSTextGeom->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    m_pFPSTextGeom->setColor(osg::Vec4f(m_clrTextColor.m_fltR, m_clrTextColor.m_fltG, m_clrTextColor.m_fltB, m_clrTextColor.m_fltA));
    m_pFPSTextGeom->setCharacterSize((float)m_dblTextSize);
    m_pFPSTextGeom->setLayout(osgText::Text::LEFT_TO_RIGHT);
    m_pFPSTextGeom->setAlignment(osgText::Text::LEFT_CENTER);
    m_pFPSTextGeom->setBackdropType(osgText::Text::OUTLINE);
    m_pFPSTextGeom->setBackdropColor(clrBackdrop);
    m_pFPSTextGeom->setBackdropType(osgText::Text::OUTLINE);
    m_pFPSTextGeom->setBackdropOffset(0.2f, 0.2f);
    pFPSGeode->addDrawable(m_pFPSTextGeom.get());

    osg::ref_ptr<osg::Geode>    pInfoGeode  = new osg::Geode;
    pInfoGeode->setCullingActive(false);
    m_pInfoTextGeom = new osgText::Text;
    m_pFPSTextGeom->setDataVariance(osg::Object::DYNAMIC);
    m_pInfoTextGeom->setFont(pFont.get());
    m_pInfoTextGeom->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    m_pInfoTextGeom->setColor(osg::Vec4f(m_clrTextColor.m_fltR, m_clrTextColor.m_fltG, m_clrTextColor.m_fltB, m_clrTextColor.m_fltA));
    m_pInfoTextGeom->setCharacterSize((float)m_dblTextSize);
    m_pInfoTextGeom->setLayout(osgText::Text::LEFT_TO_RIGHT);
    m_pInfoTextGeom->setAlignment(osgText::Text::RIGHT_CENTER);
    m_pInfoTextGeom->setBackdropType(osgText::Text::OUTLINE);
    m_pInfoTextGeom->setBackdropColor(clrBackdrop);
    m_pInfoTextGeom->setBackdropType(osgText::Text::OUTLINE);
    m_pInfoTextGeom->setBackdropOffset(0.2f, 0.2f);
    pInfoGeode->addDrawable(m_pInfoTextGeom.get());
    pInfoGeode->getOrCreateStateSet()->setRenderBinDetails(-9999, "RenderBin");

    osg::ref_ptr<osg::Geode>    pPanGode    = new osg::Geode;
    pPanGode->setCullingActive(false);
    m_pPaneGeom = new osg::Geometry;
    m_pPaneGeom->setUseDisplayList(false);
    osg::ref_ptr<osg::Vec3Array>    pPaneVtx = new osg::Vec3Array;
    pPaneVtx->push_back(osg::Vec3(0.0f, 0.0f, -0.5f));
    pPaneVtx->push_back(osg::Vec3(1.0f, 0.0f, -0.5f));
    pPaneVtx->push_back(osg::Vec3(1.0f, 1.0f, -0.5f));
    pPaneVtx->push_back(osg::Vec3(0.0f, 1.0f, -0.5f));
    m_pPaneGeom->setVertexArray(pPaneVtx.get());
    m_pPaneGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::StateSet *pPaneStateset = m_pPaneGeom->getOrCreateStateSet();
    pPaneStateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    pPaneStateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    const osg::Vec4f clrPane(m_clrPaneColor.m_fltR, m_clrPaneColor.m_fltG, m_clrPaneColor.m_fltB, m_clrPaneColor.m_fltA);
    osg::Vec4Array *pColor = new osg::Vec4Array(1, &clrPane);
    m_pPaneGeom->setColorArray(pColor);
    m_pPaneGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    pPanGode->addDrawable(m_pPaneGeom.get());
    pPanGode->getOrCreateStateSet()->setRenderBinDetails(-9998, "RenderBin");

    m_pHudCamera->addChild(pPanGode.get());
    m_pHudCamera->addChild(pInfoGeode.get());
    m_pHudCamera->addChild(pFPSGeode.get());
    //if(pLogoGoede.valid())
    //{
    //    m_pHudCamera->addChild(pLogoGoede);
    //}

    addChild(m_pHudCamera.get());
    setNodeMask(m_bVisible ? 0xFFFFFFFF : 0x00000000);
    setBarWidth(m_nBarWidth);

    m_pEventHandler = new StatusBarEventHandler(this);
    m_bCreated = true;

    m_pHudCamera->addEventCallback(m_pEventHandler.get());
    return true;
}


void StatusBar::setTextColor(const cmm::FloatColor &color)
{
    if(osg::equivalent(m_clrTextColor.m_fltR, color.m_fltR)
        && osg::equivalent(m_clrTextColor.m_fltG, color.m_fltG)
        && osg::equivalent(m_clrTextColor.m_fltB, color.m_fltB))
    {
        return;
    }
    m_clrTextColor = color;

    if(m_bCreated)
    {
        const osg::Vec4f color(m_clrTextColor.m_fltR, m_clrTextColor.m_fltG, m_clrTextColor.m_fltB, m_clrTextColor.m_fltA);
        m_pFPSTextGeom->setColor(color);
        m_pInfoTextGeom->setColor(color);
    }
}


const cmm::FloatColor &StatusBar::getTextColor(void) const
{
    return m_clrTextColor;
}


void StatusBar::setPaneColor(const cmm::FloatColor &color)
{
    if(osg::equivalent(m_clrPaneColor.m_fltR, color.m_fltR)
        && osg::equivalent(m_clrPaneColor.m_fltG, color.m_fltG)
        && osg::equivalent(m_clrPaneColor.m_fltB, color.m_fltB)
        && osg::equivalent(m_clrPaneColor.m_fltA, color.m_fltA))
    {
        return;
    }
    m_clrPaneColor = color;

    if(m_bCreated)
    {
        osg::Vec4Array *pColorArray = dynamic_cast<osg::Vec4Array *>(m_pPaneGeom->getColorArray());
        osg::Vec4 &color = pColorArray->at(0u);
        color.set(m_clrPaneColor.m_fltR, m_clrPaneColor.m_fltG, m_clrPaneColor.m_fltB, m_clrPaneColor.m_fltA);
    }
}


const cmm::FloatColor& StatusBar::getPaneColor() const
{
    return m_clrPaneColor;
}


void StatusBar::setDemHelper(IDemHelper *pDemHelper)
{
    m_pDemHelper = pDemHelper;
}

void StatusBar::setFPS(double dblFPS)
{
    if(!m_bCreated) return;

    std::stringstream ss;
    ss << "帧率: " << dblFPS;

    const std::wstring strStringW = cmm::ANSIToUnicode(ss.str());
    m_pFPSTextGeom->setText(osgText::String(strStringW.c_str()));
}


double StatusBar::getCameraHieght()
{
    return m_dblCameraHeight;
}


void StatusBar::setInfo(const osg::Vec3d &ptMousePos, double dblCameraHeight)
{
    if(!m_bCreated) return;

    m_dblCameraHeight = dblCameraHeight;

    std::stringstream ss;
    if(!ptMousePos.isNaN())
    {
        ss << "鼠标位置[" << (ptMousePos.y() >= 0.0 ? "北纬: " : "南纬: ");

        // 纬度
        const double dblD1 = fabs(ptMousePos.y());
        const double dblM1 = (dblD1 - floor(dblD1)) * 60.0;
        const double dblS1 = (dblM1 - floor(dblM1)) * 60.0;
        ss << (unsigned)floor(dblD1) << "°" << (unsigned)floor(dblM1) << "′" << (unsigned)floor(dblS1) << "″";
		//ss << std::setprecision(16) << osg::DegreesToRadians(ptMousePos.y());
        ss << "  ";

        // 经度
        ss << (ptMousePos.x() >= 0.0 ? "东经: " : "西经: ");
        const double dblD2 = fabs(ptMousePos.x());
        const double dblM2 = (dblD2 - floor(dblD2)) * 60.0;
        const double dblS2 = (dblM2 - floor(dblM2)) * 60.0;
        ss << (unsigned)floor(dblD2) << "°" << (unsigned)floor(dblM2) << "′" << (unsigned)floor(dblS2) << "″";
		//ss << std::setprecision(16) << osg::DegreesToRadians(ptMousePos.x());
        ss << "  ";

        // 海拔
        ss << "高程: " << std::setprecision(2) << std::setiosflags(std::ios::fixed) << ptMousePos.z() << "米]";
        ss << "  ";
    }

    ss << "视角海拔高度[";

    bool bKM = (dblCameraHeight >= 1000);
    dblCameraHeight = bKM ? dblCameraHeight / 1000 : dblCameraHeight;

    // 海拔
    ss << std::setprecision(2) << std::setiosflags(std::ios::fixed) << dblCameraHeight;
    if(bKM)
    {
        ss << "公里]";
    }
    else
    {
        ss << "米]";
    }

    const std::wstring strStringW = cmm::ANSIToUnicode(ss.str());
    m_pInfoTextGeom->setText(osgText::String(strStringW.c_str()));
}


void StatusBar::setUserString(const std::string &strUserString)
{
}


void StatusBar::setVisible(bool bVisible)
{
    if(bVisible == m_bVisible)  return;
    m_bVisible = bVisible;

    if(m_bCreated)
    {
        setNodeMask(m_bVisible ? 0xFFFFFFFF : 0x00000000);
    }
}


void StatusBar::setBarWidth(unsigned nWidth)
{
    nWidth = osg::clampAbove(nWidth, 1u);
    if(m_nBarWidth == nWidth)
    {
        return;
    }

    m_nBarWidth = nWidth;
}


void StatusBar::setTextFont(const std::string &strTextFont)
{
    std::string strFont = strTextFont;
    std::transform(strTextFont.begin(), strTextFont.end(), strFont.begin(), ::toupper);
    if(strFont == m_strTextFont)
    {
        return;
    }

    m_strTextFont = strFont;
    if(m_bCreated)
    {
        osg::ref_ptr<osgText::Font> pNewFont = osgText::readFontFile(m_strTextFont);

        m_pFPSTextGeom->setFont(pNewFont.get());

        m_pInfoTextGeom->setFont(pNewFont.get());
    }
}


const std::string &StatusBar::getTextFont(void) const
{
    return m_strTextFont;
}


void StatusBar::setTextSize(double dblSize)
{
    dblSize = osg::clampAbove(dblSize, 1.0);
    if(osg::equivalent(dblSize, m_dblTextSize))
    {
        return;
    }

    m_dblTextSize = dblSize;

    if(m_bCreated)
    {
        m_pFPSTextGeom->setCharacterSize((float)m_dblTextSize);

        m_pInfoTextGeom->setCharacterSize((float)m_dblTextSize);
    }
}


double StatusBar::getTextSize(void) const
{
    return m_dblTextSize;
}

