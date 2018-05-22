#ifndef POINT_PARAMETER_H_2677C6C7_D108_47A1_8486_A056CAA582A0_INCLUDE
#define POINT_PARAMETER_H_2677C6C7_D108_47A1_8486_A056CAA582A0_INCLUDE

#include "IPointParameter.h"
#include "Common/deuMath.h"
#include "Parameter.h"

namespace param
{

class PointParameter : public Parameter, public IPointParameter
{
public:
    explicit        PointParameter(const ID &id);
    virtual         ~PointParameter(void);

    virtual cmm::math::Sphered  getBoundingSphere(void) const;

    //供批量ive生产时使用，效率高
    void setRadius(double dRadius){if (dRadius > m_dRadius) m_dRadius = dRadius;}

protected:
    virtual osg::Node *createParameterNode(void) const;
    virtual bool    fromBson(bson::bsonDocument &bsonDoc);
    virtual bool    toBson(bson::bsonDocument &bsonDoc) const;

protected:
    virtual void    setCoordinate(const cmm::math::Point3d &point);
    virtual const   cmm::math::Point3d &getCoordinate(void) const;

    virtual void    addRefPoint(const cmm::math::Point3d &point);
    virtual void    clearRefPoint(void);

    virtual void    setAzimuthAngle(double dblAzimuth);
    virtual double  getAzimuthAngle(void) const;

    virtual void    setPitchAngle(double dblPitch);
    virtual double  getPitchAngle(void) const;

	virtual void    setRollAngle(double dblQuatZ);
	virtual double  getRollAngle(void) const;

    virtual void    setScale(const cmm::math::Point3d &scale);
    virtual const   cmm::math::Point3d &getScale(void) const;

protected:
    cmm::math::Point3d      m_ptCoordinate;
    double                  m_dblAzimuthAngle;
    double                  m_dblPitchAngle;
	double					m_dblRollAngle;
    double                  m_dRadius;
    cmm::math::Point3d      m_scale;

    std::vector<cmm::math::Point3d>     m_vecRefPoints;
};

}

#endif
