#ifndef I_EVENT_FILTER_H_58119BE4_C755_4E20_89A9_6658CA78FD04_INCLUDE
#define I_EVENT_FILTER_H_58119BE4_C755_4E20_89A9_6658CA78FD04_INCLUDE

#include <OpenSP/Ref.h>
#include <string>

namespace ea
{
//事件过滤器
class IEventFilter : public OpenSP::Ref
{
public:
    //设置优先级
    virtual void        setPriority(unsigned nPriority)               = 0;
    //获取优先级
    virtual unsigned    getPriority(void) const                       = 0;
    //添加一个行为
    virtual void        addAction(const std::string &strAction)       = 0;
    virtual unsigned    countActions(void) const                      = 0;
    virtual std::string getAction(unsigned nIndex) const              = 0;
    virtual bool        hasAction(const std::string &strAction) const = 0;
};

EA_DLL_SPECIAL IEventFilter *createEventFilter(void);

}

#endif

