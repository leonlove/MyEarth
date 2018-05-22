#include "PolygonDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/CommonOSG.h>
#include <osgUtil/Tessellator>

namespace param
{

PolygonDetail::PolygonDetail(void)
{
    m_bBorderVisible = true;
    m_bFaceVisible = false;
    m_dblBorderWidth = 1.0;
}


PolygonDetail::PolygonDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_POLYGON_ID)
{
    m_bBorderVisible = true;
    m_bFaceVisible = false;
    m_dblBorderWidth = 1.0;
}


PolygonDetail::~PolygonDetail(void)
{

}


bool PolygonDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    //顶点
    bson::bsonArrayEle *pVertices = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement("Vertices"));
    if(pVertices == NULL || pVertices->GetType() != bson::bsonArrayType)
    {
        return false;
    }

    for(unsigned i = 0; i < pVertices->ChildCount(); i++)
    {
        bson::bsonDocumentEle *pVertex = dynamic_cast<bson::bsonDocumentEle*>(pVertices->GetElement(i));
        if (pVertex == NULL)
        {
            continue;
        }

        bson::bsonDocument &vertex_doc = pVertex->GetDoc();
        bson::bsonDoubleEle *pCoordX = dynamic_cast<bson::bsonDoubleEle*>(vertex_doc.GetElement("x"));
        bson::bsonDoubleEle *pCoordY = dynamic_cast<bson::bsonDoubleEle*>(vertex_doc.GetElement("y"));

        if (pCoordX == NULL || pCoordY ==NULL)
        {
            continue;
        }

        addVertex(cmm::math::Point2d(pCoordX->DblValue(), pCoordY->DblValue()));
    }

    bson::bsonBinaryEle *pColor = dynamic_cast<bson::bsonBinaryEle*>(bsonDoc.GetElement("BorderColor"));
    if(pColor == NULL)
    {
        return false;
    }
    memcpy(&m_clrBorder, pColor->BinData(), pColor->BinDataLen());

    bson::bsonDoubleEle *pBorderWidth = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("BorderWidth"));
    if(pBorderWidth == NULL)
    {
        return false;
    }
    m_dblBorderWidth = pBorderWidth->DblValue();

    bson::bsonBoolEle *pFaceVisible = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("FaceVisible"));
    if(pFaceVisible == NULL)
    {
        return false;
    }
    m_bFaceVisible = pFaceVisible->BoolValue();

    bson::bsonBoolEle *pBorderVisible = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("BorderVisible"));
    if(pFaceVisible == NULL)
    {
        return false;
    }
    m_bFaceVisible = pFaceVisible->BoolValue();

    return true;
}


bool PolygonDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if( !bsonDoc.AddBinElement("BorderColor", (void*)&m_clrBorder, sizeof(m_clrBorder)) ||
        !bsonDoc.AddDblElement("BorderWidth", m_dblBorderWidth) ||
        !bsonDoc.AddBoolElement("BorderVisible", m_bBorderVisible) ||
        !bsonDoc.AddBoolElement("FaceVisible", m_bFaceVisible))
    {
        return false;
    }

    bson::bsonArrayEle *vertices = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Vertices"));

    for(size_t i = 0; i < m_vecVertices.size(); i++)
    {
        bson::bsonDocumentEle *vertices_doc_ele = dynamic_cast<bson::bsonDocumentEle*>(vertices->AddDocumentElement());
        bson::bsonDocument    &vertices_doc     = vertices_doc_ele->GetDoc();

        if (!vertices_doc.AddDblElement("x", m_vecVertices[i].x())) continue;

        if (!vertices_doc.AddDblElement("y", m_vecVertices[i].y())) continue;
    }

    return true;
}


double PolygonDetail::getBoundingSphereRadius(void) const
{
    cmm::math::Sphered s;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    for(size_t i = 0; i < m_vecVertices.size(); i++)
    {
        cmm::math::Vector3d pt;
        pEllipsoidModel->convertLatLongHeightToXYZ(m_vecVertices[i].y(), m_vecVertices[i].x(), 0.0, pt.x(), pt.y(), pt.z());
        s.expandBy(pt);
    }

    return s.getRadius();
}


//创建多边形
osg::Node *PolygonDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    if(m_vecVertices.size() < 2u)
    {
        return NULL;
    }

    osg::Matrixd matrix;

    const osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern   = pEllipsoidModel->computeLocalEastern(vPoint.y(), vPoint.x());
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(vPoint.y(), vPoint.x());
    const osg::Quat  qtStand(osg::Vec3d(0.0, 0.0, -1.0), vecPlumbLine);

    const osg::Quat  qtAzimuth(pPointInfo->m_dblAzimuthAngle, -vecPlumbLine);

    const osg::Vec3d vecLocalRight = qtAzimuth * vecEastern;
    const osg::Quat  qtPitch(pPointInfo->m_dblPitchAngle, vecLocalRight);

    const osg::Quat  qtRotation = qtStand * qtAzimuth * qtPitch;
    matrix.postMultRotate(qtRotation);

    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    //osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);
    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;

    const osg::Quat qtStand1(vecPlumbLine, osg::Vec3d(0.0, 0.0, -1.0));
    osg::Vec3 vCenter;
    osg::ref_ptr<osg::Vec3Array> pCoordArray = new osg::Vec3Array;
    for(unsigned int i = 0; i < m_vecVertices.size(); i++)
    {
        const cmm::math::Point2d &vtx = m_vecVertices[i];

        osg::Vec3d vCoord;
        pEllipsoidModel->convertLatLongHeightToXYZ(vtx.y(), vtx.x(), vPoint.z(), vCoord.x(), vCoord.y(), vCoord.z());
        //vCoord -= vecTrans;
        //vCoord = qtStand1 * vCoord;
        pCoordArray->push_back(vCoord);
        vCenter += vCoord;
    }

    vCenter /= (float)(pCoordArray->size());
    osg::Matrix mtx;
    mtx.postMultTranslate(vCenter);
    pMatrixTransform->setMatrix(mtx);
    for(unsigned int i = 0; i < pCoordArray->size(); i++)
    {
        (*pCoordArray)[i] -= vCenter;
    }

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pMatrixTransform->addChild(pGeode.get());

    osgUtil::CommonModelCreater *pCommonModelCreater = osgUtil::CommonModelCreater::instance();
    //
    if(m_bBorderVisible)
    {
        osg::Vec4 clrBorder(m_clrBorder.m_fltR, m_clrBorder.m_fltG, m_clrBorder.m_fltB, m_clrBorder.m_fltA);
        osg::ref_ptr<osg::Geometry> pGeometry = pCommonModelCreater->createLine(pCoordArray, osg::PrimitiveSet::LINE_LOOP, m_dblBorderWidth, true, clrBorder);
        pGeode->addDrawable(pGeometry.get());
    }

    //面
    if(m_bFaceVisible)
    {
        osg::Vec4 clrFace(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA);
        osg::ref_ptr<osg::Geometry> pGeometry = pCommonModelCreater->createPolygon(pCoordArray, true, clrFace);
        osgUtil::Tessellator t;
        t.retessellatePolygons(*pGeometry.get());
        pGeode->addDrawable(pGeometry.get());
    }

    return pMatrixTransform.release();
}

}