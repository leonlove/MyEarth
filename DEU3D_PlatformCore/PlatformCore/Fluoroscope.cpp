#include "FluoroScope.h"
#include <assert.h>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/Texture2D>
#include <osg/Camera>
#include <osgViewer/View>
#include <osg/CullFace>
#include <osg/Depth>
#include <osg/CoordinateSystemNode>

#include "StatusBar.h"

class FluoroScopeEventHandler : public osgGA::GUIEventHandler
{
public:
    explicit FluoroScopeEventHandler(FluoroScope *pFluoroScope)
    {
        m_pFluoroScope          = pFluoroScope;
        m_pShadowOfViewCamera   = new osg::Camera;
    }

    void clear()
    {
        if(m_pCurrentCamera != NULL)
        {
            m_pCurrentCamera->getOrCreateStateSet()->removeAttribute(m_pFluoroScope->m_pUnPenetrableAttribute);
        }
    }

protected:
    virtual ~FluoroScopeEventHandler(void)
    {
        m_pShadowOfViewCamera = NULL;
    }

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
    {
        osgViewer::View &view = dynamic_cast<osgViewer::View &>(aa);
        m_pCurrentCamera = view.getCamera();

        if(ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
        {
            const GLbitfield nMask = m_pCurrentCamera->getClearMask();
            bool  bChangedViewCamera = false;
            if((nMask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT)
            {
                // 视图相机不能清除了模板缓存
                m_pCurrentCamera->setClearMask(nMask & ~GL_STENCIL_BUFFER_BIT);
                bChangedViewCamera = true;
            }

            osg::StateSet *pViewStateSet = m_pCurrentCamera->getOrCreateStateSet();
            osg::Stencil *pUnPenetrableAttribute = dynamic_cast<osg::Stencil *>(pViewStateSet->getAttribute(osg::StateAttribute::STENCIL));

            if(pUnPenetrableAttribute != m_pFluoroScope->m_pUnPenetrableAttribute)
            {
                // 给视图相机挂接“通不过透镜”的属性
                pViewStateSet->setAttributeAndModes(m_pFluoroScope->m_pUnPenetrableAttribute);
                bChangedViewCamera = true;
            }

            osg::Camera *pHudCamera = m_pFluoroScope->m_pHUDCamera;
            osg::Camera *pRTSCamera = m_pFluoroScope->m_pRTSCamera;

            const osg::Viewport *pViewport = m_pCurrentCamera->getViewport();

            const osg::Viewport *pRTSViewport = pRTSCamera->getViewport();
            if(pRTSViewport == NULL || pRTSViewport->width() != pViewport->width() || pRTSViewport->height() != pViewport->height())
            {
                m_pFluoroScope->m_bDirty = true;
                m_pFluoroScope->setFluoroScopeSize(0.5 * sqrt(pViewport->width() * pViewport->width() + pViewport->height() * pViewport->height()));
            }

            pHudCamera->setViewport(pViewport->x(), pViewport->y(), pViewport->width(), pViewport->height());
            pRTSCamera->setViewport(pViewport->x(), pViewport->y(), pViewport->width(), pViewport->height());
            if(m_pFluoroScope->m_bFlatMode)
            {
                pHudCamera->setViewMatrix(osg::Matrix::identity());
                pRTSCamera->setViewMatrix(osg::Matrix::identity());
                pHudCamera->setProjectionMatrixAsOrtho2D(-pViewport->width() / 2, pViewport->width() / 2, -pViewport->height() / 2, pViewport->height() / 2);
                pRTSCamera->setProjectionMatrixAsOrtho2D(-pViewport->width() / 2, pViewport->width() / 2, -pViewport->height() / 2, pViewport->height() / 2);
            }
            else
            {
                pHudCamera->setViewMatrix(m_pCurrentCamera->getViewMatrix());
                pRTSCamera->setViewMatrix(m_pCurrentCamera->getViewMatrix());
                pHudCamera->setProjectionMatrixAsPerspective(45.0, pViewport->width() / pViewport->height(), 1.0, 1e10);
                pRTSCamera->setProjectionMatrixAsPerspective(45.0, pViewport->width() / pViewport->height(), 1.0, 1e10);
                //pHudCamera->setProjectionMatrix(m_pCurrentCamera->getProjectionMatrix());
                //pRTSCamera->setProjectionMatrix(m_pCurrentCamera->getProjectionMatrix());
            }

            if(m_pFluoroScope->m_bDirty)
            {
                m_pFluoroScope->createFluoroScopeFrame();
                m_pFluoroScope->createFluoroScopeLens();
                m_pFluoroScope->m_bDirty = false;
            }

            return false;
        }

        return false;
    }

protected:
    FluoroScope                 *m_pFluoroScope;
    osg::ref_ptr<osg::Camera>   m_pShadowOfViewCamera;
    osg::ref_ptr<osg::Camera>   m_pCurrentCamera;
};

FluoroScope::FluoroScope(void)
{
    m_clrFrameColor.set(0.2f, 1.0f, 0.2f, 1.0f);
    m_dblFluoroScopeSize    = 100.0f;
    m_dblFrameWidth         = 8.0f;
    m_nPolygonSideCount     = 360u;

    m_bFlatMode             = true;
    m_vecTranslate.set(0.0f, 0.0f, 0.0f);
    m_pStatusBar            = NULL;
    m_bEnable               = false;
}


FluoroScope::~FluoroScope(void)
{
    unInit();
}

void FluoroScope::createUnPenetrableAttribute(void)
{
    if(m_pUnPenetrableAttribute.valid())
    {
        return;
    }

    m_pUnPenetrableAttribute = new osg::Stencil;
    m_pUnPenetrableAttribute->setFunction(osg::Stencil::NOTEQUAL, 0x1, ~0x0u);
    m_pUnPenetrableAttribute->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
}

void FluoroScope::createPenetrableAttribute(void)
{
    if(m_pPenetrableAttribute.valid())
    {
        return;
    }

    m_pPenetrableAttribute = new osg::Stencil;
    m_pPenetrableAttribute->setFunction(osg::Stencil::ALWAYS, 0x1, ~0x0u);
    m_pPenetrableAttribute->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
}

bool FluoroScope::init(void)
{
    createPenetrableAttribute();
    createUnPenetrableAttribute();

    m_pFluoroScopeEventHandler = new FluoroScopeEventHandler(this);
    addEventCallback(m_pFluoroScopeEventHandler.get());

    // 1、创建HUD相机，用来绘制透视镜的边框
    createHudCamera();
    m_pHUDCamera->getOrCreateStateSet()->setAttributeAndModes(m_pPenetrableAttribute);
    addChild(m_pHUDCamera.get());

    // 2、创建一个渲染到模板缓存的RTS相机，用来将透视镜的镜片绘制到模板缓存
    createRtsCamera();
    addChild(m_pRTSCamera.get());

    // 3、设置自身的StateSet，关闭光照、深度测试、多边形模式
    osg::StateSet *pStateSet = getOrCreateStateSet();

    osg::ref_ptr<osg::PolygonMode> pPolygonMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
    pStateSet->setAttributeAndModes(pPolygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::BACK);
    pStateSet->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    setEnable(m_bEnable);

    m_bDirty = false;

    return true;
}

void FluoroScope::unInit(void)
{
    removeChildren(0u, getNumChildren());
}

void FluoroScope::setPosition(const osg::Vec3 &vecPos)
{
    m_vecTranslate = vecPos;
    if(!m_bFlatMode)
    {
        m_bDirty = true;
    }
}

void FluoroScope::createHudCamera(void)
{
    m_pHUDCamera = new osg::Camera;
    m_pHUDCamera->setProjectionMatrix(osg::Matrix::ortho2D(-1.0, 1.0, -1.0, 1.0));
    m_pHUDCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pHUDCamera->setViewMatrix(osg::Matrix::identity());
    m_pHUDCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    m_pHUDCamera->setRenderOrder(osg::Camera::POST_RENDER);
    m_pHUDCamera->setAllowEventFocus(false);
    m_pHUDCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);

    createFluoroScopeFrame();
    m_pHUDCamera->addChild(m_pFluoroScopeFrameNode.get());
}

void FluoroScope::createRtsCamera(void)
{
    m_pRTSCamera = new osg::Camera;
    m_pRTSCamera->setProjectionMatrix(osg::Matrix::ortho2D(-1.0, 1.0, -1.0, 1.0));
    m_pRTSCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pRTSCamera->setViewMatrix(osg::Matrix::identity());
    m_pRTSCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    osg::Vec4 clr;
    clr.set(1.0, 1.0, 1.0, 1.0);
    m_pRTSCamera->setClearColor(clr);
    m_pRTSCamera->setClearStencil(0);
    m_pRTSCamera->setRenderOrder(osg::Camera::PRE_RENDER, 0);
    m_pRTSCamera->setAllowEventFocus(false);
    m_pRTSCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);

