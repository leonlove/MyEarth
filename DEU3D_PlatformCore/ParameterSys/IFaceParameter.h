#ifndef I_FACE_PARAMETER_H_43CB72B5_431C_435C_9750_0E409A72476F_INCLUDE
#define I_FACE_PARAMETER_H_43CB72B5_431C_435C_9750_0E409A72476F_INCLUDE

#include "Export.h"
#include "IParameter.h"
#include "Common/deuMath.h"

namespace param
{

class IFaceParameter : virtual public IParameter
{
public:
    virtual void                            addCoordinate(const cmm::math::Point3d &point)                          = 0;
    virtual bool                            insertCoordinate(unsigned nIndex, const cmm::math::Point3d &point)      = 0;
    virtual bool                            setCoordinate(unsigned nIndex, const cmm::math::Point3d &point)         = 0;
    virtual bool                            removeCoordinate(unsigned nIndex)                                       = 0;
    virtual unsigned                        getCoordinateCount(void) const                                          = 0;
    virtual const cmm::math::Point3d       &getCoordinate(unsigned nIndex) const                                    = 0;
    virtual void                            setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates)    = 0;
    virtual const std::vector<cmm::math::Point3d> &getCoordinates(void) const                                       = 0;

    virtual void                            addPart(unsigned nOffset, unsigned nCount)                              = 0;
    virtual bool                            getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const      = 0;
    virtual unsigned int                    getPartCount(void) const                                                = 0;
};

}

#endif
