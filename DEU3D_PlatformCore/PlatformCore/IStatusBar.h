#ifndef I_STATUS_BAR_H_A11CC59F_8BF8_450B_B582_68DC2B36A53B_INCLUDE
#define I_STATUS_BAR_H_A11CC59F_8BF8_450B_B582_68DC2B36A53B_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include "common/Common.h"

class IDemHelper : virtual public OpenSP::Ref
{
public:
    virtual double getDem(double dblLongitude, double dblLatitude) const = 0;
};


class IStatusBar : virtual public OpenSP::Ref
{
public:
    virtual void    setVisible(bool bShow = true)                           = 0;
    virtual bool    isVisible(void) const                                   = 0;
    virtual void    setUserString(const std::string &strUserString)         = 0;
    virtual void    setTextColor(const cmm::FloatColor &color)              = 0;
    virtual const   cmm::FloatColor &getTextColor(void) const               = 0;

    virtual void    setTextSize(double dblSize)                             = 0;
    virtual double  getTextSize(void) const                                 = 0;

    virtual void    setTextFont(const std::string &strTextFont)             = 0;
    virtual const   std::string &getTextFont(void) const                    = 0;

    virtual void    setPaneColor(const cmm::FloatColor &color)              = 0;
    virtual const   cmm::FloatColor& getPaneColor() const                   = 0;

    virtual void    setBarWidth(unsigned nWidth)                            = 0;
    virtual void    setDemHelper(IDemHelper *pDemHelper)                    = 0;
};

#endif