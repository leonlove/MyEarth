#ifndef EVENT_OBJECT_H_1EBCDCB8_CA63_4C10_A18A_50531D8AB5C1_INCLUDE
#define EVENT_OBJECT_H_1EBCDCB8_CA63_4C10_A18A_50531D8AB5C1_INCLUDE

#include <OpenSP/sp.h>

#include "IEventObject.h"

#include <map>
//#include <BSONLib/bson.h>

namespace ea
{

class EventObject : public IEventObject
{
public:
    explicit EventObject(void) {};
    virtual ~EventObject(void) {};

public:
    virtual void                setAction(const std::string &strAction);
    virtual const std::string  &getAction(void);
    virtual void                putExtra(const std::string &strName, const cmm::variant_data &data);
    virtual bool                getExtra(const std::string &strName, cmm::variant_data &data);

protected:
    std::string          m_strAction;
    std::map<std::string, cmm::variant_data>      m_mapData;
};
}

#endif
