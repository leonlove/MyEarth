#include "SubGround.h"

#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/NodeCallback>
#include <osgUtil/CullVisitor>
#include <osgViewer/View>
#include <osg/PolygonMode>
#include <osg/Material>

#include <osgUtil/Radial.h>
#include <osg/TexMat>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <common/deuMath.h>
#include "StatusBar.h"
const static double gs_dblMaxEyeHeightBlend = 6000.0;


SubGround::SubGround(void)
         : m_pPlanetNode(NULL),
           m_dblSubGroundDepth(800.0),
           m_nUpdateSpeed(1u),
           m_dblTerrainOpacity(1.0),
           m_bsBoundingSphere(osg::Vec3d(0.0, 0.0, 0.0), 1.0)
{
    m_clrGroundColor.m_fltR = 0.25;
    m_clrGroundColor.m_fltG = 0.25;
    m_clrGroundColor.m_fltB = 0.75;
    m_clrGroundColor.m_fltA = 1.0;
}


SubGround::~SubGround(void)
{
}


bool SubGround::initialize(osg::Node *pPlanetNode)
{
    if(!pPlanetNode)    return false;
    m_pPlanetNode   = pPlanetNode;

    m_pSubGroundGeode = new osg::Geode;
    m_pSubGroundGeode->setName("SubGround Geode");

    m_pBoardGeometry = createBoardGeom();
    m_pSubGroundGeode->addDrawable(m_pBoardGeometry.get());

    m_pSubGroundTrans = new osg::MatrixTransform;
    m_pSubGroundTrans->setName("SubGround MatrixTransfrom");
    m_pSubGroundTrans->addChild(m_pSubGroundGeode.get());

    addChild(m_pSubGroundTrans.get());

    setName("SubGround");
    setNodeMask(0xFF000000);

    m_bsBoundingSphere = pPlanetNode->getBound();
    return true;
}


void SubGround::setGroundColor(const cmm::FloatColor &color)
{
    m_clrGroundColor = color;
    m_clrGroundColor.m_fltA = 1.0f;

    if(m_pSubGroundTexture.valid())
    {
        unsigned char *pPixel = m_pSubGroundImage->data();

        cmm::FloatColor color1;
        color1.m_fltR = m_clrGroundColor.m_fltR * 0.5 + 0.5;
        color1.m_fltG = m_clrGroundColor.m_fltG * 0.5 + 0.5;
        color1.m_fltB = m_clrGroundColor.m_fltB * 0.5 + 0.5;

        cmm::FloatColor color2;
        color2.m_fltR = m_clrGroundColor.m_fltR * 0.25 + 0.75;
        color2.m_fltG = m_clrGroundColor.m_fltG * 0.25 + 0.75;
        color2.m_fltB = m_clrGroundColor.m_fltB * 0.25 + 0.75;

        const unsigned char r1 = (unsigned char)(color1.m_fltR * 255.0f);
        const unsigned char g1 = (unsigned char)(color1.m_fltG * 255.0f);
        const unsigned char b1 = (unsigned char)(color1.m_fltB * 255.0f);

        const unsigned char r2 = (unsigned char)(color2.m_fltR * 255.0f);
        const unsigned char g2 = (unsigned char)(color2.m_fltG * 255.0f);
        const unsigned char b2 = (unsigned char)(color2.m_fltB * 255.0f);

        pPixel[ 0] = /*r1*/132;
        pPixel[ 1] = /*g1*/132;
        pPixel[ 2] = /*b1*/132;

		pPixel[ 4] = /*r2*/132;
		pPixel[ 5] = /*g2*/132;
		pPixel[ 6] = /*b2*/132;

		pPixel[ 8] = /*r2*/132;
		pPixel[ 9] = /*g2*/132;
		pPixel[10] = /*b2*/132;

		pPixel[12] = /*r1*/132;
		pPixel[13] = /*g1*/132;
		pPixel[14] = /*b1*/132;

        m_pSubGroundTexture->dirtyTextureObject();
    }
}


