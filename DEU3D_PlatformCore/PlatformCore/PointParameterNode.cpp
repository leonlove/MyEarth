#include "PointParameterNode.h"
#include "PipeConnectorBuilder.h"
#include "HoleInfo.h"
#include <ParameterSys/IPointParameter.h>
#include <osg/CoordinateSystemNode>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include <osg/PagedLOD>
#include <osg/Point>
#include <osg/Billboard>
#include <osg/CullFace>
#include <osg/Material>
#include <osg/LineWidth>
#include <osgUtil/Tessellator>

#include <ParameterSys/IHole.h>
#include <IDProvider/Definer.h>

#include "FileReadInterceptor.h"
#include "Registry.h"
#include "ParmRectifyThreadPool.h"
#include "BubbleTextBuilder.h"

using namespace osg;
/*
class ScreenSizeImage: public osgText::Text
{
public:
    ScreenSizeImage(){_tex = new osg::Texture2D();}
    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        drawMyImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }
    virtual osg::BoundingBox computeBound() const
    {
        _textBB.set(-0.5 * _size.x(), -0.5 * _size.y(), 0, 0.5 * _size.x(), 0.5 * _size.x(), 0);
        return osgText::TextBase::computeBound();
    }
    void    setImage(osg::Image *pImg){_tex->setImage(pImg);}
    void    setSize(double w, double h){_size.set(w,h);}

protected:
    osg::ref_ptr<osg::Texture2D> _tex;
    osg::Vec2d                   _size;

    void drawMyImplementation(osg::State& state, const osg::Vec4& colorMultiplier) const
    {
        unsigned int contextID = state.getContextID();

        state.applyMode(GL_BLEND,true);
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);

    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
    #endif

        if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
        {
            unsigned int frameNumber = state.getFrameStamp()?state.getFrameStamp()->getFrameNumber():0;
            AutoTransformCache& atc = _autoTransformCache[contextID];
            const osg::Matrix& modelview = state.getModelViewMatrix();
            const osg::Matrix& projection = state.getProjectionMatrix();

            osg::Vec3 newTransformedPosition = _position*modelview;

            int width = atc._width;
            int height = atc._height;

            const osg::Viewport* viewport = state.getCurrentViewport();
            if (viewport)
            {
                width = static_cast<int>(viewport->width());
                height = static_cast<int>(viewport->height());
            }

            bool doUpdate = atc._traversalNumber==-1;
            if (atc._traversalNumber>=0)
            {
                if (atc._modelview!=modelview)
                {
                    doUpdate = true;
                }
                else if (width!=atc._width || height!=atc._height)
                {
                    doUpdate = true;
                }
                else if (atc._projection!=projection)
                {
                    doUpdate = true;
                }
            }
        
            atc._traversalNumber = frameNumber;
            atc._width = width;
            atc._height = height;
        
            if (doUpdate)
            {    
                atc._transformedPosition = newTransformedPosition;
                atc._projection = projection;
                atc._modelview = modelview;

                computePositions(contextID);
            }
        
        }
    
    
        // Ensure that the glyph coordinates have been transformed for
        // this context id.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(_mtxTextureGlyphQuadMap);

            if ( !_textureGlyphQuadMap.empty() )
            {
                const GlyphQuads& glyphquad = (_textureGlyphQuadMap.begin())->second;
                if ( glyphquad._transformedCoords[contextID].empty() )
                {
                    computePositions(contextID);
                }
            }
        }

        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        state.Normal(_normal.x(), _normal.y(), _normal.z());
        _textBB.set(-0.5 * _size.x(), -0.5 * _size.y(), 0, 0.5 * _size.x(), 0.5 * _size.x(), 0);

        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
        #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
        #endif


        if (_textBB.valid())
        {
            const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

            osg::Vec3 c00(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c10(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c11(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
            osg::Vec3 c01(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);
            
            osg::ref_ptr<osg::Vec2Array> tc = new osg::Vec2Array;
            
            tc->push_back(osg::Vec2(0.0, 0.0));
            tc->push_back(osg::Vec2(1.0, 0.0));
            tc->push_back(osg::Vec2(1.0, 1.0));
            tc->push_back(osg::Vec2(0.0, 1.0));

            osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
            va->push_back(c00);
            va->push_back(c10);
            va->push_back(c11);
            va->push_back(c01);

            state.applyTextureAttribute(0,_tex.get());
            state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(tc->front()));
            state.disableColorPointer();
            state.setVertexPointer( 3, GL_FLOAT, 0, &(va->front()));
            state.drawQuads(0,tc->size());
        }
    }
};
osg::Drawable *createPolygonSheet(const std::vector<osg::Vec3d> &vecVertices, bool bGenTexCoord);
osg::Drawable *createPrismSide(const std::vector<osg::Vec3d> &vecTopVertices, const std::vector<osg::Vec3d> &vecBottomVertices, bool bGenTexCoord);
osg::Vec3Array *genConeNormal(unsigned nHint, double dblTopRadius, double dblBottomRadius, double dblHeight);
osg::Vec3Array *genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint);
osg::Geometry *createCircleSheet(osg::Vec3Array *pVertices, bool bHasTex);

PointParameterNode::PointParameterNode(param::IParameter *pParameter) : 
    ParameterNode(pParameter)
{
    _name = "PointParameterNode";
    m_bTransedOntoGlobe = true;
    setID(pParameter->getID());
}


PointParameterNode::~PointParameterNode(void)
{
}


bool PointParameterNode::initFromParameter()
{
    if(!ParameterNode::initFromParameter())
    {
        return false;
    }

    param::IPointParameter *pPointParamter = dynamic_cast<param::IPointParameter *>(m_pParameter.get());
    if(pPointParamter == NULL)
    {
        return false;
    }

    cmm::math::Point3d pt   = pPointParamter->getCoordinate();
    m_ptPosition.set(pt.x(), pt.y(), pt.z());

    m_dblPitchAngle    = pPointParamter->getPitchAngle();
    m_dblAzimuthAngle  = pPointParamter->getAzimuthAngle();

    removeChildren(0, getNumChildren());
    addChild(createNodeByParameter());

    return true;
}


void computeTextureCoord(osg::Geometry *pGeometry)
{
    if(pGeometry == NULL)   return;

    osg::Vec3Array *pVertex = dynamic_cast<osg::Vec3Array *>(pGeometry->getVertexArray());

    osg::BoundingBox bb;
    unsigned int nSize = pVertex->size();
    for(unsigned int i = 0; i < nSize; i++)
    {
        bb.expandBy((*pVertex)[i]);
    }

    osg::ref_ptr<osg::Vec2Array> pNormal = new osg::Vec2Array;
    float w = bb.xMax() - bb.xMin();
    float h = bb.yMax() - bb.yMin();
    for(unsigned int i = 0; i < nSize; i++)
    {
        float x = ((*pVertex)[i]._v[0] - bb.xMin()) / w;
        float y = ((*pVertex)[i]._v[1] - bb.yMin()) / h;
        pNormal->push_back(osg::Vec2(x, y));
    }

    pGeometry->setTexCoordArray(0, pNormal);

    return;
}


osg::Node * PointParameterNode::createNodeByParameter()
{
    param::ISymbol *pSymbol = m_pParameter->getSymbol();
    
    m_bTransedOntoGlobe = false;
    for(unsigned int i = 0; i < pSymbol->getNumDetail(); i++)
    {
        OpenSP::sp<param::IDetail> pDetail = pSymbol->getDetail(i);
        param::IStaticModelDetail *pStatic = dynamic_cast<param::IStaticModelDetail*>(pDetail.get());
        if(pStatic != NULL && pStatic->isOnGlobe())
        {
            m_bTransedOntoGlobe = true;
        }
        //if (pStatic && pStatic->isOnGlobe()) m_bTransedOntoGlobe = true;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::Matrixd mtx;
    if(m_bTransedOntoGlobe)
    {
        osg::Vec3d vTrans;
        pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition._v[1], m_ptPosition._v[0], m_ptPosition._v[2] + m_dblHeight, vTrans._v[0], vTrans._v[1], vTrans._v[2]);
        mtx.setTrans(vTrans);
    }
    else
    {
        pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight(m_ptPosition._v[1], m_ptPosition._v[0], m_ptPosition._v[2] + m_dblHeight, mtx);
    }
    pMatrixTransform->setMatrix(mtx);

    osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
    pPagedLOD->setCenter(osg::Vec3(0.0, 0.0, 0.0));
    pPagedLOD->setRadius(pSymbol->getBoundSphereRadius());
    pMatrixTransform->addChild(pPagedLOD.get());

    unsigned int nDetail = pSymbol->getNumDetail();
    for(unsigned int i = 0; i < nDetail; i++)
    {
        OpenSP::sp<param::IDetail> pDetail = pSymbol->getDetail(i);
        addChildByDetail(pPagedLOD.get(), pDetail);
    }

    return pMatrixTransform.release();
}


void PointParameterNode::addChildByDetail(osg::LOD *pLOD, param::IDetail *pDetail)
{
    if(pDetail == NULL) return;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    if(m_bTransedOntoGlobe)
    {
        osg::Vec3d vTrans;
        osg::Matrixd mtx, mtx_t;
        pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight(m_ptPosition._v[1], m_ptPosition._v[0], m_ptPosition._v[2] + m_dblHeight, mtx);
        pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition._v[1], m_ptPosition._v[0], m_ptPosition._v[2] + m_dblHeight, vTrans._v[0], vTrans._v[1], vTrans._v[2]);

        mtx_t.setTrans(-vTrans);
        mtx.postMult(mtx_t);
        pMatrixTransform->setMatrix(mtx);
    }

    osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pLOD);

    double dblMin, dblMax;
    pDetail->getRange(dblMin, dblMax);

    const std::string &strType = pDetail->getStyle();

    bool bIgnoreImage = false;
    osg::ref_ptr<osg::Node> pDetailNode = NULL;
    if(strType.compare(param::POINT_DETAIL) == 0)
    {
        pDetailNode = createPointDetail(pDetail);
    }
    else if(strType.compare(param::CUBE_DETAIL) == 0)
    {
        pDetailNode = createCubeDetail(pDetail);
    }
    else if(strType.compare(param::CYLINDER_DETIAL) == 0)
    {
        pDetailNode = createCylinderDetail(pDetail);
    }
    else if(strType.compare(param::SPHERE_DETAIL) == 0)
    {
        pDetailNode = createSphereDetail(pDetail);
    }
    else if(strType.compare(param::PRISM_DETAIL) == 0)
    {
        pDetailNode = createPrismDetail(pDetail);
    }
    else if(strType.compare(param::PYRAMID_DETAIL) == 0)
    {
        pDetailNode = createPyramidDetail(pDetail);
    }
    else if (strType.compare(param::SECTOR_DETAIL) == 0)
    {
        pDetailNode = createSectorDetail(pDetail);
    }
    else if (strType.compare(param::PIPECONNECTOR_DETAIL) == 0)
    {
        pDetailNode = createPipeConnectorDetail(pDetail);
    }
    else if(strType.compare(param::IMAGE_DETAIL) == 0)
    {
        pDetailNode = createImageDetail(pDetail);
    }
    else if(strType.compare(param::STATIC_DETAIL) == 0)
    {
        bIgnoreImage = true;
        param::IStaticModelDetail *pStaModel = dynamic_cast<param::IStaticModelDetail *>(pDetail);
        if(pStaModel == NULL)
        {
            return;
        }

        const ID &modelID = pStaModel->getModelID();
        if(modelID.ModelID.m_nType == MODEL_ID || modelID.ModelID.m_nType == SHARE_MODEL_ID)
        {

            unsigned int nIndex = pPagedLOD->getNumFileNames();
            pPagedLOD->setFileID(nIndex, modelID);
            pPagedLOD->setRange(nIndex, dblMin, dblMax);

            // YJS：严重警告！！！
            // 对于种植的房子，它们没有上球，这里怎么办？上球的旋转矩阵放在哪里？？
        }
        return;
    }
    else if(strType.compare(param::BubbleText_DETAIL) == 0)
    {
        pDetailNode = createBubbleTextDetail(pDetail);
    }
    else if(strType.compare(param::Polygon_DETAIL) == 0)
    {
        pDetailNode = createPolygonDetail(pDetail);
    }
    if(!pDetailNode.valid())    return;

    osg::StateSet *pStateSet = pDetailNode->getOrCreateStateSet();
    const param::IDynModelDetail *pDynModelDetail = dynamic_cast<const param::IDynModelDetail *>(pDetail);
    if(pDynModelDetail)
    {
        osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);
        pStateSet->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
        if(!bIgnoreImage)
        {
            const ID &imgID = pDynModelDetail->getImageID();
            bindTexture(imgID, pStateSet);
        }

        const cmm::FloatColor &color = pDynModelDetail->getDynModelColor();
        if(color.m_fltA < 0.98)
        {
            pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
    }


    if(m_bTransedOntoGlobe)
    {
        pMatrixTransform->addChild(pDetailNode);
        pPagedLOD->addChild(pMatrixTransform, dblMin, dblMax, "");
        const osg::BoundingSphere &bs = pMatrixTransform->getBound();
        const double dblRadius = bs.radius() * 1.5;
        pPagedLOD->setCenter(bs._center);
        if(dblRadius > pPagedLOD->getRadius())
        {
            pPagedLOD->setRadius(dblRadius);
        }
    }
    else
    {
        pPagedLOD->addChild(pDetailNode, dblMin, dblMax, "");
        const osg::BoundingSphere &bs = pDetailNode->getBound();
        const double dblRadius = bs.radius() * 1.5;
        pPagedLOD->setCenter(bs._center);
        if(dblRadius > pPagedLOD->getRadius())
        {
            pPagedLOD->setRadius(dblRadius);
        }
    }

    return;
}


osg::Node *PointParameterNode::createPointDetail(const param::IDetail *pDetail) const
{
    const param::IDynPointDetail *pPointDetail = dynamic_cast<const param::IDynPointDetail *>(pDetail);
    if(!pPointDetail)   return NULL;

    const cmm::FloatColor &clr = pPointDetail->getPointColor();
    const double dblSize = pPointDetail->getPointSize();

    osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;
    pVertex->push_back(osg::Vec3(0.0, 0.0, 0.0));
    osg::ref_ptr<osg::Vec4Array> pColor = new osg::Vec4Array;
    pColor->push_back(osg::Vec4(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA));

    osg::ref_ptr<osg::Point> pPoint = new osg::Point;
    pPoint->setSize(dblSize);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVertex);
    pGeometry->setColorArray(pColor);
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->getOrCreateStateSet()->setAttribute(pPoint);
    pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, pVertex->size()));
    pGeode->addDrawable(pGeometry);

    return pGeode.release();
}


osg::Node *PointParameterNode::createCubeDetail(const param::IDetail *pDetail) const
{
    const param::ICubeDetail *pCubeDetail = dynamic_cast<const param::ICubeDetail *>(pDetail);
    if(pCubeDetail == NULL) return NULL;

    const cmm::FloatColor &clr = pCubeDetail->getDynModelColor();

    double dblLength, dblWidth, dblHeight;
    pCubeDetail->getCubeSize(dblLength, dblWidth, dblHeight);

    osg::ref_ptr<osg::Box>  pShape = new osg::Box(osg::Vec3(0, 0, 0), dblLength, dblWidth, dblHeight);
    osg::ref_ptr<osg::ShapeDrawable> pShapeDrawable = new osg::ShapeDrawable(pShape);
    pShapeDrawable->setColor(osg::Vec4(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA));

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pShapeDrawable);
    return pGeode.release();
}


bool PointParameterNode::IHole2HoleInfo(const param::IHole *pHole, double dblOffset, HoleInfo &hole) const
{
    const std::string &strHoleType = pHole->getHoleType();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Matrix mtx;
    pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight(m_ptPosition[1], m_ptPosition[0], m_ptPosition[2] + m_dblHeight, mtx);
    mtx = osg::Matrix::inverse(mtx);

    osg::Vec3d vHolePoint;

    if(0 == strHoleType.compare(param::HoleType_Circle))
    {
        hole.m_bCircleHole  = true;
        hole.m_nHint        = 20;

        cmm::math::Point2d ptHoleCenter;
        double dblHoleRadius;
        pHole->getCircle(ptHoleCenter, dblHoleRadius);

        pEllipsoidModel->convertLatLongHeightToXYZ(ptHoleCenter._v[1], ptHoleCenter._v[0], m_ptPosition._v[2] + m_dblHeight + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);

        hole.m_ptCenter     = mtx.preMult(vHolePoint);;
        hole.m_dblRadius    = dblHoleRadius;

        return true;
    }
    else if(0 == strHoleType.compare(param::HoleType_Rectangle))
    {
        hole.m_bCircleHole  = false;

        cmm::math::Point2d ptHoleCenter;
        double dblWidth, dblHeight, dblAzimuthAngle;
        pHole->getRectangle(ptHoleCenter, dblWidth, dblHeight, dblAzimuthAngle);

        double dblDeltaAngle = dblAzimuthAngle - m_dblAzimuthAngle;
        osg::Quat qt;
        qt.makeRotate(dblDeltaAngle, osg::Vec3d(0.0, 0.0, 1.0));

        pEllipsoidModel->convertLatLongHeightToXYZ(ptHoleCenter._v[1], ptHoleCenter._v[0], m_ptPosition._v[2] + m_dblHeight + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);
        vHolePoint          = mtx.preMult(vHolePoint);

        osg::Vec3d vCorner;
        
        //左上角
        vCorner.set(vHolePoint[0] - dblWidth * 0.5, vHolePoint[1] + dblHeight * 0.5, vHolePoint[2]);
        hole.m_vecPolygon.push_back(qt * vCorner);
        //右上角
        vCorner.set(vHolePoint[0] + dblWidth * 0.5, vHolePoint[1] + dblHeight * 0.5, vHolePoint[2]);
        hole.m_vecPolygon.push_back(qt * vCorner);
        //右下角
        vCorner.set(vHolePoint[0] + dblWidth * 0.5, vHolePoint[1] - dblHeight * 0.5, vHolePoint[2]);
        hole.m_vecPolygon.push_back(qt * vCorner);
        //左下角
        vCorner.set(vHolePoint[0] - dblWidth * 0.5, vHolePoint[1] - dblHeight * 0.5, vHolePoint[2]);
        hole.m_vecPolygon.push_back(qt * vCorner);

        return true;
    }
    else if(0 == strHoleType.compare(param::HoleType_Polygon))
    {
        hole.m_bCircleHole  = false;

        std::vector<cmm::math::Point2d> vecVertex;
        pHole->getAllPolygonVertex(vecVertex);

        unsigned int nCount = vecVertex.size();
        for(unsigned int i = 0; i < nCount; i++)
        {
            pEllipsoidModel->convertLatLongHeightToXYZ(vecVertex[i]._v[1], vecVertex[i]._v[0], m_ptPosition._v[2] + m_dblHeight + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);
            hole.m_vecPolygon.push_back(mtx.preMult(vHolePoint));
        }

        return true;
    }
    
    return false;
}

osg::Node *PointParameterNode::createCylinderDetail(const param::IDetail *pDetail) const
{
    const param::ICylinderDetail *pCylinderDetail = dynamic_cast<const param::ICylinderDetail *>(pDetail);
    if(pCylinderDetail == NULL)     return NULL;

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;

    const cmm::FloatColor &clr = pCylinderDetail->getDynModelColor();
    const bool bTopVisible = pCylinderDetail->getTopVisible();
    const bool bBottomVisible = pCylinderDetail->getBottomVisible();

    const double dblTopRadius = pCylinderDetail->getRadiusTop();
    const double dblBottomRadius = pCylinderDetail->getRadiusBottom();
    const double dblHeight = pCylinderDetail->getHeight();

    const ID &imgID = pCylinderDetail->getImageID();

    unsigned int nHint = 40;

    const double dblTheta = atan2(fabs(dblTopRadius - dblBottomRadius), dblHeight);
    const double dblSectionRatio = sin(dblTheta * 2.0) * 6.0;
    const unsigned nSection  = osg::clampAbove((unsigned)ceil(dblSectionRatio), 1u);
    const unsigned nVtxCount = nHint + 1u;
    const unsigned nVerticesCount = nVtxCount * nVtxCount;
    const unsigned nVerticesCount2 = nVerticesCount + nVerticesCount;

    osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array;
    pClrArray->push_back(osg::Vec4(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA));

    osg::ref_ptr<osg::Vec3Array>    pSideNormals       = genConeNormal(nHint, dblTopRadius, dblBottomRadius, dblHeight);
    osg::ref_ptr<osg::Vec3Array>    pSideVertices      = new osg::Vec3Array;
    osg::ref_ptr<osg::UShortArray>  pSideVtxIndices    = new osg::UShortArray;
    osg::ref_ptr<osg::UShortArray>  pSideNormalIndices = new osg::UShortArray;

    pSideVertices->reserve(nVerticesCount);
    pSideVtxIndices->reserve(nVerticesCount2);
    pSideNormalIndices->reserve(nVerticesCount2);
    
    osg::ref_ptr<osg::Geometry>     pSideGeometry = new osg::Geometry;
    pSideGeometry->setVertexArray(pSideVertices.get());
    pSideGeometry->setVertexIndices(pSideVtxIndices.get());
    
    pSideGeometry->setNormalArray(pSideNormals.get());
    pSideGeometry->setNormalIndices(pSideNormalIndices.get());
    pSideGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::ref_ptr<osg::Vec2Array>    pSideTexCoord = NULL;
    osg::ref_ptr<osg::UShortArray>  pSideTexIndices = NULL;
    const bool bHasTex = imgID.isValid();
    if(bHasTex)
    {
        pSideTexCoord = new osg::Vec2Array;
        pSideGeometry->setTexCoordArray(0u, pSideTexCoord.get());
        pSideTexCoord->reserve(nVerticesCount);

        pSideTexIndices = new osg::UShortArray;
        pSideGeometry->setTexCoordIndices(0u, pSideTexIndices.get());
        pSideTexIndices->reserve(nVerticesCount2);
    }

    const double dblSectionHeight = dblHeight / nSection;
    const double dblRadiusDetal = (dblBottomRadius - dblTopRadius) / nSection;
    double dblRadius = dblTopRadius;
    osg::Vec3d ptSectionCenter(0.0, 0.0, dblHeight * 0.5);
    const unsigned nOffset = nVtxCount + nVtxCount;
    const osg::Vec2d vecTexCoordDetal(1.0 / nHint, 1.0 / nSection);
    osg::Vec2d  ptTexCoord(0.0, 0.0);
    for(unsigned n = 0u; n <= nSection; n++)
    {
        const osg::ref_ptr<osg::Vec3Array> pVtxArray = genCircleVertices(ptSectionCenter, dblRadius, nHint);
        pSideVertices->insert(pSideVertices->end(), pVtxArray->begin(), pVtxArray->end());
        
        const unsigned nIndexBegin = n * nVtxCount;
        for(unsigned m = 0u; m <= nHint; m++)
        {
            const unsigned nIndex0 = nIndexBegin + m;
            const unsigned nIndex1 = nIndex0 + nVtxCount;

            pSideVtxIndices->push_back(nIndex0);
            pSideVtxIndices->push_back(nIndex1);

            pSideNormalIndices->push_back(m);
            pSideNormalIndices->push_back(m);

            if(bHasTex)
            {
                pSideTexCoord->push_back(ptTexCoord);
                ptTexCoord.x() += vecTexCoordDetal.x();

                pSideTexIndices->push_back(nIndex0);
                pSideTexIndices->push_back(nIndex1);
            }
        }

        if(bHasTex)
        {
            ptTexCoord.y() += vecTexCoordDetal.y();
            ptTexCoord.x()  = 0.0;
        }

        if(n < nSection)
        {
            osg::ref_ptr<osg::DrawArrays>   pSectionPrim = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, n * nVtxCount * 2u, nVtxCount * 2u);
            pSideGeometry->addPrimitiveSet(pSectionPrim.get());
        }

        ptSectionCenter.z() -= dblSectionHeight;
        dblRadius += dblRadiusDetal;
    }

    osg::ref_ptr<osg::Geometry> pTopGeometry, pBottomGeometry;

    std::vector<param::IHole *> holeList;
    pCylinderDetail->getHoleList(holeList);

    bool bHasHole = (holeList.size() > 0 ? true : false);

    if(!bHasHole)
    {
        if(bTopVisible && dblTopRadius > FLT_EPSILON)
        {
            osg::ref_ptr<osg::Vec3Array>    pTopVertices = new osg::Vec3Array;
            pTopVertices->push_back(osg::Vec3(0.0, 0.0, dblHeight * 0.5));
            pTopVertices->insert(pTopVertices->end(), pSideVertices->begin(), pSideVertices->begin() + nVtxCount);
            pTopGeometry = createCircleSheet(pTopVertices.get(), bHasTex);
        }

        if(bBottomVisible && dblBottomRadius > FLT_EPSILON)
        {
            osg::ref_ptr<osg::Vec3Array>    pBottomVertices = new osg::Vec3Array;
            pBottomVertices->push_back(osg::Vec3(0.0, 0.0, -dblHeight * 0.5));
            pBottomVertices->insert(pBottomVertices->end(), pSideVertices->rbegin(), pSideVertices->rbegin() + nVtxCount);
            pBottomGeometry = createCircleSheet(pBottomVertices.get(), bHasTex);
        }
    }
    else
    {
        HoleInfoList topHoleList, bottomHoleList;
        for(unsigned int i = 0; i < holeList.size(); i++)
        {
            HoleInfo hole; 
            bool bOnTopFace = holeList[i]->isOnTopFace();
            if(!IHole2HoleInfo(holeList[i], bOnTopFace ? dblHeight * 0.5 : -dblHeight * 0.5, hole))
            {
                continue;
            }

            if(bOnTopFace)
            {
                topHoleList.push_back(hole);
            }
            else
            {
                bottomHoleList.push_back(hole);
            }
        }

        if(bTopVisible)
        {
            if(topHoleList.empty())
            {
                osg::ref_ptr<osg::Vec3Array>    pTopVertices = new osg::Vec3Array;
                pTopVertices->push_back(osg::Vec3(0.0, 0.0, dblHeight * 0.5));
                pTopVertices->insert(pTopVertices->end(), pSideVertices->begin(), pSideVertices->begin() + nVtxCount);
                pTopGeometry = createCircleSheet(pTopVertices.get(), bHasTex);
            }
            else
            {
                pTopGeometry = createCircleWithHoles(osg::Vec3(0.0, 0.0, dblHeight * 0.5), dblRadius, 20, topHoleList);
            }
        }

        if(bBottomVisible)
        {
            if(bottomHoleList.empty())
            {
                osg::ref_ptr<osg::Vec3Array>    pBottomVertices = new osg::Vec3Array;
                pBottomVertices->push_back(osg::Vec3(0.0, 0.0, -dblHeight * 0.5));
                pBottomVertices->insert(pBottomVertices->end(), pSideVertices->rbegin(), pSideVertices->rbegin() + nVtxCount);
                pBottomGeometry = createCircleSheet(pBottomVertices.get(), bHasTex);
            }
            else
            {
                pBottomGeometry = createCircleWithHoles(osg::Vec3(0.0, 0.0, -dblHeight * 0.5), dblRadius, 20, bottomHoleList);
            }
        }
    }

    osg::StateSet *pStateSet = pSideGeometry->getOrCreateStateSet();
    if(bHasTex)
    {
        bindTexture(imgID, pStateSet);
    }
    else
    {
        pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    }

    pSideGeometry->setColorArray(pClrArray);
    pSideGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeode->addDrawable(pSideGeometry.get());

    pStateSet = pTopGeometry->getOrCreateStateSet();
    if(bHasTex)
    {
        computeTextureCoord(pTopGeometry);
        bindTexture(imgID, pStateSet);
    }
    else
    {
        pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    }

    pTopGeometry->setColorArray(pClrArray);
    pTopGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeode->addDrawable(pTopGeometry.get());

    pStateSet = pBottomGeometry->getOrCreateStateSet();
    if(bHasTex)
    {
        computeTextureCoord(pBottomGeometry);
        bindTexture(imgID, pStateSet);
    }
    else
    {
        pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    }

    pBottomGeometry->setColorArray(pClrArray);
    pBottomGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeode->addDrawable(pBottomGeometry.get());

    return pGeode.release();
}


osg::Node *PointParameterNode::createSphereDetail(const param::IDetail *pDetail) const
{
    const param::ISphereDetail *pShereDetail = dynamic_cast<const param::ISphereDetail *>(pDetail);
    if(pShereDetail == NULL)    return NULL;

    const cmm::FloatColor &clr = pShereDetail->getDynModelColor();

    const double dblRadius = pShereDetail->getRadius();

    osg::ref_ptr<osg::ShapeDrawable> pShape = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), dblRadius));
    pShape->setColor(osg::Vec4(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA));


    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pShape);
    return pGeode.release();
}


osg::Node *PointParameterNode::createPrismDetail(const param::IDetail *pDetail) const
{
    const param::IPrismDetail *pPrismDetail = dynamic_cast<const param::IPrismDetail *>(pDetail);
    if(pPrismDetail == NULL)
    {
        return NULL;
    }

    const std::vector<cmm::math::Point2d> &vecVerticesCoord = pPrismDetail->getVertices();
    if(vecVerticesCoord.size() < 3u)
    {
        return NULL;
    }

    const cmm::FloatColor &color = pPrismDetail->getDynModelColor();
    osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array(1);
    (*pClrArray)[0].set(color.m_fltR, color.m_fltG, color.m_fltB, color.m_fltA);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const double dblHeight = m_ptPosition.z() + m_dblHeight;
    osg::Vec3d ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition.y(), m_ptPosition.x(), dblHeight, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    osg::Vec3d vecPositionUp = ptCenterPos;
    vecPositionUp.normalize();

    osg::Quat   qtRotation;

    const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
    osg::Quat   qtToAxisZ;
    qtToAxisZ.makeRotate(vecPositionUp, vecAxisZ);
    qtRotation *= qtToAxisZ;

    const osg::Vec3d    vecEastern(-sin(m_ptPosition.x()), cos(m_ptPosition.x()), 0.0);
    const osg::Vec3d    vecAxisX(1.0, 0.0, 0.0);
    osg::Quat  qtToAxisX;
    qtToAxisX.makeRotate(vecEastern, vecAxisX);
    qtRotation *= qtToAxisX;

    ptCenterPos = qtRotation * ptCenterPos;

    cmm::math::Polygon2     polygon;
    std::vector<osg::Vec3d>             vecVertices(vecVerticesCoord.size());
    std::vector<osg::Vec3d>::iterator   itorVtx = vecVertices.begin();
    std::vector<cmm::math::Point2d>::const_iterator itorVtxCoord = vecVerticesCoord.begin();
    for( ; itorVtxCoord != vecVerticesCoord.end(); ++itorVtxCoord, ++itorVtx)
    {
        const cmm::math::Point2d &vtxCoord = *itorVtxCoord;
        osg::Vec3d &vtx = *itorVtx;

        pEllipsoidModel->convertLatLongHeightToXYZ(vtxCoord.y(), vtxCoord.x(), dblHeight, vtx.x(), vtx.y(), vtx.z());
        vtx  = qtRotation * vtx;
        vtx -= ptCenterPos;

        polygon.addVertex(cmm::math::Point2d(vtx.x(), vtx.y()));
    }

    const double dblPolygonArea = polygon.area();
    const bool bAntiClockwise = (dblPolygonArea >= 0.0);
    if(!bAntiClockwise)
    {
        std::reverse(vecVertices.begin(), vecVertices.end());
    }

    const double dblHalfHeight = pPrismDetail->getHeight() * 0.5;

    std::vector<osg::Vec3d>     vecTopVertices(vecVertices);
    std::vector<osg::Vec3d>     vecBottomVertices(vecVertices.begin(), vecVertices.end());
    for(unsigned n = 0u; n < vecVertices.size(); n++)
    {
        osg::Vec3d &vtx0 = vecTopVertices[n];
        vtx0.z() += dblHalfHeight;

        osg::Vec3d &vtx1 = vecBottomVertices[n];
        vtx1.z() -= dblHalfHeight;
    }

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;

    const ID &imgSide = pPrismDetail->getImageID();
    osg::ref_ptr<osg::Drawable> pSideFace = createPrismSide(vecTopVertices, vecBottomVertices, imgSide.isValid());
    if(pSideFace.valid())
    {
        osg::Geometry *pGeometry = pSideFace->asGeometry();
        if(pGeometry != NULL)
        {
            pGeometry->setColorArray(pClrArray);
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        }
        pGeode->addDrawable(pSideFace.get());
    }


    std::vector<param::IHole *> holeList;
    pPrismDetail->getHoleList(holeList);

    bool bHasHole = (holeList.size() > 0 ? true : false);

    HoleInfoList topHoleList, bottomHoleList;
    for(unsigned int i = 0; i < holeList.size(); i++)
    {
        HoleInfo hole; 
        bool bOnTopFace = holeList[i]->isOnTopFace();
        if(!IHole2HoleInfo(holeList[i], bOnTopFace ? dblHalfHeight : -dblHalfHeight, hole))
        {
            continue;
        }

        if(bOnTopFace)
        {
            topHoleList.push_back(hole);
        }
        else
        {
            bottomHoleList.push_back(hole);
        }
    }

    if(pPrismDetail->getTopVisible())
    {
        const ID &imgTop = pPrismDetail->getTopImageID();
        const bool bHasTexture = imgTop.isValid();

        osg::ref_ptr<osg::Drawable> pTopFace;
        if(topHoleList.empty())
        {
            pTopFace = createPolygonSheet(vecTopVertices, bHasTexture);
        }
        else
        {
            pTopFace = createSheetWithHoles(vecTopVertices, topHoleList);
        }
        if(pTopFace.valid())
        {
            osg::StateSet *pStateSet = pTopFace->getOrCreateStateSet();
            if(bHasTexture)
            {
                computeTextureCoord(pTopFace->asGeometry());
                bindTexture(imgTop, pStateSet);
            }
            else
            {
                pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
            }

            osg::Geometry *pGeometry = pTopFace->asGeometry();
            if(pGeometry != NULL)
            {
                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            }
            pGeode->addDrawable(pTopFace.get());
        }
    }

    if(pPrismDetail->getBottomVisible())
    {
        const ID &imgBottom = pPrismDetail->getBottomImageID();
        const bool bHasTexture = imgBottom.isValid();
        std::vector<osg::Vec3d>    vecBottomVerticesR(vecBottomVertices.rbegin(), vecBottomVertices.rend());
        osg::ref_ptr<osg::Drawable> pBottomFace;
        if(topHoleList.empty())
        {
            pBottomFace = createPolygonSheet(vecBottomVerticesR, bHasTexture);
        }
        else
        {
            pBottomFace = createSheetWithHoles(vecBottomVerticesR, bottomHoleList);
        }
        if(pBottomFace.valid())
        {
            osg::StateSet *pStateSet = pBottomFace->getOrCreateStateSet();
            if(bHasTexture)
            {
                computeTextureCoord(pBottomFace->asGeometry());
                bindTexture(imgBottom, pStateSet);
            }
            else
            {
                pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
            }

            osg::Geometry *pGeometry = pBottomFace->asGeometry();
            if(pGeometry != NULL)
            {
                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            }
            pGeode->addDrawable(pBottomFace.get());
        }
    }

    return pGeode.release();
}


osg::Node *PointParameterNode::createSectorDetail(const param::IDetail *pDetail) const
{
    const param::ISectorDetail *pSectorDetail = dynamic_cast<const param::ISectorDetail *>(pDetail);
    if(pSectorDetail == NULL)
    {
        return NULL;
    }

    const double dblAngleDelta = pSectorDetail->getEndAngle() - pSectorDetail->getBeginAngle();
    const unsigned nSegCount   = 60u;
    const double dblAngleBias  = dblAngleDelta / nSegCount;

    const double dblOuterRadius = std::max(pSectorDetail->getRadius1(), pSectorDetail->getRadius2());
    const double dblInnerRadius = std::min(pSectorDetail->getRadius1(), pSectorDetail->getRadius2());

    osg::PrimitiveSet::Mode eMode = osg::PrimitiveSet::TRIANGLE_FAN;
    osg::ref_ptr<osg::Vec3Array>    pVerticesArray = new osg::Vec3Array;
    if(cmm::math::floatEqual(dblInnerRadius, 0.0))
    {
        // 标准扇形
        pVerticesArray->reserve(nSegCount + 1u);
        pVerticesArray->push_back(osg::Vec3(0.0, 0.0, 0.0));

        double dblAngle = pSectorDetail->getBeginAngle();
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            osg::Vec3 vtx;
            vtx.x() = cos(dblAngle) * pSectorDetail->getRadius1();
            vtx.y() = sin(dblAngle) * pSectorDetail->getRadius1();
            vtx.z() = 0.0;
            pVerticesArray->push_back(vtx);

            dblAngle += dblAngleBias;
        }
        eMode = osg::PrimitiveSet::TRIANGLE_FAN;
    }
    else if(cmm::math::floatEqual(dblInnerRadius, dblOuterRadius))
    {
        // 只是一个框框而已
        pVerticesArray->reserve(nSegCount);

        double dblAngle = pSectorDetail->getBeginAngle();
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            osg::Vec3 vtx;
            vtx.x() = cos(dblAngle) * pSectorDetail->getRadius1();
            vtx.y() = sin(dblAngle) * pSectorDetail->getRadius1();
            vtx.z() = 0.0;
            pVerticesArray->push_back(vtx);

            dblAngle += dblAngleBias;
        }
        eMode = osg::PrimitiveSet::LINE_STRIP;
    }
    else
    {
        // 扇环
        pVerticesArray->reserve(nSegCount + nSegCount);

        double dblAngle = pSectorDetail->getBeginAngle();
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            const double dblCos = cos(dblAngle);
            const double dblSin = sin(dblAngle);

            const osg::Vec3 vtx0(dblCos * pSectorDetail->getRadius1(), dblSin * pSectorDetail->getRadius1(), 0.0);
            const osg::Vec3 vtx1(dblCos * pSectorDetail->getRadius2(), dblSin * pSectorDetail->getRadius2(), 0.0);
            pVerticesArray->push_back(vtx0);
            pVerticesArray->push_back(vtx1);

            dblAngle += dblAngleBias;
        }

        eMode = osg::PrimitiveSet::TRIANGLE_STRIP;
    }

    osg::ref_ptr<osg::Vec4Array>    pColorArray = new osg::Vec4Array;
    const cmm::FloatColor &color = pSectorDetail->getDynModelColor();
    pColorArray->push_back(osg::Vec4(color.m_fltR, color.m_fltG, color.m_fltB, color.m_fltA));

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVerticesArray.get());
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(eMode, 0u, pVerticesArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    pGeode->addDrawable(pGeometry.get());

    const double dblBorderWidth = pSectorDetail->getBorderWidth();
    if(dblBorderWidth > 0.99 && eMode != osg::PrimitiveSet::LINE_STRIP)
    {
        osg::ref_ptr<osg::Geometry> pBorderGeom = new osg::Geometry;
        pBorderGeom->setVertexArray(pVerticesArray.get());

        if(dblAngleDelta < osg::PI * 2.0 - FLT_EPSILON)
        {
            // 这不是一个整圆
            if(eMode == osg::PrimitiveSet::TRIANGLE_FAN)
            {
                pVerticesArray->push_back(osg::Vec3(0.0, 0.0, 0.0));
            }
            osg::ref_ptr<osg::DrawArrays> pBorderPrim = new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0u, pVerticesArray->size());
            pBorderGeom->addPrimitiveSet(pBorderPrim);
        }
        else
        {
            // 这是一个整圆，因此不需要绘制径向边线
            if(eMode == osg::PrimitiveSet::TRIANGLE_FAN)
            {
                // 普通扇形
                osg::ref_ptr<osg::DrawArrays> pBorderPrim = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 1u, pVerticesArray->size() - 1u);
                pBorderGeom->addPrimitiveSet(pBorderPrim);
            }
            else
            {
                // 扇环
                const unsigned nCount = pVerticesArray->size() / 2u;
                osg::ref_ptr<osg::DrawArrays> pBorderPrim1 = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0u, nCount);
                osg::ref_ptr<osg::DrawArrays> pBorderPrim2 = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, nCount, nCount);
                pBorderGeom->addPrimitiveSet(pBorderPrim1);
                pBorderGeom->addPrimitiveSet(pBorderPrim2);
            }
        }

        if(eMode == osg::PrimitiveSet::TRIANGLE_STRIP)
        {
            // 扇环
            osg::ref_ptr<osg::UShortArray>  pIndicesArray = new osg::UShortArray;
            pIndicesArray->reserve(pVerticesArray->size());

            //const unsigned nLoop = pVerticesArray->size() / 2u;
            const int nCount = pVerticesArray->size();
            for(int i = 0u; i < nCount; i += 2)
            {
                pIndicesArray->push_back(i);
            }
            for(int i = nCount - 1; i > 0; i -= 2)
            {
                pIndicesArray->push_back(i);
            }
            pBorderGeom->setVertexIndices(pIndicesArray.get());
        }

        const cmm::FloatColor &clrBorder = pSectorDetail->getBorderColor();
        osg::ref_ptr<osg::Vec4Array>    pBorderColor = new osg::Vec4Array;
        pBorderColor->push_back(osg::Vec4(clrBorder.m_fltR, clrBorder.m_fltG, clrBorder.m_fltB, 1.0));
        pBorderGeom->setColorArray(pBorderColor.get());
        pBorderGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::ref_ptr<osg::LineWidth>    pLineWidth = new osg::LineWidth(dblBorderWidth);
        osg::StateSet *pStateSet = pBorderGeom->getOrCreateStateSet();
        pStateSet->setAttributeAndModes(pLineWidth.get());

        pGeode->addDrawable(pBorderGeom.get());
    }

    return pGeode.release();
}


osg::Node *PointParameterNode::createPyramidDetail(const param::IDetail *pDetail) const
{
    const param::IPyramidDetail *pPyramidDetail = dynamic_cast<const param::IPyramidDetail *>(pDetail);
    if(pPyramidDetail == NULL)
    {
        return NULL;
    }

    const std::vector<cmm::math::Point3d> &vecBottomVertices = pPyramidDetail->getBottomVertices();
    if(vecBottomVertices.empty())
    {
        return NULL;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const cmm::FloatColor &color = pPyramidDetail->getDynModelColor();
    osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array(1);
    (*pClrArray)[0].set(color.m_fltR, color.m_fltG, color.m_fltB, color.m_fltA);

    const double dblHeight = m_ptPosition.z() + m_dblHeight;
    osg::Vec3d ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition.y(), m_ptPosition.x(), dblHeight, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    osg::Vec3d vecPositionUp = ptCenterPos;
    vecPositionUp.normalize();

    osg::Quat   qtRotation;

    const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
    osg::Quat   qtToAxisZ;
    qtToAxisZ.makeRotate(vecPositionUp, vecAxisZ);
    qtRotation *= qtToAxisZ;

    const osg::Vec3d    vecEastern(-sin(m_ptPosition.x()), cos(m_ptPosition.x()), 0.0);
    const osg::Vec3d    vecAxisX(1.0, 0.0, 0.0);
    osg::Quat  qtToAxisX;
    qtToAxisX.makeRotate(vecEastern, vecAxisX);
    qtRotation *= qtToAxisX;

    ptCenterPos = qtRotation * ptCenterPos;

    std::vector<osg::Vec3d>             vecVertices(vecBottomVertices.size() + 1);
    osg::Vec3d &ptTop = vecVertices.front();
    ptTop.set(0.0, 0.0, 0.0);

    std::vector<osg::Vec3d>::iterator   itorVtx = vecVertices.begin() + 1u;
    std::vector<cmm::math::Point3d>::const_iterator itorVtxCoord = vecBottomVertices.begin();
    for( ; itorVtxCoord != vecBottomVertices.end(); ++itorVtxCoord, ++itorVtx)
    {
        const cmm::math::Point3d &vtxCoord = *itorVtxCoord;
        osg::Vec3d &vtx = *itorVtx;

        pEllipsoidModel->convertLatLongHeightToXYZ(vtxCoord.y(), vtxCoord.x(), vtxCoord.z(), vtx.x(), vtx.y(), vtx.z());
        vtx  = qtRotation * vtx;
        vtx -= ptCenterPos;
    }

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;

    //const ID &imgSide = pPyramidDetail->getImageID();
    {
        osg::ref_ptr<osg::Geometry>     pSideFace = new osg::Geometry;

        osg::ref_ptr<osg::Vec3Array>    pSideVtx = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array>    pSideNormal = new osg::Vec3Array;
        std::vector<osg::Vec3d>::const_iterator itorVtx1 = vecVertices.begin() + 1u;
        std::vector<osg::Vec3d>::const_iterator itorVtx2 = itorVtx1 + 1u;
        for( ; itorVtx1 != vecVertices.end(); ++itorVtx1)
        {
            const osg::Vec3d &vtx0 = vecVertices.front();
            const osg::Vec3d &vtx1 = *itorVtx1;
            const osg::Vec3d &vtx2 = *itorVtx2;
            pSideVtx->push_back(vtx0);
            pSideVtx->push_back(vtx1);
            pSideVtx->push_back(vtx2);

            const osg::Vec3d vec1 = vtx1 - vtx0;
            const osg::Vec3d vec2 = vtx2 - vtx0;
            osg::Vec3d vecNormal = vec1 ^ vec2;
            vecNormal.normalize();
            pSideNormal->push_back(vecNormal);

            if(++itorVtx2 == vecVertices.end())
            {
                itorVtx2 = vecVertices.begin() + 1u;
            }
        }

        pSideFace->setVertexArray(pSideVtx.get());
        pSideFace->setNormalArray(pSideNormal.get());
        pSideFace->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
        pSideFace->setColorArray(pClrArray.get());
        pSideFace->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, pSideVtx->size());
        pSideFace->addPrimitiveSet(pDrawArrays.get());
        pGeode->addDrawable(pSideFace.get());
    }

    if(pPyramidDetail->getBottomVisible())
    {
        const ID &imgBottom = pPyramidDetail->getBottomImageID();
        const bool bHasTexture = imgBottom.isValid();
        std::vector<osg::Vec3d>    vecBottomVertices(vecVertices.begin() + 1, vecVertices.end());
        osg::ref_ptr<osg::Drawable> pBottomFace;
        pBottomFace = createPolygonSheet(vecBottomVertices, bHasTexture);
        if(pBottomFace.valid())
        {
            osg::StateSet *pStateSet = pBottomFace->getOrCreateStateSet();
            if(bHasTexture)
            {
                computeTextureCoord(pBottomFace->asGeometry());
                bindTexture(imgBottom, pStateSet);
            }
            else
            {
                pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
            }

            osg::Geometry *pGeometry = pBottomFace->asGeometry();
            if(pGeometry != NULL)
            {
                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            }
            pGeode->addDrawable(pBottomFace.get());
        }
    }

    return pGeode.release();
}


osg::Node *PointParameterNode::createPipeConnectorDetail(const param::IDetail *pDetail) const
{
    const param::IPipeConnectorDetail *pPCDetail = dynamic_cast<const param::IPipeConnectorDetail *>(pDetail);
    if(pPCDetail == NULL)   return NULL;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d  ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition.y(), m_ptPosition.x(), m_ptPosition.z() + m_dblHeight, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    osg::Vec3d vecCenterUp = ptCenterPos;
    vecCenterUp.normalize();

    osg::Quat   qtRotation;

    const osg::Vec3d    vecAxisZ(0.0, 0.0, 1.0);
    osg::Quat   qtToAxisZ;
    qtToAxisZ.makeRotate(vecCenterUp, vecAxisZ);
    qtRotation *= qtToAxisZ;

    const osg::Vec3d    vecEastern(-sin(m_ptPosition.x()), cos(m_ptPosition.x()), 0.0);
    const osg::Vec3d    vecAxisX(1.0, 0.0, 0.0);
    osg::Quat  qtToAxisX;
    qtToAxisX.makeRotate(vecEastern, vecAxisX);
    qtRotation *= qtToAxisX;

    ptCenterPos = qtRotation * ptCenterPos;

    PipeConnectorBuilder pcb;
    for(size_t i = 0; i < pPCDetail->getJointsCount(); i++)
    {
        cmm::math::Point3d posTarget;
        double dblLength, dblRadius;
        pPCDetail->getJoint(i, posTarget, dblLength, dblRadius);

        osg::Vec3d vecTargetDir;
        pEllipsoidModel->convertLatLongHeightToXYZ(posTarget.y(), posTarget.x(), posTarget.z() + m_dblHeight, vecTargetDir.x(), vecTargetDir.y(), vecTargetDir.z());
        vecTargetDir  = qtRotation * vecTargetDir;
        vecTargetDir -= ptCenterPos;

        vecTargetDir.normalize();
        vecTargetDir *= dblLength;

        pcb.addJoint(vecTargetDir, dblRadius);
    }
    pcb.setCorner(osg::Vec3d(0.0, 0.0, 0.0));
    pcb.setType(pPCDetail->getType());

    const cmm::FloatColor &color = pPCDetail->getDynModelColor();
    pcb.setColor(osg::Vec4(color.m_fltB, color.m_fltG, color.m_fltB, color.m_fltA));

    osg::ref_ptr<osg::Node> pConnectorNode = pcb.buildPipeConnector();

    return pConnectorNode.release();
}


osg::Node *PointParameterNode::createImageDetail(const param::IDetail *pDetail) const
{
    const param::IDynImageDetail *pImageDetail = dynamic_cast<const param::IDynImageDetail *>(pDetail);
    if(pImageDetail == NULL)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image> pImage;

    const ID &imgID = pImageDetail->getImageID();
    if(imgID.isValid() && imgID.ModelID.m_nType == IMAGE_ID)
    {
        pImage = osgDB::readImageFile(imgID);
    }
    else
    {
        pImage = osgDB::readImageFile(pImageDetail->getFilePath());
    }

    if(!pImage.valid())
    {
        return NULL;
    }

    osg::ref_ptr<ScreenSizeImage> ssi = new ScreenSizeImage;
    ssi->setImage(pImage);

    double w, h;
    pImageDetail->getImageSize(w, h);
    ssi->setSize(w, h);

    ssi->setFont("fonts/times.ttf");
    ssi->setCharacterSize(1);

    ssi->setAxisAlignment(pImageDetail->getOrientateEye() ? osgText::Text::SCREEN : osgText::Text::USER_DEFINED_ROTATION);
    ssi->setCharacterSizeMode(pImageDetail->getLockSize() ? osgText::Text::SCREEN_COORDS : osgText::Text::OBJECT_COORDS);
    if (pImageDetail->getOrientateEye() == false)
    {
        osg::Quat q(osg::Vec3d(0,0,1), m_ptPosition);
        ssi->setRotation(q);
    }

    ssi->setAlignment(osgText::Text::CENTER_CENTER);
    ssi->setText(" ");
    ssi->setPosition(osg::Vec3(0, 0, 0));

    osg::StateSet* stateset = new osg::StateSet;
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND, osg::StateAttribute::OFF);
    ssi->setStateSet(stateset);

    osg::ref_ptr<osg::Geode> g = new osg::Geode;
    g->addDrawable(ssi);
    return g.release();
}


osg::Drawable *createPrismSide(const std::vector<osg::Vec3d> &vecTopVertices, const std::vector<osg::Vec3d> &vecBottomVertices, bool bGenTexCoord)
{
    if(vecTopVertices.size() < 2u || vecBottomVertices.size() < 2u)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;
    pVtxArray->reserve((vecTopVertices.size() + vecBottomVertices.size()) * 3u);

    osg::ref_ptr<osg::Vec3Array>   pNormalArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array>   pTextureCoord = (bGenTexCoord ? new osg::Vec2Array : NULL);
    const osg::Vec2d ptTexTop0(0.0, 1.0), ptTexBottom0(0.0, 0.0), ptTexBottom1(1.0, 0.0), ptTexTop1(1.0, 1.0);
    const unsigned nCount = std::min(vecTopVertices.size(), vecBottomVertices.size());
    for(unsigned i = 0u; i < nCount; i++)
    {
        const osg::Vec3d &top0 = vecTopVertices[i];
        const osg::Vec3d &top1 = vecTopVertices[(i + 1) % vecTopVertices.size()];
        const osg::Vec3d &bottom0 = vecBottomVertices[i];
        const osg::Vec3d &bottom1 = vecBottomVertices[(i + 1) % vecBottomVertices.size()];

        pVtxArray->push_back(top0);
        pVtxArray->push_back(bottom0);
        pVtxArray->push_back(top1);

        pVtxArray->push_back(bottom0);
        pVtxArray->push_back(bottom1);
        pVtxArray->push_back(top1);

        const osg::Vec3d vec0 = bottom0 - top0;
        const osg::Vec3d vec1 = top1 - top0;
        osg::Vec3d vecNormal0 = vec0 ^ vec1;
        vecNormal0.normalize();
        pNormalArray->push_back(vecNormal0);
        pNormalArray->push_back(vecNormal0);
        pNormalArray->push_back(vecNormal0);

        const osg::Vec3d vec2 = bottom0 - bottom1;
        const osg::Vec3d vec3 = top1 - bottom1;
        osg::Vec3d vecNormal1 = vec3 ^ vec2;
        vecNormal1.normalize();
        pNormalArray->push_back(vecNormal1);
        pNormalArray->push_back(vecNormal1);
        pNormalArray->push_back(vecNormal1);

        if(bGenTexCoord)
        {
            pTextureCoord->push_back(ptTexTop0);
            pTextureCoord->push_back(ptTexBottom0);
            pTextureCoord->push_back(ptTexTop1);

            pTextureCoord->push_back(ptTexBottom0);
            pTextureCoord->push_back(ptTexBottom1);
            pTextureCoord->push_back(ptTexTop1);
        }
    }

    osg::ref_ptr<osg::Geometry>     pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());

    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    if(bGenTexCoord)
    {
        pGeometry->setTexCoordArray(0u, pTextureCoord.get());
    }

    osg::ref_ptr<osg::DrawArrays>   pDrawArray = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArray.get());
    return pGeometry.release();
}


osg::Drawable *createPolygonSheet(const std::vector<osg::Vec3d> &vecVertices, bool bGenTexCoord)
{
    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;
    pVtxArray->assign(vecVertices.begin(), vecVertices.end());

    osg::Vec3 vec0 = vecVertices[0] - vecVertices[1];
    vec0.normalize();
    osg::Vec3 vec1 = vecVertices[2] - vecVertices[1];
    vec1.normalize();

    osg::Vec3 vecNormal = vec1 ^ vec0;
    vecNormal.normalize();

    osg::ref_ptr<osg::Vec3Array>   pNormalArray = new osg::Vec3Array;
    pNormalArray->push_back(vecNormal);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());
    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    if(bGenTexCoord)
    {
        osg::Vec2d  ptMin(FLT_MAX, FLT_MAX), ptMax(-FLT_MAX, -FLT_MAX);
        std::vector<osg::Vec3d>::const_iterator itorVtx = vecVertices.begin();
        for( ; itorVtx != vecVertices.end(); ++itorVtx)
        {
            const osg::Vec3d &vtx = *itorVtx;
            if(vtx.x() < ptMin.x())    ptMin.x() = vtx.x();
            if(vtx.y() < ptMin.y())    ptMin.y() = vtx.y();
            if(vtx.x() > ptMax.x())    ptMax.x() = vtx.x();
            if(vtx.y() > ptMax.y())    ptMax.y() = vtx.y();
        }

        osg::Vec2d vecTexRatio = ptMax - ptMin;
        vecTexRatio.x() = osg::clampAbove(vecTexRatio.x(), 0.0001);
        vecTexRatio.y() = osg::clampAbove(vecTexRatio.y(), 0.0001);

        osg::ref_ptr<osg::Vec2Array>   pTextureCoord = new osg::Vec2Array;
        for(itorVtx = vecVertices.begin(); itorVtx != vecVertices.end(); ++itorVtx)
        {
            const osg::Vec3d &vtx = *itorVtx;

            osg::Vec2d ptCoord;
            ptCoord.x() = (vtx.x() - ptMin.x()) / vecTexRatio.x();
            ptCoord.y() = (vtx.y() - ptMin.y()) / vecTexRatio.y();
            pTextureCoord->push_back(ptCoord);
        }

        pGeometry->setTexCoordArray(0, pTextureCoord.get());
    }

    osg::ref_ptr<osg::DrawArrays>   pDrawArray = new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArray.get());

    osg::ref_ptr<osgUtil::Tessellator>  pTessellator = new osgUtil::Tessellator;
    pTessellator->retessellatePolygons(*pGeometry);

    return pGeometry.release();
}


osg::Vec3Array *genConeNormal(unsigned nHint, double dblTopRadius, double dblBottomRadius, double dblHeight)
{
    osg::ref_ptr<osg::Vec3Array>    pNormalArray = new osg::Vec3Array;
    pNormalArray->reserve(nHint + 2u);

    const double dblDetalAngle = osg::PI * 2.0 / nHint;
    double dblAngle = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double dblCos = cos(dblAngle);
        const double dblSin = sin(dblAngle);

        osg::Vec3   vtxTop;
        vtxTop.x() = dblTopRadius * dblCos;
        vtxTop.y() = dblTopRadius * dblSin;
        vtxTop.z() = dblHeight;

        osg::Vec3   vtxBottom;
        vtxBottom.x() = dblBottomRadius * dblCos;
        vtxBottom.y() = dblBottomRadius * dblSin;
        vtxBottom.z() = 0.0;

        const osg::Vec3 vecRadiation(dblCos, dblSin, 0.0);
        const osg::Vec3 vecUp(0.0, 0.0, 1.0);
        const osg::Vec3 vecAxis = vecUp ^ vecRadiation;

        osg::Vec3 vecBottom2Top = vtxTop - vtxBottom;
        vecBottom2Top.normalize();
        osg::Vec3 vecNormal = vecAxis ^ vecBottom2Top;
        vecNormal.normalize();
        pNormalArray->push_back(vecNormal);

        dblAngle += dblDetalAngle;
    }

    return pNormalArray.release();
}


osg::Vec3Array *genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint)
{
    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array;
    pVtxArray->reserve(nHint + 2u);

    const double dblDetalAngle = osg::PI * 2.0 / nHint;
    double dblAngle = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double dblCos = cos(dblAngle);
        const double dblSin = sin(dblAngle);

        osg::Vec3   vtx;
        vtx.x() = dblRadius * dblCos + ptCenter.x();
        vtx.y() = dblRadius * dblSin + ptCenter.y();
        vtx.z() = ptCenter.z();
        pVtxArray->push_back(vtx);
        dblAngle += dblDetalAngle;
    }
    return pVtxArray.release();
}


osg::Geometry *createCircleSheet(osg::Vec3Array *pVertices, bool bHasTex)
{
    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVertices);

    const osg::Vec3 &vtx0 = pVertices->front();
    const osg::Vec3 &vtx1 = pVertices->at(1u);
    const unsigned nStep   = (pVertices->size() > 4u ? pVertices->size() / 4u : 1u);
    const osg::Vec3 &vtx2 = pVertices->at(1u + nStep);
    const osg::Vec3 vec1 = vtx1 - vtx0;
    const osg::Vec3 vec2 = vtx2 - vtx0;

    osg::ref_ptr<osg::Vec3Array>    pNormals = new osg::Vec3Array(1u);
    osg::Vec3 &vec = pNormals->front();
    vec = vec1 ^ vec2;
    vec.normalize();

    pGeometry->setNormalArray(pNormals.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    if(bHasTex)
    {
        computeTextureCoord(pGeometry);
    }

    osg::ref_ptr<osg::DrawArrays>   pPrim = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0u, pVertices->size());
    pGeometry->addPrimitiveSet(pPrim.get());

    return pGeometry.release();
}

void PointParameterNode::traverse(osg::NodeVisitor& nv)
{
    if(nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        return Group::traverse(nv);
    }

    if(m_bFollowTerrain && m_bHasIntered)
    {
        osg::Vec3d vTrans;
        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        pEllipsoidModel->convertLatLongHeightToXYZ(m_InterPosition._v[1], m_InterPosition._v[0], m_InterPosition._v[2] + m_dblHeight, vTrans._v[0], vTrans._v[1], vTrans._v[2]);
        osg::Matrixd mtx;
        mtx.setTrans(vTrans);

        osg::MatrixTransform *pMatrixTransform = dynamic_cast<osg::MatrixTransform *>(getChild(0));
        pMatrixTransform->setMatrix(mtx);
        m_bHasIntered = false;
        m_pParmRectifyRequest = NULL;

        return Group::traverse(nv);
    }

    if(m_bFollowTerrain)
    {
        FileReadInterceptor *pFileReadInterceptor = NULL;
        const unsigned nCallbackCount = osgDB::Registry::instance()->getReadFileCallbackCount();
        for(unsigned n = 0u; n < nCallbackCount; n++)
        {
            pFileReadInterceptor = dynamic_cast<FileReadInterceptor *>(osgDB::Registry::instance()->getReadFileCallback(n));
            if(pFileReadInterceptor)
            {
                break;
            }
        }
        if(pFileReadInterceptor == NULL)
        {
            return Group::traverse(nv);
        }

        if(pFileReadInterceptor->getLastTerrainUpdate() + 1 > osg::Timer::instance()->time_s())
        {
            ParmRectifyThreadPool *pThreadPool = Registry::instance()->getParmRectifyThreadPool();
            pThreadPool->requestParmRectify(this, m_pParmRectifyRequest);
        }
    }

    return Group::traverse(nv);
}

osg::Node *PointParameterNode::createBubbleTextDetail(const param::IDetail *pDetail) const
{
    const param::IBubbleTextDetail *pBTD = dynamic_cast<const param::IBubbleTextDetail *>(pDetail);
    if (!pBTD) return NULL;

    BubbleTextBuilder btb(pBTD);
    return btb.Build(osg::Vec3d(0,0,0));
}

osg::Node *PointParameterNode::createPolygonDetail(const param::IDetail *pDetail) const
{
    const param::IPolygonDetail *pPD = dynamic_cast<const param::IPolygonDetail *>(pDetail);
    if (!pPD) return NULL;

    if(pPD->getVertexCount() < 2u)
    {
        return NULL;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptPosition.y(), m_ptPosition.x(), m_ptPosition.z(), ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    osg::Vec3d vecPositionUp = ptCenterPos;
    vecPositionUp.normalize();

    osg::Quat   qtRotation;

    const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
    osg::Quat   qtToAxisZ;
    qtToAxisZ.makeRotate(vecPositionUp, vecAxisZ);
    qtRotation *= qtToAxisZ;

    const osg::Vec3d    vecEastern(-sin(m_ptPosition.x()), cos(m_ptPosition.x()), 0.0);
    const osg::Vec3d    vecAxisX(1.0, 0.0, 0.0);
    osg::Quat  qtToAxisX;
    qtToAxisX.makeRotate(vecEastern, vecAxisX);
    qtRotation *= qtToAxisX;

    ptCenterPos = qtRotation * ptCenterPos;

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> pVtxArray = new osg::Vec3Array;
    for(unsigned i = 0; i < pPD->getVertexCount(); i++)
    {
        cmm::math::Vector2d v;
        pPD->getVertex(i, v);

        osg::Vec3d dest;
        pEllipsoidModel->convertLatLongHeightToXYZ(v.y(), v.x(), m_ptPosition.z() + 1.0, dest.x(), dest.y(), dest.z());
        dest  = qtRotation * dest;
        dest -= ptCenterPos;

        pVtxArray->push_back(dest);
    }
    pGeometry->setVertexArray(pVtxArray);

    //线
    if(pPD->getBorderWidth() > 0.99)
    {
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, pVtxArray->size()));

        //线宽
        osg::LineWidth *linewidth = new osg::LineWidth(pPD->getBorderWidth());
        osg::StateSet *ss = pGeometry->getOrCreateStateSet();
        ss->setAttributeAndModes(linewidth);
    }

    //面
    if(pPD->getFaceVisible())
    {
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVtxArray->size()));
    }
    
    //颜色
    osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array();
    cmm::FloatColor bc = pPD->getBorderColor();
    cmm::FloatColor fc = pPD->getDynModelColor();

    if(pPD->getBorderWidth() > 0.99) pClrArray->push_back(osg::Vec4(bc.m_fltR, bc.m_fltG, bc.m_fltB, bc.m_fltA));
    if(pPD->getFaceVisible())        pClrArray->push_back(osg::Vec4(fc.m_fltR, fc.m_fltG, fc.m_fltB, fc.m_fltA));

    pGeometry->setColorArray(pClrArray);
    pGeometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

    //法线
    osg::ref_ptr<osg::Vec3dArray> pNormalArray = new osg::Vec3dArray;
    osg::Vec3d normal = ((*pVtxArray)[0] - (*pVtxArray)[1]) ^ ((*pVtxArray)[2] - (*pVtxArray)[1]);
    normal.normalize();
    pNormalArray->push_back(normal);

    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    //生成三角形
    osg::ref_ptr<osgUtil::Tessellator>  pTessellator = new osgUtil::Tessellator;
    pTessellator->retessellatePolygons(*pGeometry);

    osg::ref_ptr<osg::Geode> g = new osg::Geode;
    g->addDrawable(pGeometry);
    return g.release();
}
*/