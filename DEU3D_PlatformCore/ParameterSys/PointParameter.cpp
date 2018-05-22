#include "PointParameter.h"

#include <osg/MatrixTransform>
#include <osg/PagedLOD>
#include <osg/PagedLOD_MultiCenter>
#include <osg/CoordinateSystemNode>
#include <osgUtil/CommonOSG.h>
#include "Detail.h"
#include <iostream>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>

namespace param
{

PointParameter::PointParameter(const ID &id) : 
    Parameter(id),
    m_dblAzimuthAngle(0.0),
    m_dblPitchAngle(0.0),
	m_dblRollAngle(10505.0),
    m_scale(1.0, 1.0, 1.0),
    m_dRadius(-1.0)
{
}


PointParameter::~PointParameter(void)
{
}

bool PointParameter::fromBson(bson::bsonDocument &bsonDoc)
{
     if(!Parameter::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;

    //坐标
    {
        bson::bsonArrayEle *pCoordinateEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Coordinate"));

        if(pCoordinateEle == NULL)
        {
            return false;
        }

        if(pCoordinateEle->ChildCount() != 1)
        {
            return false;
        }

        bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pCoordinateEle->GetElement(0u));
        bson::bsonDocument &doc = pDocEle->GetDoc();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("x"));
        double dblX = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("y"));
        double dblY = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("z"));
        double dblZ = pDoubleEle->DblValue();

        setCoordinate(cmm::math::Point3d(dblX, dblY, dblZ));
    }

    //方位角
    {
        bson::bsonDoubleEle *pAzimuthAngleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("AzimuthAngle"));
        if(pAzimuthAngleEle != NULL)
        {
            setAzimuthAngle(pAzimuthAngleEle->DblValue());
        }
    }

    //倾斜角
    { 
        bson::bsonDoubleEle *pPitchAngleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("PitchAngle"));
        if(pPitchAngleEle != NULL)
        {
            setPitchAngle(pPitchAngleEle->DblValue());
        }
    }

	//QuatZ
	{ 
		bson::bsonDoubleEle *pRollAngleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("RollAngle"));
		if(pRollAngleEle != NULL)
		{
			setRollAngle(pRollAngleEle->DblValue());
		}
	}

    //??
    { 
        bson::bsonArrayEle *pScaleArray = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Scale"));
        if(pScaleArray != NULL)
        {
            setScale(cmm::math::Point3d(pScaleArray->GetElement(0u)->DblValue(), pScaleArray->GetElement(1u)->DblValue(), pScaleArray->GetElement(2u)->DblValue()));
        }
    }

    bson::bsonDoubleEle *pRadius = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Radius"));
    if (pRadius == NULL)
    {
        return false;
    }
    m_dRadius = pRadius->DblValue();

    // 参考点
    {
        bson::bsonArrayEle *pRefPointsArray = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("RefPoints"));
        if(pRefPointsArray == NULL)
        {
            // 允许没有参考点
            return true;
        }

        const unsigned nPointsCount = pRefPointsArray->ChildCount();
        for(unsigned n = 0u; n < nPointsCount; n++)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pRefPointsArray->GetElement(n));
            bson::bsonDocument &doc = pDocEle->GetDoc();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("x"));
            const double dblX = pDoubleEle->DblValue();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("y"));
            const double dblY = pDoubleEle->DblValue();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("z"));
            const double dblZ = pDoubleEle->DblValue();

            addRefPoint(cmm::math::Point3d(dblX, dblY, dblZ));
        }
    }

    return true;
}


