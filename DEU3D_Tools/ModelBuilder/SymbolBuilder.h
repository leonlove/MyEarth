#ifndef SYMBOL_BUILDER_H_4ED71E1E_DB9C_49D4_8779_CEB34B4FCC53_INCLUDE
#define SYMBOL_BUILDER_H_4ED71E1E_DB9C_49D4_8779_CEB34B4FCC53_INCLUDE

#include "ISymbolBuilder.h"
#include <DEUDB/IDEUDB.h>
#include <OpenSP\sp.h>
#include "Builder.h"

class SymbolBuilder:public ISymbolBuilder, public Builder
{
public:
    SymbolBuilder(void);

    void writeConfigFile(sp<IDEUException> e = NULL);

    void initialize(const std::string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *ea, sp<IDEUException> e = NULL);
    void buildIveSymbol(const std::string &strFile, sp<IDEUException> e = NULL);
    void buildParamSymbol(ISymbol *pSymbol, sp<IDEUException> e = NULL);
    ID   insertImage(const std::string &strFile, sp<IDEUException> e = NULL);

protected:
    bool writeCategory(bsonArrayEle *pSymbolIDs);
    unsigned short  _dataset_code;
    sp<deudbProxy::IDEUDBProxy>      _db;
	std::string strTargetPath;
};

#endif
