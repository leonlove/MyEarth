#ifndef PARAMETER_H_72766D47_1F2C_4B35_B1A4_5BFF0ED0FF49_INCLUDE
#define PARAMETER_H_72766D47_1F2C_4B35_B1A4_5BFF0ED0FF49_INCLUDE

#include "Typedefs.h"
#include <ParameterSys\IParameter.h>
#include <DEUDBProxy/IDEUDBProxy.h>

class Parameter:public OpenSP::Ref
{
public:
    Parameter(void){}
    ~Parameter(void){}

    virtual const ID&           getID()const = 0;
    virtual cmm::math::Sphered  getBoundingSphere(void) = 0;
    virtual bool                writeToDB(const ID &idParent, deudbProxy::IDEUDBProxy *db) = 0;
    virtual param::IParameter*  base() = 0;

protected:
    cmm::math::Sphered  _bs;
};

typedef std::list<OpenSP::sp<Parameter>> ParameterList;

#endif
