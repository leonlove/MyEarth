#ifndef I_FLUORO_SCOPE_H_C4DC60C5_06E0_43DE_81F1_53824BAD471A_INCLUDE
#define I_FLUORO_SCOPE_H_C4DC60C5_06E0_43DE_81F1_53824BAD471A_INCLUDE


#include <OpenSP/Ref.h>

class IFluoroScope : virtual public OpenSP::Ref
{
public:
    virtual void        setEnable(bool bEnable)                 = 0;
    virtual bool        isEnable(void) const                    = 0;
    virtual void        setFlatMode(bool bFlat)                 = 0;
    virtual bool        isFlatMode() const                      = 0;
    virtual void        setPolygonSideCount(unsigned nCount)    = 0;
    virtual unsigned    getPolygonSideCount(void) const         = 0;
    virtual void        setFluoroScopeSize(double dblSize)      = 0;
    virtual double      getFluoroScopeSize(void) const          = 0;
};


#endif
