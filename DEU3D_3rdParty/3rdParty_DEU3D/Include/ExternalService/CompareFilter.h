#ifndef _COMPAREFILTER_INCLUDE_H_FC871BCB_9A76_4D0D_B66C_305708A754F8_
#define _COMPAREFILTER_INCLUDE_H_FC871BCB_9A76_4D0D_B66C_305708A754F8_

#include "OGCFilter.h"
#include "ICompareFilter.h"

class CompareFilter : public OGCFilter,public ICompareFilter
{
public:
    explicit CompareFilter(void);
    virtual ~CompareFilter(void);

    virtual void setPropertyName(const std::string& strProperty) { m_strPropertyName = strProperty; }
    virtual std::string getPropertyName() const { return m_strPropertyName; }
    virtual void setLiteral(const std::string& strLiteral) { m_strLiteral = strLiteral; }
    virtual std::string getLiteral() const { return m_strLiteral; }
    virtual void setLiteral2(const std::string& strLiteral) { m_strLiteral2 = strLiteral; }
    virtual std::string getLiteral2() const { return m_strLiteral2; }
protected:
    std::string m_strPropertyName;
    std::string m_strLiteral;
    std::string m_strLiteral2;
};

#endif //_COMPAREFILTER_INCLUDE_H_FC871BCB_9A76_4D0D_B66C_305708A754F8_

