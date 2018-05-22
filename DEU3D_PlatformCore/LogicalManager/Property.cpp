#include "Property.h"
#include <sstream>

namespace logical
{

cmm::MemPool Property::ms_MemPool(sizeof(Property), 64u * 1024u, true, "PoolFor[Property]");

Property::Property(void)
{
    m_bSimpleProperty = false;
}


Property::~Property(void)
{
}


const std::string &Property::getTitle(void) const
{
    return m_strTitle;
}


const cmm::variant_data &Property::getValue(void) const
{
    return m_varValue;
}


std::string Property::getValueAsString(void) const
{
    if(!isSimpleProperty())
    {
        static std::string strValue;
        return strValue;
    }

    std::stringstream ss;
    switch(m_varValue.m_eValidate)
    {
        case cmm::variant_data::VT_String:
            return (std::string)m_varValue;
        case cmm::variant_data::VT_Int:
            ss << (int)m_varValue;
            break;
        case cmm::variant_data::VT_UInt:
            ss << (unsigned int)m_varValue;
            break;
        case cmm::variant_data::VT_Bool:
            ss << (int)(bool)m_varValue;
            break;
        case cmm::variant_data::VT_Double:
            ss << (double)m_varValue;
            break;
        case cmm::variant_data::VT_Float:
            ss << (float)m_varValue;
            break;
        case cmm::variant_data::VT_ID:
            return ((ID)m_varValue).toString();
            break;
        case cmm::variant_data::VT_Int64:
            ss << (__int64)m_varValue;
            break;
        case cmm::variant_data::VT_UInt64:
            ss << (unsigned __int64)m_varValue;
            break;
    }

    return ss.str();
}


bool Property::isSimpleProperty(void) const
{
    return m_bSimpleProperty;
}


unsigned Property::getChildrenCount(void) const
{
    return m_vecChildrenProperty.size();
}


cmm::variant_data Property::getChildrenTitles(void) const
{
    cmm::variant_data var;
    if(m_bSimpleProperty)   return var;

    std::vector<std::string>    vecStrings;
    var.m_eValidate = cmm::variant_data::VT_StringList;
    std::vector<OpenSP::sp<Property> >::const_iterator itorProp = m_vecChildrenProperty.begin();
    for( ; itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        const OpenSP::sp<Property> &pProp = *itorProp;
        const std::string &strTitle = pProp->getTitle();
        vecStrings.push_back(strTitle);
    }
    var = vecStrings;
    return var;
}


IProperty *Property::getChild(unsigned nIndex)
{
    if(nIndex >= m_vecChildrenProperty.size())
    {
        return NULL;
    }

    Property *pProperty = m_vecChildrenProperty[nIndex].get();
    return pProperty;
}


const IProperty *Property::getChild(unsigned nIndex) const
{
    if(nIndex >= m_vecChildrenProperty.size())
    {
        return NULL;
    }

    const Property *pProperty = m_vecChildrenProperty[nIndex].get();
    return pProperty;
}


IProperty *Property::getChild(const std::string &strTitle)
{
    std::vector<OpenSP::sp<Property> >::iterator itorProp = m_vecChildrenProperty.begin();
    for( ; itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        OpenSP::sp<Property> &pProp = *itorProp;
        if(strTitle == pProp->getTitle())
        {
            return pProp.get();
        }
    }

    return NULL;
}


const IProperty *Property::getChild(const std::string &strTitle) const
{
    std::vector<OpenSP::sp<Property> >::const_iterator itorProp = m_vecChildrenProperty.begin();
    for( ; itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        const OpenSP::sp<Property> &pProp = *itorProp;
        if(strTitle == pProp->getTitle())
        {
            return pProp.get();
        }
    }

    return NULL;
}


IProperty *Property::findProperty(const std::string &strTitle, bool bCheckSelf)
{
    // 1. if self is such property, return 'this' simply
    if(bCheckSelf)
    {
        if(_stricmp(m_strTitle.c_str(), strTitle.c_str()) == 0)
        {
            return this;
        }
    }

    // 2. find the first level
    //    this step is not necessary, but I still put it here, because I want the first level of children being found at first
    std::vector<OpenSP::sp<Property> >::iterator itorProp = m_vecChildrenProperty.begin();
    for( ; itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        OpenSP::sp<Property> &pProperty = *itorProp;
        if(_stricmp(pProperty->getTitle().c_str(), strTitle.c_str()) == 0)
        {
            return pProperty.get();
        }
    }

    // 3. sail children
    for(itorProp = m_vecChildrenProperty.begin(); itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        OpenSP::sp<Property> &pProperty = *itorProp;
        IProperty *pFound = pProperty->findProperty(strTitle, false);
        if(pFound)  return pFound;
    }

    return NULL;
}


const IProperty *Property::findProperty(const std::string &strTitle, bool bCheckSelf) const
{
    // 1. if self is such property, return 'this' simply
    if(bCheckSelf)
    {
        if(m_strTitle == strTitle)
        {
            return this;
        }
    }

    // 2. find the first level
    //    this step is not necessary, but I still put it here, because I want the first level of children being found at first
    std::vector<OpenSP::sp<Property> >::const_iterator itorProp = m_vecChildrenProperty.begin();
    for( ; itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        const OpenSP::sp<Property> &pProperty = *itorProp;
        if(pProperty->getTitle() == strTitle)
        {
            return pProperty.get();
        }
    }

    // 3. sail children
    for(itorProp = m_vecChildrenProperty.begin(); itorProp != m_vecChildrenProperty.end(); ++itorProp)
    {
        const OpenSP::sp<Property> &pProperty = *itorProp;
        const IProperty *pFound = pProperty->findProperty(strTitle, false);
        if(pFound)  return pFound;
    }

    return NULL;
}


}