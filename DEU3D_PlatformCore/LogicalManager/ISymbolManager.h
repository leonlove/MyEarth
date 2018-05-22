#ifndef I_SYMBOL_MANAGER_H_FD99F63D_9452_4467_A144_1AF9F5FDDBA7_INCLUDE
#define I_SYMBOL_MANAGER_H_FD99F63D_9452_4467_A144_1AF9F5FDDBA7_INCLUDE

#include <string>
#include "OpenSP/Ref.h"
#include "IDProvider/ID.h"
#include "Network/IDEUNetwork.h"
#include "ISymbolCategory.h"
#include "Export.h"

namespace logical
{
    class ISymbolManager : public OpenSP::Ref
    {
    public:
        virtual bool                    initialize(const std::string &strHost, const std::string &strPort) = 0;
        virtual bool                    importLocalCategory(const std::string &strCategoryFile) = 0;
        virtual       ISymbolCategory*  getRootCategory(void)                                   = 0;
        virtual const ISymbolCategory*  getRootCategory(void) const                             = 0;

        virtual void                    gatherDBs(std::vector<std::string>& vecDBs) const       = 0;
    };

    LOGICAL_EXPORT ISymbolManager *createSymbolManager(void);
}

#endif
