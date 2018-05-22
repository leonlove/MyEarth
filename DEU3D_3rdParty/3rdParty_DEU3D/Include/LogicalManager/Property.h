#ifndef PROPERTY_H_A3736214_6650_4D03_BCB7_C5AE089DCA3D_INCLUDE
#define PROPERTY_H_A3736214_6650_4D03_BCB7_C5AE089DCA3D_INCLUDE

#include "IProperty.h"
#include <vector>
#include <string>
#include <common/memPool.h>

namespace logical
{

class Property : public IProperty
{
public:
    explicit Property(void);
    virtual ~Property(void);

public:
    void *operator new(size_t t)
    {
        return ms_MemPool.alloc();
    }
    void operator delete(void *p)
    {
        ms_MemPool.free(p);
    }

protected:  // methods from IProperty
    // methods for normal Property
    virtual const std::string &getTitle(void)  const;
    virtual const cmm::variant_data &getValue(void) const;
    virtual std::string getValueAsString(void) const;

    // methods if IProperty present an array or a structure
    virtual bool     isSimpleProperty(void) const;
    virtual unsigned getChildrenCount(void) const;

    // get children titles.
    virtual cmm::variant_data getChildrenTitles(void) const;

    // get child property by index.
    // attention: if IProperty is an structure, 'nIndex' maynot present explicitly
    virtual IProperty *getChild(unsigned nIndex);
    virtual const IProperty *getChild(unsigned nIndex) const;

    // get child property by title.
    // if IProperty is an array, it returns nothing
    virtual IProperty *getChild(const std::string &strTitle);
    virtual const IProperty *getChild(const std::string &strTitle) const;

    virtual IProperty *findProperty(const std::string &strTitle, bool bCheckSelf = false);
    virtual const IProperty *findProperty(const std::string &strTitle, bool bCheckSelf = false) const;

protected:
    friend class        PropertyManager;
    std::string         m_strTitle;
    cmm::variant_data   m_varValue;
    bool                m_bSimpleProperty;
    std::vector<OpenSP::sp<Property> >      m_vecChildrenProperty;
    static cmm::MemPool         ms_MemPool;
};

}
#endif
