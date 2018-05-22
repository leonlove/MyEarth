#include "ParamPoint.h"
#include <Common\CoordinateTransform.h>
#include <osg\CoordinateSystemNode>
#include <osg\Geode>
#include "Common/deuMath.h"
#include <ParameterSys\PointParameter.h>
#include <IDProvider/Definer.h>
#include "Builder.h"

ParamPoint::ParamPoint(unsigned short dataset_code)
{
    if (dataset_code <= 100)
    {
        throw std::string("�������ݼ���ű������100");
    }

    _id = ID::genNewID();
    _id.ObjectID.m_nDataSetCode = dataset_code;
    _id.ObjectID.m_nType = PARAM_POINT_ID;

    _base = dynamic_cast<param::PointParameter*>(param::createParameter(_id));
}

ParamPoint::ParamPoint(param::IPointParameter *base)
{
    if (base == NULL)
    {
        throw std::string("���󣺵��������Ϊ��");
    }

    _base = base;
    _id   = base->getID();
}

ParamPoint::ParamPoint(const cmm::math::Point3d &pt, unsigned short dataset_code)
{
    if (dataset_code <= 100)
    {
        throw std::string("�������ݼ���ű������100");
    }

    _id = ID::genNewID();
    _id.ObjectID.m_nDataSetCode = dataset_code;
    _id.ObjectID.m_nType = PARAM_POINT_ID;

    _base = dynamic_cast<param::PointParameter*>(param::createParameter(_id));
    _base->setCoordinate(pt);
}

bool ParamPoint::fromModelSymbol(Model &m)
{
    osg::Vec3d center = m.mt()->getMatrix().getTrans();

    //��γ�ȸ߶����������ĵ�
    cmm::math::Point3d llh(0.0, 0.0, 0.0);
    osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(center.x(), center.y(), center.z(), llh.y(), llh.x(), llh.z());

    _base->setCoordinate(llh);
    _base->setAzimuthAngle(0.0);
    _base->setPitchAngle(0.0);
    
    param::PointParameter *p = dynamic_cast<param::PointParameter*>(_base.get());
    p->setRadius(m.mt()->getBound().radius());
    m.getDetail(_base.get());
    
    _contains_model = true;
    return true;
}

const ID& ::ParamPoint::getID()const
{
    return _id;
}

cmm::math::Sphered ParamPoint::getBoundingSphere(void)
{
    if (_bs.getRadius() < 0.001)
    {
        _bs = _base->getBoundingSphere();
        _bs.setCenter(_base->getCoordinate());
    }

    return _bs;
}

bool ParamPoint::writeToDB(const ID &idParent, deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("����db����Ϊ��");
    }

    bson::bsonDocument doc;
    param::Parameter *pParam = dynamic_cast<param::Parameter *>(_base.get());
    if (!pParam->toBson(doc))
    {
        return false;
    }

    bson::bsonStream bs;
    if (!doc.Write(&bs))
    {
        return false;
    }

    return db->addBlock(getID(), bs.Data(), bs.DataLen());
}

param::IParameter* ParamPoint::base()
{
    return dynamic_cast<param::IParameter*>(_base.get());
}