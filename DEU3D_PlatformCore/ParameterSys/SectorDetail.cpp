#include "SectorDetail.h"

#include <IDProvider/Definer.h>

#include <osg/MatrixTransform>
#include <osg/PagedLOD>
#include <osg/CoordinateSystemNode>

#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/CommonOSG.h>
#include <osg/CullFace>
#include <osgUtil\CommonModelCreater.h>

namespace param
{

SectorDetail::SectorDetail(void)
{
}

SectorDetail::SectorDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_SECTOR_ID)
{
    m_dblBorderWidth = 0.0;
    m_dblBeginAngle = 0.0;
    m_dblRadius1 = 0.0;
    m_dblRadius2 = 0.0;
    m_dblEndAngle = cmm::math::PI * 2.0;
    m_clrBorderColor.m_fltR = 1.0;
    m_clrBorderColor.m_fltG = 1.0;
    m_clrBorderColor.m_fltB = 1.0;
    m_clrBorderColor.m_fltA = 1.0;
}


SectorDetail::~SectorDetail(void)
{

}


bool SectorDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Radius1"));
    if(pDoubleEle == NULL)  return false;
    const double dblRadius1 = pDoubleEle->DblValue();
    setRadius1(dblRadius1);

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Radius2"));
    if(pDoubleEle == NULL)  return false;
    const double dblRadius2 = pDoubleEle->DblValue();
    setRadius2(dblRadius2);

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("BeginAngle"));
    if(pDoubleEle == NULL)  return false;
    const double dblBeginAngle = pDoubleEle->DblValue();
    setBeginAngle(dblBeginAngle);

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("EndAngle"));
    if(pDoubleEle == NULL)  return false;
    const double dblEndAngle = pDoubleEle->DblValue();
    setEndAngle(dblEndAngle);

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("BorderWidth"));
    if(pDoubleEle == NULL)  return false;
    const double dblBorderWidth = pDoubleEle->DblValue();
    setBorderWidth(dblBorderWidth);

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("BorderColor"));
    if(pArrayEle == NULL || pArrayEle->ChildCount() != 4)
    {
        return false;
    }

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
    if(pDoubleEle == NULL)  return false;
    m_clrBorderColor.m_fltR = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
    if(pDoubleEle == NULL)  return false;
    m_clrBorderColor.m_fltG = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
    if(pDoubleEle == NULL)  return false;
    m_clrBorderColor.m_fltB = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
    if(pDoubleEle == NULL)  return false;
    m_clrBorderColor.m_fltA = pDoubleEle->DblValue();

    return true;
}


bool SectorDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Radius1", m_dblRadius1))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Radius2", m_dblRadius2))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("BeginAngle", m_dblBeginAngle))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("EndAngle", m_dblEndAngle))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("BorderWidth", m_dblBorderWidth))
    {
        return false;
    }


    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BorderColor"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_clrBorderColor.m_fltR) ||
        !pArrayEle->AddDblElement(m_clrBorderColor.m_fltG) ||
        !pArrayEle->AddDblElement(m_clrBorderColor.m_fltB) ||
        !pArrayEle->AddDblElement(m_clrBorderColor.m_fltA))
    {
        return false;
    }

    return true;
}


double SectorDetail::getBoundingSphereRadius(void) const
{
    return std::max(m_dblRadius1, m_dblRadius2);
}


//¥¥Ω®…»–Œ
osg::Node *SectorDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    osg::Matrixd matrix;
    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();
    const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), pPointInfo->m_dblAzimuthAngle, pPointInfo->m_dblPitchAngle);
    matrix.postMultRotate(qtRotation);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);
    osg::ref_ptr<osg::Node> pNode = osgUtil::CommonModelCreater::instance()->createSector(m_dblBeginAngle, 
                                                                                          m_dblEndAngle, 
                                                                                          m_dblRadius1,
                                                                                          m_dblRadius2,
                                                                                          false,
                                                                                          osg::Vec4(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA),
                                                                                          osg::Vec4(m_clrBorderColor.m_fltR, m_clrBorderColor.m_fltG, m_clrBorderColor.m_fltB, m_clrBorderColor.m_fltA),
                                                                                          m_dblBorderWidth);
    pMatrixTransform->addChild(pNode);
    return pMatrixTransform.release();
}

}