    createFluoroScopeLens();

    m_pRTSCamera->addChild(m_pFluoroScopeLensNode.get());
}

void FluoroScope::createFluoroScopeLens(void)
{
    if(!m_pFluoroScopeLensNode.valid())
    {
        m_pFluoroScopeLensNode = new osg::MatrixTransform;

        osg::ref_ptr<osg::Stencil>  pStencil = new osg::Stencil;
        pStencil->setFunction(osg::Stencil::ALWAYS, 0x1, ~0x0u);
        pStencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

        osg::StateSet *pLensStateSet = m_pFluoroScopeLensNode->getOrCreateStateSet();
        pLensStateSet->setAttributeAndModes(pStencil.get());
    }
    else
    {
        m_pFluoroScopeLensNode->removeChildren(0, m_pFluoroScopeLensNode->getNumChildren());
    }

    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array;
    pVtxArray->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));

    const float    fltTheta = 2.0f * osg::PI / m_nPolygonSideCount;
    float    fltCurTheta = -osg::PI_2 + fltTheta * 0.5;
    for(unsigned i = 0; i <= m_nPolygonSideCount; i++)
    {
        const float x = cos(fltCurTheta) * m_dblFluoroScopeSize;
        const float y = sin(fltCurTheta) * m_dblFluoroScopeSize;

        pVtxArray->push_back(osg::Vec3(x, y, 0.0f));

        fltCurTheta += fltTheta;
    }

    osg::ref_ptr<osg::Geometry>        pGeom = new osg::Geometry;
    pGeom->setVertexArray(pVtxArray.get());
    pGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0u, pVtxArray->size()));

    osg::ref_ptr<osg::Vec4Array>    pColorArray = new osg::Vec4Array;
    //pColorArray->push_back(osg::Vec4(0.8f, 0.8f, 1.0f, 1.0f));
    pColorArray->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pGeom->setColorArray(pColorArray.get());
    pGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    pGeode->addDrawable(pGeom.get());

    m_pFluoroScopeLensNode->setMatrix(calcFluoroScopeMatrix());
    m_pFluoroScopeLensNode->addChild(pGeode.get());

    return;
}

