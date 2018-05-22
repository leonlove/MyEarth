#ifndef EVENT_ADAPTER_H_588072A5_3D32_430C_AA98_EDED134D2272_INCLUDE
#define EVENT_ADAPTER_H_588072A5_3D32_430C_AA98_EDED134D2272_INCLUDE

#include <vector>
#include "IEventAdapter.h"
#include <OpenThreads/Mutex>

namespace ea
{
class EventAdapter : public IEventAdapter
{
public:
    EventAdapter(void);
    EventAdapter(const EventAdapter &adapter);
    virtual ~EventAdapter(void);

public:
    virtual void sendBroadcast(IEventObject *pEvent);
    virtual bool registerReceiver(IEventReceiver *pEventReceiver, IEventFilter *pEventFilter);
    virtual void unregisterReceiver(IEventReceiver *pEventReceiver);

protected:
    typedef std::pair< OpenSP::sp<IEventReceiver>, OpenSP::sp<IEventFilter> >   ReceiverPair;
    typedef std::vector<ReceiverPair>       ReceiverList;

    struct SortEventFunctor
    {
        bool operator()(const ReceiverPair &lhs, const ReceiverPair &rhs) const;
    };

    struct FindEventFunctor
    {
        const IEventReceiver *m_pEventReceiver;

        FindEventFunctor(const IEventReceiver *pEventReceiver): m_pEventReceiver(pEventReceiver) {}

        bool operator()(const ReceiverPair &ep)
        {
            return ep.first == m_pEventReceiver;
        }
    };

protected:
    ReceiverList        m_vecReceivers;
    OpenThreads::Mutex  m_mtxReceivers;
};

}

#endif
