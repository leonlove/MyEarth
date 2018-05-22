#ifndef EVENT_FILTER_H_66AFB8C9_9844_4468_B560_B4D2B2604F99_INCLUDE
#define EVENT_FILTER_H_66AFB8C9_9844_4468_B560_B4D2B2604F99_INCLUDE

#include <vector>

#include "Export.h"
#include "IEventFilter.h"

namespace ea
{

class EventFilter : public IEventFilter
{
public:
    EventFilter(void);
    EventFilter(const std::string &strAction);
    EventFilter(const EventFilter &filter);
    virtual ~EventFilter(void);

protected:
    virtual void            setPriority(unsigned nPriority);
    virtual unsigned int    getPriority(void) const;
    virtual void            addAction(const std::string &strAction);
    virtual unsigned int    countActions(void) const;
    virtual std::string     getAction(unsigned nIndex) const;
    virtual bool            hasAction(const std::string &strAction) const;

protected:
    unsigned int             m_nPriority;
    std::vector<std::string> m_vecActions;
};

}

#endif
