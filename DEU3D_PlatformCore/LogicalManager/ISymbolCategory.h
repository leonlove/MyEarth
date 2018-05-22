#ifndef I_SYMBOL_CATEGORY_H_2C00C16D_9128_4ABF_BAE4_E96C11FB24E2_INCLUDE
#define I_SYMBOL_CATEGORY_H_2C00C16D_9128_4ABF_BAE4_E96C11FB24E2_INCLUDE

#include "export.h"
#include <string>
#include "OpenSP/Ref.h"
#include "IDProvider/ID.h"

namespace logical
{
    class ISymbolCategory : public OpenSP::Ref
    {
    public:
        virtual const ID&               getID(void) const                                   = 0;
        virtual const std::string&      getName(void) const                                 = 0;
        virtual void                    setName(const std::string &strName)                 = 0;
        virtual const std::string&      getDescription(void) const                          = 0;
        virtual void                    setDescription(const std::string& strDesc)          = 0;
        virtual unsigned                getSymbolsCount(void) const                         = 0;
        virtual const ID&               getSymbol(unsigned nIndex) const                    = 0;
        virtual bool                    addSymbol(const ID &id)                             = 0;
        virtual bool                    removeSymbol(const ID &id)                          = 0;
        virtual bool                    addSymbolDB(const std::string &strDB)               = 0;
        virtual unsigned                getSubCategoriesCount(void) const                   = 0;
        virtual ISymbolCategory*        getSubCategory(unsigned nIndex)                     = 0;
        virtual const ISymbolCategory*  getSubCategory(unsigned nIndex) const               = 0;
        virtual bool                    addSubCategory(ISymbolCategory *pCategory)          = 0;
        virtual ISymbolCategory*        createSubCategory(const std::string &strName)       = 0;
        virtual bool                    removeSubCategory(const ISymbolCategory *pCategory) = 0;
        virtual unsigned                getParentCount(void) const                          = 0;
        virtual ISymbolCategory*        getParent(unsigned nIndex)                          = 0;
        virtual const ISymbolCategory*  getParent(unsigned nIndex) const                    = 0;
    };
}

#endif


