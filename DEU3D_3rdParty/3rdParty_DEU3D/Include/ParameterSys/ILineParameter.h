#ifndef I_LINE_PARAMETER_H_FB4103D6_C1E6_4DF7_BE06_13D5E55CFC49_INCLUDE
#define I_LINE_PARAMETER_H_FB4103D6_C1E6_4DF7_BE06_13D5E55CFC49_INCLUDE

#include "Export.h"
#include "IParameter.h"
#include "Common/deuMath.h"

namespace param
{

class ILineParameter : virtual public IParameter
{
public:
    virtual void                            addCoordinate(const cmm::math::Point3d &point)                          = 0;
    virtual bool                            insertCoordinate(unsigned nIndex, const cmm::math::Point3d &point)      = 0;
    virtual bool                            setCoordinate(unsigned nIndex, const cmm::math::Point3d &point)         = 0;
    virtual bool                            removeCoordinate(unsigned nIndex)                                       = 0;
    virtual unsigned int                    getCoordinateCount(void) const                                          = 0;
    virtual const cmm::math::Point3d       &getCoordinate(unsigned nIndex) const                                    = 0;
    virtual void                            setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates)    = 0;
    virtual const std::vector<cmm::math::Point3d> &getCoordinates(void) const                                       = 0;

    virtual void                            addPart(unsigned nOffset, unsigned nCount)                              = 0;
    virtual bool                            getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const      = 0;
    virtual unsigned int                    getPartCount(void) const                                                = 0;

    virtual void                            setTerrainMagnet(bool bTerrainMagnet)                                   = 0;
    virtual bool                            getTerrainMagnet(void) const                                            = 0;

	virtual void                            addColor(const cmm::math::Vector3d &color)								= 0;
};

}

#endif

