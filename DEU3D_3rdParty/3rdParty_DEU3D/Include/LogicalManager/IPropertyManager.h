#ifndef I_PROPERTY_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE
#define I_PROPERTY_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE

#include "Export.h"

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/Common.h>
#include <Common/variant.h>
#include <string>
#include <vector>
#include "IProperty.h"
#include <Network/IDEUNetwork.h>

namespace logical
{

class IPropertyManager : public OpenSP::Ref
{
public:
    virtual bool        addLocalPropertyServer(const std::string &strServer)                    = 0;
    virtual bool        removeLocalPropertyServer(const std::string &strServer)                 = 0;
    virtual IProperty  *findProperty(const ID &id, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
    virtual IProperty  *createPropertyByBsonStream(const void *pBsonStream, unsigned int nLen)  = 0;
    virtual IProperty  *createPropertyByJsonStream(const std::string &strJsonCode)              = 0;
    virtual bool        saveProperties2Local(const std::string &strSaveName, IDList &idList)    = 0;
};

LOGICAL_EXPORT IPropertyManager *createPropertyManager(void);

}

#endif