bool PointParameter::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Parameter::toBson(bsonDoc))
    {
        return false;
    }

    bson::bsonArrayEle *pCoordinateEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Coordinate"));
    if(pCoordinateEle == NULL)
    {
        return false;
    }

    bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pCoordinateEle->AddDocumentElement());
    if(pDocEle == NULL)
    {
        return false;
    }

    bson::bsonDocument &doc = pDocEle->GetDoc();
    if(!doc.AddDblElement("x", m_ptCoordinate.x()) ||
       !doc.AddDblElement("y", m_ptCoordinate.y()) ||
       !doc.AddDblElement("z", m_ptCoordinate.z()))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("AzimuthAngle", m_dblAzimuthAngle) ||
       !bsonDoc.AddDblElement("PitchAngle", m_dblPitchAngle) ||
	   !bsonDoc.AddDblElement("RollAngle", m_dblRollAngle))
    {
        return false;
    }

    {
        bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Scale"));
        pArrayEle->AddDblElement(m_scale.x());
        pArrayEle->AddDblElement(m_scale.y());
        pArrayEle->AddDblElement(m_scale.z());
    }

    bsonDoc.AddDblElement("Radius", m_dRadius);

    //添加属性数据
    bsonDoc.AddStringElement("ID", getID().toString().c_str());

    //包围球
    cmm::math::Sphered bound = getBoundingSphere();
    bson::bsonArrayEle *pBoundingSphere = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BoundingSphere"));
    pBoundingSphere->AddDblElement(bound.getCenter().x());
    pBoundingSphere->AddDblElement(bound.getCenter().y());
    pBoundingSphere->AddDblElement(bound.getCenter().z());
    pBoundingSphere->AddDblElement(bound.getRadius());

    //最大可视范围
    bsonDoc.AddDblElement("MaxRange", getMaxRange());

    if(!m_vecRefPoints.empty())
    {
        bson::bsonArrayEle *pRefPoitnsArray = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("RefPoints"));
        for(std::vector<cmm::math::Point3d>::const_iterator itor = m_vecRefPoints.begin(); itor != m_vecRefPoints.end(); ++itor)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pRefPoitnsArray->AddDocumentElement());
            if(pDocEle == NULL)
            {
                continue;
            }

            bson::bsonDocument &doc = pDocEle->GetDoc();
            if(!doc.AddDblElement("x", itor->x()) ||
                !doc.AddDblElement("y", itor->y()) ||
                !doc.AddDblElement("z", itor->z()))
            {
                return false;
            }
        }
    }

    return true;
}


void PointParameter::setCoordinate(const cmm::math::Point3d &point)
{
    m_ptCoordinate = point;
}

const cmm::math::Point3d &PointParameter::getCoordinate(void) const
{
    return m_ptCoordinate;
}


void PointParameter::addRefPoint(const cmm::math::Point3d &point)
{
    m_vecRefPoints.push_back(point);
}


void PointParameter::clearRefPoint(void)
{
    m_vecRefPoints.clear();
}


void PointParameter::setAzimuthAngle(double dblAzimuth)
{
    m_dblAzimuthAngle = dblAzimuth;
}


double PointParameter::getAzimuthAngle(void) const
{
    return m_dblAzimuthAngle;
}


void PointParameter::setPitchAngle(double dblPitch)
{
    //m_dblPitchAngle = cmm::math::clampBetween(dblPitch, 0.0, cmm::math::PI);
	m_dblPitchAngle = dblPitch;
}


double PointParameter::getPitchAngle(void) const
{
    return m_dblPitchAngle;
}


void PointParameter::setRollAngle(double dblQuatZ)
{
	m_dblRollAngle = dblQuatZ;
}


double PointParameter::getRollAngle(void) const
{
	return m_dblRollAngle;
}


void PointParameter::setScale(const cmm::math::Point3d &scale)
{
    m_scale = scale;
}

const cmm::math::Point3d &PointParameter::getScale(void) const
{
    return m_scale;
}

cmm::math::Sphered PointParameter::getBoundingSphere(void) const
{
    cmm::math::Sphered s;
    cmm::math::Point3d center = m_ptCoordinate;
    center.z() += m_dblHeight;
    s.setCenter(center);
    s.setRadius(m_dRadius);
    return s;
}

