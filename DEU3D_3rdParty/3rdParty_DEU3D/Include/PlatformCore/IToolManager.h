#ifndef I_TOOL_MANAGER_H_BE0B7DD6_EFEA_4B9E_819F_2C60D9B11767_INCLUDE
#define I_TOOL_MANAGER_H_BE0B7DD6_EFEA_4B9E_819F_2C60D9B11767_INCLUDE

#include <OpenSP/Ref.h>
#include "Export.h"
#include "IToolBase.h"

class IToolManager : virtual public OpenSP::Ref
{
public:
    virtual       IToolBase     *createTool(const std::string &strToolType, const std::string &strName) = 0;
    virtual void                setAutoClear(bool bAutoClear)                                           = 0;
    virtual bool                getAutoClear(void) const                                                = 0;
    virtual const IToolBase     *getTool(const std::string &strName) const                              = 0;
    virtual       IToolBase     *getTool(const std::string &strName)                                    = 0;
    virtual void                setActiveTool(const std::string &strName)                               = 0;
    virtual const std::string   &getActiveTool(void) const                                              = 0;
    virtual void                deactiveTool(void)                                                      = 0;
};


namespace ea
{
    class IEventAdapter;
}

#endif
