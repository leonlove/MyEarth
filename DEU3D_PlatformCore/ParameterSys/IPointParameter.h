#ifndef I_POINT_PARAMETER_H_6362AF78_F794_4263_AD79_276205A269BC_INCLUDE
#define I_POINT_PARAMETER_H_6362AF78_F794_4263_AD79_276205A269BC_INCLUDE

#include "IParameter.h"
#include <Common/deuMath.h>

namespace param
{

class IPointParameter : virtual public IParameter
{
public:
    virtual void    setCoordinate(const cmm::math::Point3d &point) = 0;
    virtual const   cmm::math::Point3d &getCoordinate(void) const = 0;

    virtual void    addRefPoint(const cmm::math::Point3d &point) = 0;
    virtual void    clearRefPoint(void) = 0;

    virtual void    setAzimuthAngle(double dblAzimuth) = 0;
    virtual double  getAzimuthAngle(void) const = 0;

    virtual void    setPitchAngle(double dblPitch) = 0;
    virtual double  getPitchAngle(void) const = 0;

	virtual void    setRollAngle(double dblQuatZ) = 0;
	virtual double  getRollAngle(void) const = 0;

    virtual void    setScale(const cmm::math::Point3d &scale)   = 0;
    virtual const   cmm::math::Point3d &getScale(void) const = 0;
};

}

#endif
