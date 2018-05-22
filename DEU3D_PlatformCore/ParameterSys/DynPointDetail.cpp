#include "DynPointDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/SharedStateAttributes>

namespace param
{

DynPointDetail::DynPointDetail(void)
{
    m_dblPointSize = 1.0;
    m_PointClr.m_fltR = 1.0;
    m_PointClr.m_fltG = 1.0;
    m_PointClr.m_fltB = 1.0;
    m_PointClr.m_fltA = 1.0;
}

DynPointDetail::DynPointDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_DYN_POINT_ID))
{
    m_dblPointSize = 1.0;
    m_PointClr.m_fltR = 1.0;
    m_PointClr.m_fltG = 1.0;
    m_PointClr.m_fltB = 1.0;
    m_PointClr.m_fltA = 1.0;
}

DynPointDetail::~DynPointDetail(void)
{

}

bool DynPointDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Detail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;
    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Size"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setPointSize(pDoubleEle->DblValue());


    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Color"));
    if(pArrayEle == NULL || pArrayEle->ChildCount() != 4)
    {
        return false;
    }

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_PointClr.m_fltR = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_PointClr.m_fltG = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_PointClr.m_fltB = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_PointClr.m_fltA = pDoubleEle->DblValue();

    return true;
}

bool DynPointDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Size", m_dblPointSize))
    {
        return false;
    }

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Color"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_PointClr.m_fltR) ||
        !pArrayEle->AddDblElement(m_PointClr.m_fltG) ||
        !pArrayEle->AddDblElement(m_PointClr.m_fltB) ||
        !pArrayEle->AddDblElement(m_PointClr.m_fltA))
    {
        return false;
    }

    return true;
}

double DynPointDetail::getBoundingSphereRadius(void) const
{
    return m_dblPointSize;
}


osg::Node *DynPointDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    //ÉÏÇò¾ØÕó
    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::Vec3d vecTrans;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    pMatrixTransform->setMatrix(osg::Matrix::translate(vecTrans));

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pMatrixTransform->addChild(pGeode);

    osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;
    pVertex->push_back(osg::Vec3(0.0, 0.0, 0.0));

    osg::Vec4 vClr(m_PointClr.m_fltR, m_PointClr.m_fltG, m_PointClr.m_fltB, m_PointClr.m_fltA);
    osg::ref_ptr<osg::Vec4Array> pColorArray = osg::SharedStateAttributes::instance()->getColorArrayByColor(vClr);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeode->addDrawable(pGeometry);

    pGeometry->setVertexArray(pVertex.get());
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, pVertex->size()));

    osg::StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();

    if(m_dblPointSize > 1.0)
    {
        osg::Point *pPoint = osg::SharedStateAttributes::instance()->getPoint(m_dblPointSize);
        pStateSet->setAttribute(pPoint);
    }

    return pMatrixTransform.release();
}

}
