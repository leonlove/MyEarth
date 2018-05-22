#ifndef _I_OGCFILTER_H_311EF6A4_D13C_42B6_A2DF_2C389FEA12CA_
#define _I_OGCFILTER_H_311EF6A4_D13C_42B6_A2DF_2C389FEA12CA_

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <string>

enum FilterType
{
    Logical_And                = 0,
    Logical_Or                 = 1,
    Logical_EndAnd             = 2,
    Logical_EndOr              = 3,
    Logical_Not                = 4,
    Logical_EndNot             = 5,
    Compare_EqualTo            = 6,
    Compare_NotEqualTo         = 7,
    Compare_LessThan           = 8,
    Compare_GreaterThan        = 9,
    Compare_LessThanEqualTo    = 10,
    Compare_GreaterThanEqualTo = 11,
    Compare_Like               = 12,
    Compare_Between            = 13,
    BBOX                       = 14,
    ID                         = 15

};

class IOGCFilter : public OpenSP::Ref
{
public:
    virtual void setFilterType(FilterType fType) = 0;
    virtual FilterType getFilterType() = 0;
    virtual IOGCFilter* getFilter(unsigned nIndex) = 0;
    virtual void addFilter(IOGCFilter* pFilter) = 0;
    virtual void setFilter(unsigned nIndex,IOGCFilter* pFilter) = 0;
    virtual void removeFilter(unsigned nIndex) = 0;
    virtual std::string toString() = 0;
};

IOGCFilter* createFilter(FilterType fType);

#endif //_I_OGCFILTER_H_311EF6A4_D13C_42B6_A2DF_2C389FEA12CA_