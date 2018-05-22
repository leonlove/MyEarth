#ifndef I_EVENT_FILTER_H_58119BE4_C755_4E20_89A9_6658CA78FD04_INCLUDE
#define I_EVENT_FILTER_H_58119BE4_C755_4E20_89A9_6658CA78FD04_INCLUDE

#include <OpenSP/Ref.h>
#include <string>

namespace ea
{
//�¼�������
class IEventFilter : public OpenSP::Ref
{
public:
    //�������ȼ�
    virtual void        setPriority(unsigned nPriority)               = 0;
    //��ȡ���ȼ�
    virtual unsigned    getPriority(void) const                       = 0;
    //���һ����Ϊ
    virtual void        addAction(const std::string &strAction)       = 0;
    virtual unsigned    countActions(void) const                      = 0;
    virtual std::string getAction(unsigned nIndex) const              = 0;
    virtual bool        hasAction(const std::string &strAction) const = 0;
};

EA_DLL_SPECIAL IEventFilter *createEventFilter(void);

}

#endif

