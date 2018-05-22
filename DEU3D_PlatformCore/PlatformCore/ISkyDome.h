#ifndef I_SKY_DOME_H_0015E87E_35C9_4F69_B55B_7AEFD7BB1065_INCLUDE
#define I_SKY_DOME_H_0015E87E_35C9_4F69_B55B_7AEFD7BB1065_INCLUDE

#include <OpenSP/Ref.h>
#include <Common/Common.h>

class ISkyDome : virtual public OpenSP::Ref
{
public:
    virtual void getDateTime(cmm::DateTime &dateTime) const = 0;
    virtual void setDateTime(const cmm::DateTime &dateTime) = 0;

    virtual void   setAmbientBrightness(double dblValue)    = 0;
    virtual double getAmbientBrightness(void) const         = 0;

    virtual void   setDiffuseBrightness(double dblValue)    = 0;
    virtual double getDiffuseBrightness(void) const         = 0;

    virtual void setMoonVisible(bool bVisible)              = 0;
    virtual bool getMoonVisible(void) const                 = 0;

    virtual void setStarsVisible(bool bVisible)             = 0;
    virtual bool getStarsVisible(void) const                = 0;

    virtual void setSunVisible(bool bVisible)               = 0;
    virtual bool getSunVisible(void) const                  = 0;

    virtual void setAtmosphereVisible(bool bVisible)        = 0;
    virtual bool getAtmosphereVisible(void) const           = 0;

    virtual bool                     setLightMode(const std::string &strMode)  = 0;
    virtual const std::string       &getLightMode(void) const                        = 0;
};


#endif

