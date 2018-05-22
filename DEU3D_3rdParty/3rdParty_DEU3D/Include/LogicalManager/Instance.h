#ifndef LOGICAL_INSTANCE_H_2419B677_51EF_4F00_9660_EB1BE1FFEA11_INCLUDE
#define LOGICAL_INSTANCE_H_2419B677_51EF_4F00_9660_EB1BE1FFEA11_INCLUDE

#include "Object.h"
#include "IInstance.h"

namespace logical
{

class Instance : public Object, virtual public IInstance
{
public:
    explicit Instance(const ID &id);
    virtual ~Instance(void);

protected:
    virtual void            setState(const std::string &strStateName, bool bState);
    virtual bool            getState(const std::string &strStateName) const;
    virtual IInstance      *asInstance(void) {return this;}
    //virtual const cmm::math::Sphered &getBound(void) const       {   return m_BoundingSphere;    };
};

}

#endif