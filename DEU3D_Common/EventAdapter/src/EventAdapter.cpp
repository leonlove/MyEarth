#include <algorithm>
#include <OpenSP/sp.h>

#include "EventAdapter.h"

#include "IEventReceiver.h"
#include "IEventFilter.h"
#include "IEventObject.h"

namespace ea
{

EA_DLL_SPECIAL const std::string ACTION_POINT_TOOL          = "action_point_tool";
EA_DLL_SPECIAL const std::string ACTION_LINE_TOOL           = "action_line_tool";
EA_DLL_SPECIAL const std::string ACTION_POLYLINE_TOOL       = "action_polyline_tool";
EA_DLL_SPECIAL const std::string ACTION_RECT_TOOL           = "action_rect_tool";
EA_DLL_SPECIAL const std::string ACTION_ELLIPSE_TOOL        = "action_ellipse_tool";
EA_DLL_SPECIAL const std::string ACTION_POLYGON_TOOL        = "action_polygon_tool";
EA_DLL_SPECIAL const std::string ACTION_EDITING_TOOL        = "action_editing_tool";
EA_DLL_SPECIAL const std::string ACTION_NAVPATH_KEYFRAME    = "action_navpath_keyframe";
EA_DLL_SPECIAL const std::string ACTION_CAMERA_POSE_CHANGED = "action_camera_pose_changed";
EA_DLL_SPECIAL const std::string ACTION_SUBMIT_DATA			= "action_submit_data";
EA_DLL_SPECIAL const std::string ACTION_START_SERVICE		= "action_start_service";
EA_DLL_SPECIAL const std::string ACTION_STOP_SERVICE		= "action_stop_service";
EA_DLL_SPECIAL const std::string ACTION_SUBMIT_BREAK		= "action_submit_break";
EA_DLL_SPECIAL const std::string ACTION_TILE_BUILDER 		= "action_tile_builder";
EA_DLL_SPECIAL const std::string ACTION_MODEL_BUILDER 		= "action_model_builder";
//EA_DLL_SPECIAL const std::string ACTION_STARTALL_SERVICE	= "action_startall_service";

/*¡Ÿ ±œ˚œ¢***************************************************************/
EA_DLL_SPECIAL const std::string ACTION_MEASURE_TOOL        = "action_measure_tool";
EA_DLL_SPECIAL const std::string ACTION_AREA_TOOL           = "action_area_tool";
EA_DLL_SPECIAL const std::string ACTION_DISTANCE_TOOL       = "action_distance_tool";
/************************************************************************/

IEventAdapter *createEventAdapter(void)
{
    static OpenSP::sp<IEventAdapter> pEventAdapter;
    if(!pEventAdapter.valid())
    {
        pEventAdapter = new EventAdapter;
    }
    return pEventAdapter.get();
}


EventAdapter::EventAdapter(void)
{

}


EventAdapter::EventAdapter(const EventAdapter &adapter)
            : m_vecReceivers(adapter.m_vecReceivers)
{
}


EventAdapter::~EventAdapter(void)
{

}


bool EventAdapter::SortEventFunctor::operator()(const ReceiverPair &lhs,const ReceiverPair &rhs) const
{
    if (lhs.second->getPriority() > rhs.second->getPriority())
    {
        return true;
    }
    return false;
}


void EventAdapter::sendBroadcast(IEventObject *pEvent)
{
    /*ReceiverList  vecReceivers;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxReceivers);
        vecReceivers.assign(m_vecReceivers.begin(), m_vecReceivers.end());
    }

    ReceiverList::iterator itor = vecReceivers.begin();
    for(; itor != vecReceivers.end(); ++itor)
    {
        ReceiverPair &rp = *itor;
        IEventFilter *pFilter = rp.second.get();
        if(pFilter->hasAction(pEvent->getAction()))
        {
            IEventReceiver *pReceiver = rp.first.get();
            pReceiver->onReceive(this, pEvent);
        }
    }*/

    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxReceivers);

    ReceiverList::iterator itor = m_vecReceivers.begin();
    for(; itor != m_vecReceivers.end(); ++itor)
    {
        ReceiverPair &rp = *itor;
        IEventFilter *pFilter = rp.second.get();
        if(pFilter->hasAction(pEvent->getAction()))
        {
            IEventReceiver *pReceiver = rp.first.get();
            pReceiver->onReceive(this, pEvent);
        }
    }
    return;
}


bool EventAdapter::registerReceiver(IEventReceiver *pEventReceiver, IEventFilter *pEventFilter)
{
    if(pEventReceiver == NULL || pEventFilter == NULL)
    {
        return false;
    }

    ReceiverPair ep;
    ep.first  = pEventReceiver;
    ep.second = pEventFilter;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxReceivers);
        m_vecReceivers.push_back(ep);
        std::sort(m_vecReceivers.begin(), m_vecReceivers.end(), SortEventFunctor());
    }
    return true;
}


void EventAdapter::unregisterReceiver(IEventReceiver *pEventReceiver)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxReceivers);
    ReceiverList::iterator itor = std::find_if(m_vecReceivers.begin(), m_vecReceivers.end(), FindEventFunctor(pEventReceiver));
    if(m_vecReceivers.end() != itor)
    {
        m_vecReceivers.erase(itor);
    }
    return;
}

}
