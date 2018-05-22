#ifndef I_PARAMETER_H_1CCB729D_0F8C_484F_A2CD_FB9AE6625791_INCLUDE
#define I_PARAMETER_H_1CCB729D_0F8C_484F_A2CD_FB9AE6625791_INCLUDE

#include "Export.h"
#include <Common\variant.h>
#include <common\DEUBson.h>
#include <common\deuMath.h>

namespace param
{

class IParameter : public OpenSP::Ref
{
public:
    virtual const ID               &getID(void) const                                                    = 0;
    virtual void                    setID(const ID &id)                                                  = 0;

    virtual void                    setFollowByTerrain(bool bFollow)                                    = 0;
    virtual bool                    isFollowByTerrain(void) const                                       = 0;

    virtual void                    setCoverOrder(unsigned nOrder)                                      = 0;
    virtual unsigned                getCoverOrder(void) const                                           = 0;

    virtual void                    setHeight(double dblHeight)                                         = 0;
    virtual double                  getHeight(void) const                                               = 0;

    virtual void                    setActProperty(unsigned nIndex)                                     = 0;
    virtual unsigned int            getActProperty(void) const                                          = 0;

    virtual unsigned int            addProperty(const std::string &strKey, const std::string &strProp)  = 0;
    virtual void                    getProperty(unsigned int nIndex, std::string &strKey, std::string &strProp) const = 0;
    virtual unsigned int            getPropertyCount(void) const                                        = 0;
    virtual unsigned int            findProperty(const std::string &strKey, std::string &strVal) const  = 0;

    virtual void                    addDetail(const ID &id, double dMinRange, double dMaxRange)         = 0;
    virtual bool                    getDetail(unsigned i, ID &id, double &dMinRange, double &dMaxRange) const  = 0;
    virtual unsigned                getNumDetail(void) const                                            = 0;
    virtual double                  getMaxRange(void) const                                             = 0;
    virtual cmm::math::Sphered      getBoundingSphere(void) const                                       = 0;

}; 

PARAM_EXPORT IParameter *createParameter(const ID &id);

}

#endif

