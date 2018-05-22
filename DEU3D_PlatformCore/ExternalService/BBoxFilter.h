#ifndef _BBOXFILTER_INCLUDE_H_67E51839_2F98_43BC_A6ED_70E45BF023D4_
#define _BBOXFILTER_INCLUDE_H_67E51839_2F98_43BC_A6ED_70E45BF023D4_

#include "IBBoxFilter.h"
#include "OGCFilter.h"

class BBoxFilter : public OGCFilter, public IBBoxFilter
{
public:
    explicit BBoxFilter(void);
    virtual ~BBoxFilter(void);

    virtual void setBBox(double dMinX,double dMinY,double dMaxX,double dMaxY);
    virtual void getBBox(double& dMinX,double& dMinY,double& dMaxX,double& dMaxY);
    virtual void setPropertyName(const std::string& strProperty) { m_strPropertyName = strProperty; }
    virtual std::string getPropertyName() const { return m_strPropertyName; }

protected:
    std::string m_strPropertyName;
    double m_dMinX;
    double m_dMinY;
    double m_dMaxX;
    double m_dMaxY;

};

#endif //_BBOXFILTER_INCLUDE_H_67E51839_2F98_43BC_A6ED_70E45BF023D4_