osg::Geometry *SubGround::createBoardGeom(void)
{
    osg::ref_ptr<osg::Geometry>     pGeometry    = new osg::Geometry;
    pGeometry->setUseDisplayList(true);
    pGeometry->setName("SubGround Geometry");

    osg::ref_ptr<osg::Vec3Array>    pVertexArray = new osg::Vec3Array;

    //const float fltWidth = 15000.0f;
    //pVertexArray->push_back(osg::Vec3(-fltWidth, -fltWidth, 0.0f));
    //pVertexArray->push_back(osg::Vec3( fltWidth, -fltWidth, 0.0f));
    //pVertexArray->push_back(osg::Vec3( fltWidth,  fltWidth, 0.0f));
    //pVertexArray->push_back(osg::Vec3(-fltWidth,  fltWidth, 0.0f));
    pVertexArray->push_back(osg::Vec3(-1.0f, -1.0f, 0.0f));
    pVertexArray->push_back(osg::Vec3( 1.0f, -1.0f, 0.0f));
    pVertexArray->push_back(osg::Vec3( 1.0f,  1.0f, 0.0f));
    pVertexArray->push_back(osg::Vec3(-1.0f,  1.0f, 0.0f));
    pGeometry->setVertexArray(pVertexArray.get());

    osg::ref_ptr<osg::Vec2Array>    pTexArray = new osg::Vec2Array;
    //const float fltTexWidth = fltWidth / 100.0f;
    //pTexArray->push_back(osg::Vec2(       0.0f,        0.0f));
    //pTexArray->push_back(osg::Vec2(fltTexWidth,        0.0f));
    //pTexArray->push_back(osg::Vec2(fltTexWidth, fltTexWidth));
    //pTexArray->push_back(osg::Vec2(       0.0f, fltTexWidth));
    pTexArray->push_back(osg::Vec2(0.0f, 0.0f));
    pTexArray->push_back(osg::Vec2(1.0f, 0.0f));
    pTexArray->push_back(osg::Vec2(1.0f, 1.0f));
    pTexArray->push_back(osg::Vec2(0.0f, 1.0f));
    pGeometry->setTexCoordArray(0u, pTexArray.get());

    osg::ref_ptr<osg::Vec4Array>    pColorArray = new osg::Vec4Array;
    pColorArray->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    m_pSubGroundImage = new osg::Image;
    m_pSubGroundImage->allocateImage(2, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE);

    m_pSubGroundTexture = new osg::Texture2D;
    m_pSubGroundTexture->setImage(m_pSubGroundImage.get());
    m_pSubGroundTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    m_pSubGroundTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    m_pSubGroundTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_pSubGroundTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);

    setGroundColor(m_clrGroundColor);

    osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();
    pStateSet->setTextureAttributeAndModes(0, m_pSubGroundTexture.get());
    osg::ref_ptr<osg::TexMat> pTexMat = new osg::TexMat;
    osg::Matrix mtx = osg::Matrix::scale(150.0f, 150.0f, 1.0f);
    pTexMat->setMatrix(mtx);
    pStateSet->setTextureAttributeAndModes(0, pTexMat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
    pStateSet->setMode(GL_SMOOTH, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

    osg::ref_ptr<osg::PolygonMode>  pPolyMode = new osg::PolygonMode;
    pPolyMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL);
    pStateSet->setAttribute(pPolyMode.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    pGeometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, pVertexArray->size()));

    //pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, pVertexArray->size()));
    return pGeometry.release();
}


osg::BoundingSphere SubGround::computeBound(void) const
{
    return m_bsBoundingSphere;
}


