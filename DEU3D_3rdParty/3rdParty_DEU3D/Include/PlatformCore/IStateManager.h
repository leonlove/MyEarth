#ifndef ISTATEMANAGER_H__
#define ISTATEMANAGER_H__ 1

#include <OpenSP/Ref.h>
#include <string>
#include <vector>

class IState;

class IStateManager : virtual public OpenSP::Ref
{
public:
    virtual IState               *createRenderState(const std::string &strTypeName, const std::string &strStateName)  = 0;
    virtual IState               *getRenderState(const std::string &strStateName)                               = 0;
    virtual std::vector<IState *> getRenderStateList(const std::string &strTypeName)                            = 0;
};

#endif