#ifndef I_VIEW_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_VIEW_STATE_H_A9931CB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <OpenSP/Ref.h>
#include <set>

#include "IState.h"

class IViewState : virtual public IState
{
public:

    enum Views
    {
        First_View  = 0x00000001,
        Second_View = 0x00000010,
        Third_View  = 0x00000100,
        Fourth_View = 0x00001000
    };
    virtual void setViews(const unsigned int views) = 0;
    virtual unsigned int getViews()                 = 0;
};

#endif