void SubGround::traverse(osg::NodeVisitor &nv)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if(!pCullVisitor)
    {
        osg::Group::traverse(nv);
        return;
    }

    static bool bShouldVisible = false;
    class GroundShower
    {
    public:
        explicit GroundShower(SubGround *pThis, const bool &bVisible, osg::NodeVisitor &visitor)
            : m_pThis(pThis), m_bVisible(bVisible), m_dblEyeActHeight(FLT_MAX), m_NodeVisitor(visitor)
        {
        }
        virtual ~GroundShower(void)
        {
            if(m_bVisible)
            {
                m_pThis->applyTerrainTransparency(m_pThis->m_dblTerrainOpacity, m_dblEyeActHeight);
                m_pThis->osg::Group::traverse(m_NodeVisitor);
            }
            else
            {
                m_pThis->applyTerrainTransparency(1.0);
            }
        }
        SubGround          *m_pThis;
        const bool         &m_bVisible;
        osg::NodeVisitor   &m_NodeVisitor;
        double              m_dblEyeActHeight;
    }__shower(this, bShouldVisible, nv);

    const unsigned nFrameNumber = nv.getFrameStamp()->getFrameNumber();
    if(nFrameNumber % m_nUpdateSpeed != 0u)
    {
        return;
    }

    static const osg::Vec3d ptSceneCenter(0.0, 0.0, 0.0);

    osg::Camera *pCamera = pCullVisitor->getCurrentCamera();
    osg::Vec3d  ptEye, ptCenter, vecUp;
    pCamera->getViewMatrixAsLookAt(ptEye, ptCenter, vecUp);

    osg::Vec3d llh;
    osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(ptEye[0], ptEye[1], ptEye[2], llh[1], llh[0], llh[2]);

    //static const double dblLimitedHeight = osg::EllipsoidModel::instance()->getRadius() + 3000.0;
    //if(ptEye.length() > dblLimitedHeight)
    if(llh[2] > gs_dblMaxEyeHeightBlend)
    {
        bShouldVisible = false;
        return;
    }

    osg::Vec3d  vecCenter2Eye = ptEye - ptSceneCenter;
    if(vecCenter2Eye.normalize() < FLT_EPSILON)
    {
        return;
    }
    if(vecCenter2Eye.isNaN())
    {
        return;
    }

    const osgUtil::Radial3 ray(ptSceneCenter, vecCenter2Eye);
    osg::Vec3d ptHitTest;
    if(!hitPlanetScene(ray, ptHitTest))
    {
        return;
    }
    osg::Vec3d vecEye2Hit = ptHitTest - ptEye;
    const double dblLen = vecEye2Hit.normalize();
    bShouldVisible = (dblLen < gs_dblMaxEyeHeightBlend);

    __shower.m_dblEyeActHeight = dblLen;

    const osg::Vec3d ptGroundPos = ptHitTest - vecCenter2Eye * m_dblSubGroundDepth;

    const osg::Quat    quatRotation(osg::Vec3d(0.0, 0.0, 1.0), vecCenter2Eye);
    const osg::Matrixd mtxScale = osg::Matrixd::scale(15000.0, 15000.0, 1.0);
    const osg::Matrixd mtxRotation(quatRotation);
    const osg::Matrixd mtxTranslation = osg::Matrixd::translate(ptGroundPos);
    const osg::Matrixd mtx = mtxScale * mtxRotation * mtxTranslation;
    m_pSubGroundTrans->setMatrix(mtx);
}


bool SubGround::hitPlanetScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest) const
{
    const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
    osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e9));
    osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
    pVisitor->setTraversalMask(0x00FFFFFF);
    m_pPlanetNode->accept(*pVisitor);

    if(!pPicker->containsIntersections())
    {
        return false;
    }

    const osgUtil::LineSegmentIntersector::Intersections &inters = pPicker->getIntersections();
    const osgUtil::LineSegmentIntersector::Intersection &inter = *inters.begin();
    ptHitTest = inter.getWorldIntersectPoint();
    return true;
}


void SubGround::applyTerrainTransparency(double dblOpacity, double dblEyeHeight)
{
    if(!m_pPlanetNode.valid()) return;

    if(dblEyeHeight < gs_dblMaxEyeHeightBlend)
    {
        const double dblBias  = osg::clampAbove(gs_dblMaxEyeHeightBlend - dblEyeHeight, 0.0);
        const double dblRatio = 1.0 - dblBias / gs_dblMaxEyeHeightBlend;
        if(dblOpacity < dblRatio)
        {
            dblOpacity = dblRatio;
        }
    }

    static double dblLastOpacity = 0.0;
    if(cmm::math::floatEqual(dblLastOpacity, dblOpacity))
    {
        return;
    }

    dblLastOpacity = dblOpacity;

    osg::StateSet *pStateSet =  m_pPlanetNode->getOrCreateStateSet();
    osg::Material *pMaterial = dynamic_cast<osg::Material *>(pStateSet->getAttribute(osg::StateAttribute::MATERIAL));
    if(dblOpacity < 0.98)
    {
        if(pMaterial == NULL)
        {
            pMaterial = new osg::Material;

            static const osg::Vec4 clrDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
            static const osg::Vec4 clrAmbient(0.8f, 0.8f, 0.8f, 1.0f);
            static const osg::Vec4 clrSpecular(0.8f, 0.8f, 0.8f, 1.0f);

            pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, clrDiffuse);
            pMaterial->setAmbient(osg::Material::FRONT_AND_BACK, clrAmbient);
            pMaterial->setSpecular(osg::Material::FRONT_AND_BACK, clrSpecular);
            pStateSet->setAttribute(pMaterial);
        }

        pMaterial->setTransparency(osg::Material::FRONT_AND_BACK, 1.0 - dblOpacity);
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    else
    {
        if(pMaterial)
        {
            pStateSet->removeAttribute(pMaterial);
        }
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);
        pStateSet->setRenderingHint(osg::StateSet::DEFAULT_BIN);
    }
}


void SubGround::setTerrainOpacity(double dblOpacity)
{
    m_dblTerrainOpacity = osg::clampBetween(dblOpacity, 0.0, 1.0);
}


double SubGround::getTerrainOpacity(void) const
{
    return m_dblTerrainOpacity;
}

