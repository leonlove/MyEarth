#ifndef _I_COMPAREFILTER_H_B32A7699_D2D0_4795_9128_CD7B1F3FF72F_
#define _I_COMPAREFILTER_H_B32A7699_D2D0_4795_9128_CD7B1F3FF72F_

#include "IOGCFilter.h"

class ICompareFilter : virtual public IOGCFilter
{
public:
    virtual void setPropertyName(const std::string& strProperty) = 0;
    virtual std::string getPropertyName() const = 0;
    virtual void setLiteral(const std::string& strLiteral) = 0;
    virtual std::string getLiteral() const = 0;
    virtual void setLiteral2(const std::string& strLiteral) = 0;
    virtual std::string getLiteral2() const = 0;
};

#endif //_I_COMPAREFILTER_H_B32A7699_D2D0_4795_9128_CD7B1F3FF72F_