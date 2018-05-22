#include "OGCFilter.h"
#include "BBoxFilter.h"
#include "CompareFilter.h"
#include <sstream>

OGCFilter::OGCFilter(void)
{
}


OGCFilter::~OGCFilter(void)
{
}

IOGCFilter* createFilter(FilterType fType)
{
    switch(fType)
    {
    case Logical_And:
    case Logical_Or:
    case Logical_Not:
    case Logical_EndAnd:
    case Logical_EndOr:
        {
            OGCFilter* pFilter = new OGCFilter();
            return pFilter;
        }
    case Compare_EqualTo:
    case Compare_NotEqualTo:
    case Compare_LessThan:
    case Compare_GreaterThan:
    case Compare_LessThanEqualTo:
    case Compare_GreaterThanEqualTo:
    case Compare_Like:
    case Compare_Between:
        {
            CompareFilter* pFilter = new CompareFilter();
            return pFilter;
        }
    case BBOX:
        {
            BBoxFilter* pFilter = new BBoxFilter();
            return pFilter;
        }
    case ID:
        {
            return NULL;
        }

    }
    return NULL;
}

IOGCFilter* OGCFilter::getFilter(unsigned nIndex)
{
    if(nIndex < 0 || nIndex >= m_pFilterVec.size())
    {
        return NULL;
    }

    IOGCFilter* pFilter = m_pFilterVec[nIndex];
    return pFilter;
}

void OGCFilter::addFilter(IOGCFilter* pFilter)
{
    m_pFilterVec.push_back(pFilter);
}

void OGCFilter::setFilter(unsigned nIndex,IOGCFilter* pFilter)
{
    if(nIndex < 0 || nIndex >= m_pFilterVec.size())
    {
        return;
    }
    m_pFilterVec[nIndex] = pFilter;
}

void OGCFilter::removeFilter(unsigned nIndex)
{
    if(nIndex < 0 || nIndex >= m_pFilterVec.size())
    {
        return;
    }
    
}

std::string OGCFilter::toString()
{
    std::string str = "";

    std::ostringstream oss;
    oss<<"(<ogc:Filter%20xmlns:ogc=\"http://www.opengis.net/ogc\"%20xmlns:gml=\"http://www.opengis.net/gml\">";

    for (unsigned n = 0;n < m_pFilterVec.size();n++)
    {
        IOGCFilter* pFilter = m_pFilterVec[n];
        switch(pFilter->getFilterType())
        {
        case Logical_And:
            {
                oss<<"<ogc:And>";
                continue;
            }
        case Logical_Or:
            {
                oss<<"<ogc:Or>";
                continue;
            }
        case Logical_EndAnd:
            {
                oss<<"</ogc:And>";
                continue;
            }
        case Logical_EndOr:
            {
                oss<<"</ogc:Or>";
                continue;
            }
        case Logical_Not:
            {
                oss<<"<ogc:Not>";
                continue;
            }
        case Logical_EndNot:
            {
                oss<<"</ogc:Not>";
                continue;
            }
        case Compare_EqualTo:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsEqualTo>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsEqualTo>";
                continue;
            }
        case Compare_NotEqualTo:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsNotEqualTo>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsNotEqualTo>";
                continue;
            }
        case Compare_GreaterThan:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsGreaterThan>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsGreaterThan>";
                continue;
            }
        case Compare_LessThan:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsLessThan>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsLessThan>";
                continue;
            }
        case Compare_GreaterThanEqualTo:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsGreaterThanOrEqualTo>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsGreaterThanOrEqualTo>";
                continue;
            }
        case Compare_LessThanEqualTo:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsLessThanOrEqualTo>";
                oss<<"<ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName>";
                oss<<"<ogc:Literal>";
                oss<<pCompareFilter->getLiteral();
                oss<<"</ogc:Literal>";
                oss<<"</ogc:PropertyIsLessThanOrEqualTo>";
                continue;
            }
        case Compare_Between:
            {
                ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pFilter);
                oss<<"<ogc:PropertyIsBetween><ogc:PropertyName>";
                oss<<pCompareFilter->getPropertyName();
                oss<<"</ogc:PropertyName><ogc:LowerBoundary><ogc:Literal>";
                oss<<pCompareFilter->getLiteral()<<"</ogc:Literal></ogc:LowerBoundary>";
                oss<<"<ogc:UpperBoundary><ogc:Literal>";
                oss<<pCompareFilter->getLiteral2()<<"</ogc:Literal></ogc:UpperBoundary>";
                oss<<"</ogc:PropertyIsBetween>";
            }
        case BBOX:
            {
                IBBoxFilter* pBBoxFilter = dynamic_cast<IBBoxFilter*>(pFilter);
                double dMinX = 0.0;
                double dMinY = 0.0;
                double dMaxX = 0.0;
                double dMaxY = 0.0;
                pBBoxFilter->getBBox(dMinX,dMinY,dMaxX,dMaxY);
                oss<<"<ogc:BBOX><ogc:PropertyName>";
                oss<<pBBoxFilter->getPropertyName();
                oss<<"</ogc:PropertyName><gml:Envelope><gml:lowerCorner>";
                oss<<dMinY<<" "<<dMinX;
                oss<<"</gml:lowerCorner><gml:upperCorner>";
                oss<<dMaxY<<" "<<dMaxX;
                oss<<"</gml:upperCorner></gml:Envelope></ogc:BBOX>";
            }
        default:
            continue;
        }

    }

    oss<<"</ogc:Filter>)";
    str = oss.str();
    return str;
}