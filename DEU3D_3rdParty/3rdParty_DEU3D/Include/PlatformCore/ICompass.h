#ifndef I_COMPASS_H__
#define I_COMPASS_H__
#include <string>
#include <OpenSP/Ref.h>

class INavigatorManager;
class ICompass: virtual public OpenSP::Ref
{
public:
    virtual void  setLayoutType(const std::string &strLayout) = 0;
    virtual const std::string &getLayoutType(void) const = 0;
    virtual void  setVisible(bool bVisible) = 0;
    virtual bool  isVisible(void) const = 0;

    virtual void setNorthernRingVisible(bool bVisible) = 0;
    virtual bool getNorthernRingVisible(void) const = 0;

    virtual void setRotationDiskVisible(bool bVisible) = 0;
    virtual bool getRotationDiskVisible(void) const = 0;

    virtual void setTranslationDiskVisible(bool bVisible) = 0;
    virtual bool getTranslationDiskVisible(void) const = 0;

    virtual void setElevationBarVisible(bool bVisible) = 0;
    virtual bool getElevationBarVisible(void) const = 0;

};
#endif