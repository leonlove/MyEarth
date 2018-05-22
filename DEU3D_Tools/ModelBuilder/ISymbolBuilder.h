#pragma once

#include "Export.h"
#include <OpenSP\Ref.h>
#include <string>
#include "IModelBuilder.h"

class ISymbolBuilder:public Ref
{
public:
    virtual void initialize(const string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *ea, sp<IDEUException> e = NULL)   = 0;
    virtual void buildIveSymbol(const std::string &strFile, sp<IDEUException> e = NULL)                                            = 0;
//    virtual void buildParamSymbol(ISymbol *pSymbol, sp<IDEUException> e = NULL)                                                    = 0;
    virtual void writeConfigFile(sp<IDEUException> e = NULL)                                                                       = 0;
    virtual ID   insertImage(const std::string &strFile, sp<IDEUException> e = NULL)                                               = 0;
};

MB_EXPORTS ISymbolBuilder* createSymbolBuilder(void);