#include <algorithm>
#include <OpenSP/sp.h>

#include "EventFilter.h"

namespace ea
{

IEventFilter *createEventFilter(void)
{
    OpenSP::sp<IEventFilter> pEventFilter = new EventFilter;
    return pEventFilter.release();
}

EventFilter::EventFilter(void)
{
    m_nPriority = 0;
}

EventFilter::EventFilter(const std::string &strAction)
{
    m_nPriority = 0;
    addAction(strAction);
}

EventFilter::EventFilter(const EventFilter &filter)
{
    m_nPriority = filter.m_nPriority;
    m_vecActions.assign(filter.m_vecActions.begin(), filter.m_vecActions.end());
}

EventFilter::~EventFilter(void)
{
    m_vecActions.clear();
}

void EventFilter::setPriority(const unsigned int nPriority)
{
    m_nPriority = nPriority;
}

unsigned int EventFilter::getPriority(void) const
{
    return m_nPriority;
}

void EventFilter::addAction(const std::string &strAction)
{
    if(strAction.empty())   return;
    if(!hasAction(strAction))
    {
        m_vecActions.push_back(strAction);
    }
}

unsigned EventFilter::countActions(void) const
{
    return m_vecActions.size();
}

std::string EventFilter::getAction(unsigned nIndex) const
{
    if(nIndex >= m_vecActions.size())
    {
        std::string strEmpty;
        return strEmpty;
    }
    return m_vecActions[nIndex];
}

bool EventFilter::hasAction(const std::string &strAction) const
{
    if(std::find(m_vecActions.begin(), m_vecActions.end(), strAction) == m_vecActions.end())
    {
        return false;
    }

    return true;
}

}