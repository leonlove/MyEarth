#include "DynPointCloudDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/SharedStateAttributes>

#include <osg/PointSprite>

namespace param
{

DynPointCloudDetail::DynPointCloudDetail(void)
{
    m_dblPointSize = 1.0;
    m_PointClr.m_fltR = 1.0;
    m_PointClr.m_fltG = 1.0;
    m_PointClr.m_fltB = 1.0;
    m_PointClr.m_fltA = 1.0;
}

DynPointCloudDetail::DynPointCloudDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_DYN_POINT_CLOUD_ID))
{
    m_dblPointSize = 1.0;
    m_PointClr.m_fltR = 1.0;
    m_PointClr.m_fltG = 1.0;
    m_PointClr.m_fltB = 1.0;
    m_PointClr.m_fltA = 1.0;
}

DynPointCloudDetail::~DynPointCloudDetail(void)
{

}

bool DynPointCloudDetail::fromBson(bson::bsonDocument &bsonDoc)
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

bool DynPointCloudDetail::toBson(bson::bsonDocument &bsonDoc) const
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

double DynPointCloudDetail::getBoundingSphereRadius(void) const
{
    return m_dblPointSize;
}


osg::Node *DynPointCloudDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCloudCreationInfo *pPointCloudInfo = dynamic_cast<const PointCloudCreationInfo*>(pInfo);
    if(pPointCloudInfo == NULL)
    {
        return NULL;
    }

	osg::ref_ptr<osg::Node> pPointCloudNode = createPointCloudNode(pPointCloudInfo);

	return pPointCloudNode.release();
}

osg::Geode* DynPointCloudDetail::createPointCloudNode(const PointCloudCreationInfo* pPointCloudInfo) const
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> galaxy = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

	for(unsigned int i = 0; i < pPointCloudInfo->m_pPoints->size(); i++)
	{
		osg::Vec3d point;
		const osg::Vec3d &coord = pPointCloudInfo->m_pPoints->at(i);
		pEllipsoidModel->convertLatLongHeightToXYZ(coord.y(), coord.x(), coord.z(), point.x(), point.y(), point.z());
		vertices->push_back(point);

		if (i < pPointCloudInfo->m_pColors->size())
		{
			const osg::Vec3d &color = pPointCloudInfo->m_pColors->at(i);
			colors->push_back(osg::Vec4(color.x(), color.y(), color.z(), 1.0));
		}
	}

	galaxy->setVertexArray(vertices);
	galaxy->setColorArray(colors);

	if (colors->size() == 0)
	{
		colors->push_back(osg::Vec4(m_PointClr.m_fltR, m_PointClr.m_fltG, m_PointClr.m_fltB, m_PointClr.m_fltA));
		galaxy->setColorBinding(osg::Geometry::BIND_OVERALL);
	}
	else
	{
		galaxy->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}
	galaxy->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, pPointCloudInfo->m_pPoints->size()));

	osg::StateSet* pStateSet = galaxy->getOrCreateStateSet();
	pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	if(m_dblPointSize > 1.0)
	{
	    osg::Point *pPoint = osg::SharedStateAttributes::instance()->getPoint(m_dblPointSize);
	    pStateSet->setAttribute(pPoint);
	}

	geode->addDrawable(galaxy);

	return geode.release();
}

}
