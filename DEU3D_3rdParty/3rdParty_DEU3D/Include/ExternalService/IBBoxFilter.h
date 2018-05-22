#ifndef _I_BBOXFILTER_H_A60BE577_EEFB_4D8E_8EEC_08095E151228_
#define _I_BBOXFILTER_H_A60BE577_EEFB_4D8E_8EEC_08095E151228_

#include "IOGCFilter.h"

class IBBoxFilter : virtual public IOGCFilter
{
public:
    virtual void setBBox(double dMinX,double dMinY,double dMaxX,double dMaxY) = 0;
    virtual void getBBox(double& dMinX,double& dMinY,double& dMaxX,double& dMaxY) = 0;
    virtual void setPropertyName(const std::string& strProperty) = 0;
    virtual std::string getPropertyName() const = 0;
};

#endif //_I_BBOXFILTER_H_A60BE577_EEFB_4D8E_8EEC_08095E151228_