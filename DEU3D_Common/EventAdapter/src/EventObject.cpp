#include "EventObject.h"
#include <Common\variant.h>

namespace ea
{

IEventObject *createEventObject()
{
    OpenSP::sp<IEventObject> pEventObject = new EventObject;
    return pEventObject.release();
}


void EventObject::setAction(const std::string &strAction)
{
    m_strAction = strAction;
}


const std::string &EventObject::getAction(void)
{
    return m_strAction;
}


void EventObject::putExtra(const std::string &strName, const cmm::variant_data &data)
{
    m_mapData[strName] = data;
}


bool EventObject::getExtra(const std::string &strName, cmm::variant_data &data)
{
    std::map<std::string, cmm::variant_data>::const_iterator itorFind = m_mapData.find(strName);
    if(itorFind == m_mapData.end())
    {
        return false;
    }
    data = itorFind->second;
    return true;
}

}