osg::Node *PointParameter::createParameterNode(void) const
{
    std::vector<ID> vecDetails;
    std::vector<std::pair<double, double> > vecRanges;
    if(!getSortedDetails(vecDetails, vecRanges))
    {
        return NULL;
    }

    osg::ref_ptr<osg::Group> pParamRoot = new osg::Group;

    osg::ref_ptr<osg::PagedLOD> pPagedLOD;
    cmm::math::Sphered sphere = getBoundingSphere();
    const cmm::math::Point3d &ptCenterCoord = sphere.getCenter();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptCenter;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), ptCenterCoord.z(), ptCenter.x(), ptCenter.y(), ptCenter.z());

    double dblRadiusGain = 0.0;
    double dblRadius = sphere.getRadius();
    if(m_vecRefPoints.empty())
    {
        pPagedLOD = new osg::PagedLOD;
    }
    else
    {
#if 0
        osg::ref_ptr<osg::Sphere>           pSphere   = new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 5.0f);
        osg::ref_ptr<osg::ShapeDrawable>    pDrawable = new osg::ShapeDrawable(pSphere);
        osg::ref_ptr<osg::Geode>            pGeode    = new osg::Geode;
        pGeode->addDrawable(pDrawable.get());

        osg::ref_ptr<osg::Group>    pGroup = new osg::Group;
        for(unsigned n = 0u; n < m_vecRefPoints.size(); n++)
        {
            const cmm::math::Point3d &ref = m_vecRefPoints.at(n);

            osg::Vec3d vecTrans;
            pEllipsoidModel->convertLatLongHeightToXYZ(ref.y(), ref.x(), ref.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

            osg::ref_ptr<osg::MatrixTransform>  pMtxTrans = new osg::MatrixTransform(osg::Matrix::translate(vecTrans));
            pMtxTrans->addChild(pGeode.get());
            pGroup->addChild(pMtxTrans.get());
        }

        pParamRoot->addChild(pGroup.get());
#endif
        osg::ref_ptr<osg::PagedLOD_MultiCenter> pPagedLOD_MC = new osg::PagedLOD_MultiCenter;

        for(std::vector<cmm::math::Point3d>::const_iterator itor = m_vecRefPoints.begin(); itor != m_vecRefPoints.end(); ++itor)
        {
            const cmm::math::Point3d &ref = *itor;

            osg::Vec3d ptRef;
            pEllipsoidModel->convertLatLongHeightToXYZ(ref.y(), ref.x(), ref.z(), ptRef.x(), ptRef.y(), ptRef.z());
            pPagedLOD_MC->addReferencedPoint(ptRef);

            const double dblDis = (ptRef - ptCenter).length();
            dblRadiusGain = osg::maximum(dblRadiusGain, dblDis);
        }
        pPagedLOD = pPagedLOD_MC;
    }

    pPagedLOD->setCenter(ptCenter);
    pPagedLOD->setRadius(dblRadius + dblRadiusGain);

    for(unsigned int i = 0; i < vecDetails.size(); i++)
    {
        pPagedLOD->setFileID(i, vecDetails[i]);
        pPagedLOD->setRange(i, vecRanges[i].first, vecRanges[i].second);
    }

    osg::ref_ptr<Detail::PointCreationInfo> pPointCreationInfo = new Detail::PointCreationInfo;

	pPointCreationInfo->m_pPoints = new osg::Vec3dArray;
    pPointCreationInfo->m_pPoints->push_back(osg::Vec3d(m_ptCoordinate.x(), m_ptCoordinate.y(), m_ptCoordinate.z()));

	pPointCreationInfo->m_dblAzimuthAngle	= m_dblAzimuthAngle;
	pPointCreationInfo->m_dblPitchAngle		= m_dblPitchAngle;
	pPointCreationInfo->m_dblRollAngle		= m_dblRollAngle;
    pPointCreationInfo->m_Scale				= osg::Vec3d(m_scale.x(), m_scale.y(), m_scale.z());

    pPagedLOD->setChildCreationInfo(pPointCreationInfo.get());

    pParamRoot->addChild(pPagedLOD.get());

    return pParamRoot.release();
}

}

