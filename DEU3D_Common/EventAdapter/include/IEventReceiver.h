#ifndef I_EVENT_RECEIVER_H_0C6C5314_4265_4261_A8B7_B5E20C7DA51F_INCLUDE
#define I_EVENT_RECEIVER_H_0C6C5314_4265_4261_A8B7_B5E20C7DA51F_INCLUDE

#include <OpenSP/Ref.h>

namespace ea
{
class IEventAdapter;
class IEventObject;

//事件接收器
class IEventReceiver : public OpenSP::Ref
{
public:
    //接受一个事件
    virtual void onReceive(IEventAdapter *pEventAdapter, IEventObject *pEventObject) = 0;
};

}
#endif
