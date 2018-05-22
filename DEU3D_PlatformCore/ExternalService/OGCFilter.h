#ifndef _OGCFILTER_INCLUDE_H_39F35C9C_07DA_420F_8A64_FEFB56D52180_
#define _OGCFILTER_INCLUDE_H_39F35C9C_07DA_420F_8A64_FEFB56D52180_

#include "IOGCFilter.h"
#include <vector>

class OGCFilter : virtual public IOGCFilter
{
public:
    explicit OGCFilter(void);
    virtual ~OGCFilter(void);

    virtual void setFilterType(FilterType fType) { m_enumFilterType = fType; }
    virtual FilterType getFilterType() { return m_enumFilterType; }
    virtual IOGCFilter* getFilter(unsigned nIndex);
    virtual void addFilter(IOGCFilter* pFilter);
    virtual void setFilter(unsigned nIndex,IOGCFilter* pFilter);
    virtual void removeFilter(unsigned nIndex);
    virtual std::string toString();
protected:
    FilterType               m_enumFilterType;
    std::vector<IOGCFilter*> m_pFilterVec;
};

#endif //_OGCFILTER_INCLUDE_H_39F35C9C_07DA_420F_8A64_FEFB56D52180_

