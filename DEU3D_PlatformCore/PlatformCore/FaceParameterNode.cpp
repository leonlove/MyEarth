#include "FaceParameterNode.h"

#include <ParameterSys/IFaceParameter.h>
#include <ParameterSys/IDetail.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgUtil/Tessellator>
#include <osg/CullFace>


FaceParameterNode::FaceParameterNode(param::IParameter *pParameter) :
    ParameterNode(pParameter)
{
}

FaceParameterNode::~FaceParameterNode(void)
{
}

bool FaceParameterNode::initFromParameter()
{
    if(!ParameterNode::initFromParameter())
    {
        return false;
    }

    param::IFaceParameter *pFaceParamter = dynamic_cast<param::IFaceParameter *>(m_pParameter.get());

    if(pFaceParamter == NULL)
    {
        return false;
    }

    m_vecPoints.clear();

    std::vector<cmm::math::Point3d> vecCoords = pFaceParamter->getCoordinates();

    osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
    for(unsigned int i = 0; i < vecCoords.size(); i++)
    {
        pVertex->push_back(osg::Vec3d(vecCoords[i].x(), vecCoords[i].y(), vecCoords[i].z()));
    }

    std::vector<std::pair<unsigned int, unsigned int> > vecPart;
    for(unsigned int i = 0; i < pFaceParamter->getPartCount(); ++i)
    {
        unsigned int nOffset, nCount;
        pFaceParamter->getPart(i, nOffset, nCount);
        vecPart.push_back(std::make_pair(nOffset, nCount));
    }

    if(vecPart.empty())
    {
        m_vecPoints.push_back(pVertex);
    }
    else
    {
        for(unsigned int i = 0; i < vecPart.size(); i++)
        {
            osg::ref_ptr<osg::Vec3dArray> pTempVertex = new osg::Vec3dArray(pVertex->begin() + vecPart[i].first, pVertex->begin() + vecPart[i].first + vecPart[i].second);
            m_vecPoints.push_back(pTempVertex);
        }
    }

    removeChildren(0, getNumChildren());
    addChild(createNodeByParameter(m_vecPoints));
    return true;
}

osg::Node *FaceParameterNode::createNodeByParameter(const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    param::ISymbol *pSymbol = m_pParameter->getSymbol();

    osg::ref_ptr<osg::LOD> pLOD = new osg::LOD;

    for(unsigned int i = 0; i < pSymbol->getNumDetail(); i++)
    {
        OpenSP::sp<param::IDetail> pDetail = pSymbol->getDetail(i);

        addChildByDetail(pLOD, pDetail, vecPoints);
    }

    return pLOD.release();
}

void FaceParameterNode::addChildByDetail(osg::LOD *pLOD, const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    if(pDetail == NULL)
    {
        return;
    }

    double dblMin, dblMax;
    pDetail->getRange(dblMin, dblMax);

    const std::string &strType = pDetail->getStyle();

    osg::ref_ptr<osg::Node> pDetailNode = NULL;
    if(strType.compare(param::FACE_DETAIL) == 0)
    {
        pDetailNode = createFaceDetail(pDetail, vecPoints);
    }

    if(pDetailNode.valid())
    {
        pLOD->addChild(pDetailNode, dblMin, dblMax);
    }
}

osg::Node *FaceParameterNode::createFaceDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    const param::IDynFaceDetail *pFaceDetail = dynamic_cast<const param::IDynFaceDetail *>(pDetail);
    if(pFaceDetail == NULL)
    {
        return NULL;
    }

    cmm::FloatColor face_color = pFaceDetail->getFaceColor();
    cmm::FloatColor border_color = pFaceDetail->getBorderColor();

    double dblBorderWidth = pFaceDetail->getBorderWidth();

    osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;
    std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator itor = vecPoints.begin();

    //计算所有点的平均值，用于平移矩阵
    osg::Vec3 vTempPoint;
    unsigned int nCount = 0;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    for( ; itor != vecPoints.end(); ++itor)
    {
        //osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
        unsigned int nSize = (*itor)->size();
        nCount += nSize;
        for(unsigned int i = 0; i < nSize; i++)
        {
            osg::Vec3d &pt1 = (*itor)->at(i);
            osg::Vec3d pt2;
            pEllipsoidModel->convertLatLongHeightToXYZ(pt1._v[1], pt1._v[0], pt1._v[2] + m_dblHeight, pt2._v[0], pt2._v[1], pt2._v[2]);
            vTempPoint += pt2;
            pVertex->push_back(pt2);
        }
    }

    //偏移量
    vTempPoint /= (float)nCount;

    //对每个点进行平移
    unsigned int nSize = pVertex->size();
    for(unsigned int i = 0; i < nSize; i++)
    {
        (*pVertex)[i] -= vTempPoint;
    }

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::Matrix matrix;
    matrix.setTrans(vTempPoint);
    pMatrixTransform->setMatrix(matrix);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pMatrixTransform->addChild(pGeode.get());

    //画边
    {
        osg::ref_ptr<osg::Geometry> pBorderGeometry = new osg::Geometry;
        pBorderGeometry->setVertexArray(pVertex);
        osg::ref_ptr<osg::Vec4Array> pBorderColorArray = new osg::Vec4Array(1);
        (*pBorderColorArray)[0].set(border_color.m_fltR, border_color.m_fltG, border_color.m_fltB, border_color.m_fltA);
        pBorderGeometry->setColorArray(pBorderColorArray);
        pBorderGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        itor = vecPoints.begin();
        unsigned int nIndex = 0;
        for(; itor != vecPoints.end(); ++itor)
        {
            nSize = (*itor)->size();
            pBorderGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, nIndex, nSize));
            nIndex += nSize;
        }

        pGeode->addDrawable(pBorderGeometry.get());
    }

    //画面
    {
        osg::ref_ptr<osg::Geometry> pFaceGeometry = new osg::Geometry;
        pFaceGeometry->setVertexArray(pVertex);
        osg::ref_ptr<osg::Vec4Array> pFaceColorArray = new osg::Vec4Array(1);
        (*pFaceColorArray)[0].set(face_color.m_fltR, face_color.m_fltG, face_color.m_fltB, face_color.m_fltA);
        pFaceGeometry->setColorArray(pFaceColorArray);
        pFaceGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        itor = vecPoints.begin();
        unsigned int nIndex = 0;
        for(; itor != vecPoints.end(); ++itor)
        {
            nSize = (*itor)->size();
            pFaceGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, nIndex, nSize));
            nIndex += nSize;
        }

        osg::ref_ptr<osgUtil::Tessellator> pTessellator = new osgUtil::Tessellator;
        pTessellator->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        pTessellator->setBoundaryOnly(false);
        pTessellator->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
        pTessellator->retessellatePolygons(*pFaceGeometry);

        pGeode->addDrawable(pFaceGeometry.get());
    }

    osg::StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();
    pStateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::OFF);
    pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    return pMatrixTransform.release();
}