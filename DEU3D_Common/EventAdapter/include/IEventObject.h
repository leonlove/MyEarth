#ifndef I_EVENT_OBJECT_H_1091D5EB_E376_47A2_B799_DAA888033380_INCLUDE
#define I_EVENT_OBJECT_H_1091D5EB_E376_47A2_B799_DAA888033380_INCLUDE


#include <OpenSP/Ref.h>
#include <string>
#include <vector>

#include "Export.h"

#include <Common\variant.h>

namespace ea
{

class IEventObject : public OpenSP::Ref
{
public:
    virtual void                setAction(const std::string &strAction) = 0;
    virtual const std::string  &getAction(void) = 0;
    virtual void                putExtra(const std::string &strName, const cmm::variant_data &data) = 0;
    virtual bool                getExtra(const std::string &strName, cmm::variant_data &data) = 0;
};

EA_DLL_SPECIAL IEventObject *createEventObject(void);

}
#endif
