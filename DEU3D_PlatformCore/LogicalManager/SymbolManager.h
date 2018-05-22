#ifndef SYMBOL_LIB_H_2FA6EF2C_F9A9_4762_BF07_1EBEC0B8F6D8_INCLUDE
#define SYMBOL_LIB_H_2FA6EF2C_F9A9_4762_BF07_1EBEC0B8F6D8_INCLUDE

#include "ISymbolManager.h"
#include "ISymbolCategory.h"
#include "common/DEUBson.h"
#include "PropertyManager.h"

#include <OpenSP/sp.h>

namespace logical
{
    class SymbolManager : public ISymbolManager
    {
    public:
        explicit SymbolManager(void);
        virtual ~SymbolManager(void);

    public:
        virtual bool                    initialize(const std::string &strHost, const std::string &strPort);
        virtual bool                    importLocalCategory(const std::string &strCategoryFile);
        virtual ISymbolCategory*        getRootCategory(void);
        virtual const ISymbolCategory*  getRootCategory(void) const;

        virtual void                    gatherDBs(std::vector<std::string>& vecDBs) const;

        IPropertyManager* getPropertyManager(void) {   return m_pPropertyManager.get();   }

    private:
        OpenSP::sp<deunw::IDEUNetwork>  m_pNetWork;
        OpenSP::sp<PropertyManager>     m_pPropertyManager;
        OpenSP::sp<ISymbolCategory>     m_pRootCategory;
        OpenSP::sp<ISymbolCategory>     m_pRootLocalCategory;
    };
}

#endif
