#ifndef RECIVER_MANAGER_H_DA5FAA09_101C_478D_B7D8_E5BFDA6EBC7A_INCLUDE
#define RECIVER_MANAGER_H_DA5FAA09_101C_478D_B7D8_E5BFDA6EBC7A_INCLUDE

#include <EventAdapter/IEventReceiver.h>
#include <OpenSP/Ref.h>
#include <osg/Node>

#include "HidenController.h"
#include "HighlightController.h"

namespace ea
{
    class IEventAdapter;
    class IEventObject;
}

class ReciverManager : public ea::IEventReceiver
{
public:
    explicit ReciverManager(void);
    virtual ~ReciverManager(void);
public:
    virtual void onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject);

public:
    void setRootNode(osg::Node *pNode);

protected:
    osg::Node                       *m_pRootNode;
    OpenSP::sp<HidenController>     m_pHidenController;
    OpenSP::sp<HighlightController> m_pHighlightController;
};

#endif
