#ifndef I_PROPERTY_H_95045BE9_B119_44C9_B9DF_F0CADA18D1DA_INCLUDE
#define I_PROPERTY_H_95045BE9_B119_44C9_B9DF_F0CADA18D1DA_INCLUDE

#include <string>
#include <OpenSP/Ref.h>
#include <Common/variant.h>

namespace logical
{

class IProperty : virtual public OpenSP::Ref
{
public:
    // methods for normal Property
    virtual const std::string &getTitle(void)                       const = 0;  // get property name
    virtual const cmm::variant_data &getValue(void)                 const = 0;  // get property value
    virtual std::string getValueAsString(void)                      const = 0;

    // methods if IProperty present an array or a structure
    virtual bool     isSimpleProperty(void)                         const = 0;  // check IProperty is an array or a structure
    virtual unsigned getChildrenCount(void)                         const = 0;  // get children count

    // get children titles.
    virtual cmm::variant_data getChildrenTitles(void)               const = 0;

    // get child property by index.
    // attention: if IProperty is an structure, 'nIndex' maynot present explicitly
    virtual IProperty *getChild(unsigned nIndex)                          = 0;
    virtual const IProperty *getChild(unsigned nIndex)              const = 0;

    // get child property by title.
    // if IProperty is an array, it returns nothing
    virtual IProperty *getChild(const std::string &strTitle)              = 0;
    virtual const IProperty *getChild(const std::string &strTitle)  const = 0;

    virtual IProperty *findProperty(const std::string &strTitle, bool bCheckSelf = false)             = 0;
    virtual const IProperty *findProperty(const std::string &strTitle, bool bCheckSelf = false) const = 0;
};

}

#endif

