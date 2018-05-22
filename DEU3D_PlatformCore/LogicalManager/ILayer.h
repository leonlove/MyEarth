#ifndef I_LAYER_H_4E99214C_945B_460E_A271_1F00558D25CD_INCLUDE
#define I_LAYER_H_4E99214C_945B_460E_A271_1F00558D25CD_INCLUDE


#include "IObject.h"
#include "Common/DEUBson.h"

namespace logical
{

class ILayer : virtual public IObject
{
public:
    virtual ILayer         *createSubLayer(void)                                            = 0;
    virtual bool            addChild(const ID &id)                                          = 0;
    virtual bool            removeChild(const ID &id)                                       = 0;
    virtual unsigned        getChildrenCount(void)                                  const   = 0;

    virtual const IObject  *getChild(unsigned nIndex)                               const   = 0;
    virtual IObject        *getChild(unsigned nIndex)                                       = 0;
    virtual const IObject  *getChild(const ID &id)                                  const   = 0;
    virtual IObject        *getChild(const ID &id)                                          = 0;

    virtual ID              getChildID(unsigned nIndex)                                     = 0;
    virtual unsigned        getChildrenLayerCount(void) const                               = 0;
    virtual ILayer         *getChildLayer(unsigned nIndex)                                  = 0;

    virtual bool            hasChild(const ID &id)                                  const   = 0;
    virtual bool            isDirty(void)                                           const   = 0;
};

}

#endif