void FluoroScope::createFluoroScopeFrame(void)
{
    if(!m_pFluoroScopeFrameNode.valid())
    {
        m_pFluoroScopeFrameNode = new osg::MatrixTransform;
    }
    else
    {
        m_pFluoroScopeFrameNode->removeChildren(0, m_pFluoroScopeFrameNode->getNumChildren());
    }

    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array>    pVtxColor = new osg::Vec4Array;

    const float    fltTheta = 2.0f * osg::PI / m_nPolygonSideCount;
    float    fltCurTheta = -osg::PI_2 + fltTheta * 0.5;
    for(unsigned i = 0; i <= m_nPolygonSideCount; i++)
    {
        const float fltCos = cos(fltCurTheta);
        const float fltSin = sin(fltCurTheta);

        const float fltR0 = m_dblFluoroScopeSize;
        const float x0 = fltCos * fltR0;
        const float y0 = fltSin * fltR0;
        pVtxArray->push_back(osg::Vec3(x0, y0, 0.0f));

        const float x1 = fltCos * (m_dblFluoroScopeSize + m_dblFrameWidth);
        const float y1 = fltSin * (m_dblFluoroScopeSize + m_dblFrameWidth);
        pVtxArray->push_back(osg::Vec3(x1, y1, 0.0f));

        pVtxColor->push_back(m_clrFrameColor * 0.5);
        pVtxColor->push_back(m_clrFrameColor);

        fltCurTheta += fltTheta;
    }

    osg::ref_ptr<osg::Geometry>        pGeom = new osg::Geometry;
    pGeom->setVertexArray(pVtxArray.get());
    pGeom->setColorArray(pVtxColor.get());
    pGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    pGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0u, pVtxArray->size()));

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    pGeode->addDrawable(pGeom.get());

    m_pFluoroScopeFrameNode->setMatrix(calcFluoroScopeMatrix());
    m_pFluoroScopeFrameNode->addChild(pGeode.get());
    return ;
}

void FluoroScope::setFrameColor(const osg::Vec4 &color)
{
    m_clrFrameColor.set(color.r(), color.g(), color.b(), 1.0f);
    m_bDirty = true;
}

const osg::Vec4 &FluoroScope::getFrameColor(void) const
{
    return m_clrFrameColor;
}

void FluoroScope::setPolygonSideCount(unsigned nCount)
{
    const unsigned nSideCount = osg::clampAbove(nCount, 3u);
    if(m_nPolygonSideCount == nSideCount)
    {
        return;
    }

    m_bDirty = true;
}

unsigned FluoroScope::getPolygonSideCount(void) const
{
    return m_nPolygonSideCount;
}

void FluoroScope::setFluoroScopeSize(double dblSize)
{
    m_dblFluoroScopeSize = osg::clampAbove(dblSize, 1.0);
    m_bDirty = true;
}

double FluoroScope::getFluoroScopeSize(void) const
{
    return m_dblFluoroScopeSize;
}

void FluoroScope::setFrameWidth(double dblWidth)
{
    m_dblFrameWidth = osg::clampAbove(dblWidth, 0.0);

    m_bDirty = true;
}

double FluoroScope::getFrameWidth(void) const
{
    return m_dblFrameWidth;
}

void FluoroScope::setFlatMode(bool bFlat)
{
    if(m_bFlatMode == bFlat)    return;

    m_bFlatMode = bFlat;
    m_bDirty = true;
}

void FluoroScope::addToWhiteList(osg::Node *pNode)
{
    if(isNodeInFluoroScope(pNode))
    {
        return;
    }

    osg::StateSet *pStateSet = pNode->getOrCreateStateSet();
    pStateSet->setAttributeAndModes(m_pPenetrableAttribute.get());
    m_setBlackListNodes.insert(pNode);
}

void FluoroScope::removeFromBlackList(osg::Node *pNode)
{
    if(isNodeInFluoroScope(pNode))
    {
        osg::StateSet *pStateSet = pNode->getOrCreateStateSet();
        pStateSet->removeAttribute(m_pPenetrableAttribute);
        m_setBlackListNodes.erase(pNode);
    }
}

void FluoroScope::clearPenetrableNode(void)
{
    std::set< osg::observer_ptr<osg::Node> >::iterator itorNode = m_setBlackListNodes.begin();
    for(; itorNode != m_setBlackListNodes.end(); ++itorNode)
    {
        osg::Node *pNode = itorNode->get();
        osg::StateSet *pStateSet = pNode->getOrCreateStateSet();
        pStateSet->removeAttribute(m_pPenetrableAttribute);
    }
    m_setBlackListNodes.clear();
}

bool FluoroScope::isNodeInFluoroScope(osg::Node *pNode) const
{
    std::set< osg::observer_ptr<osg::Node> >::const_iterator itorFind = m_setBlackListNodes.find(pNode);
    return (itorFind != m_setBlackListNodes.end());
}

osg::Matrix FluoroScope::calcFluoroScopeMatrix(void) const
{
    osg::Matrixd mtx;
    if(m_bFlatMode)
    {
        mtx.identity();
    }
    else
    {
        osg::ref_ptr<osg::EllipsoidModel> pEllipsoidModel = new osg::EllipsoidModel;
        pEllipsoidModel->computeLocalToWorldTransformFromXYZ(m_vecTranslate._v[0], m_vecTranslate._v[1], m_vecTranslate._v[2], mtx);
    }

    return mtx;
}

void FluoroScope::setEnable(bool bEnable)
{
    //m_pLayouter->removeAllCameras();
    if(!bEnable)
    {
        m_pFluoroScopeEventHandler->clear();
    }
    setNodeMask(bEnable ? 0xFFFFFFFF : 0x00000000);
    m_bEnable = bEnable;
}


void FluoroScope::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if(!pCullVisitor)
    {
        osg::Group::traverse(nv);
        return;
    }

    if(!m_pStatusBar)
    {
        return;
    }

    osg::Camera *pCamera = pCullVisitor->getCurrentCamera();
    double dblCameraHeight = m_pStatusBar->getCameraHieght();
    if(dblCameraHeight > 2000.0)
    {
        m_pFluoroScopeEventHandler->clear();
        return;
    }
    else
    {
        osg::Group::traverse(nv);
        return;
    }
}


