#include "WFSDriver.h"
#include "FeatureLayer.h"
#include <sstream>
#include "CSimpleHttpClient.h"
#include "DEUUtils.h"
#include <common/md2.h>
#include "OGCFilter.h"
#include "ICompareFilter.h"
#include "IBBoxFilter.h"

namespace deues
{

    WFSDriver::WFSDriver(void)
    {
    }


    WFSDriver::~WFSDriver(void)
    {
    }

    IWFSDriver *createWFSDriver(void)
    {
        OpenSP::sp<WFSDriver> pDriver = new WFSDriver;
        return pDriver.release();
    }

    bool WFSDriver::initialize(const std::string& strUrl,const std::string& strVersion)
    {
        int nFind = strUrl.rfind('?');
        if(nFind == -1)
        {
            m_strUrl = strUrl;
        }
        else
        {
            m_strUrl = strUrl.substr(0,nFind);
        }
       m_strVersion = strVersion;
       //get meta data
       std::ostringstream oss;
       oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<"&REQUEST=GetCapabilities"<<'\0';

       SimpleHttpClient shc;
       char* szRespBuf = NULL;
       long nResplen = 0;
       int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
       //return
       if(nRet == 0 || szRespBuf != NULL)
       {
           unsigned i = 0;
           for(i = nResplen - 1;i >= 0;i--)
           {
               if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
               {
                   break ;
               }
           }
           szRespBuf[i+1] = 0;
           m_strMetaData = szRespBuf;
           shc.FreeResponse(szRespBuf);
       }
       else
       {
           return false;
       }
       //get feature types from meta data
       if(!DEUUtils::getFeatureTypes(m_strMetaData.c_str(),m_strTypeVec))
       {
           return false;
       }
       return true;
    }

    std::string WFSDriver::getMetaData() const
    {
        return m_strMetaData;
    }

    std::vector<std::string> WFSDriver::getFeatureType() const
    {
        return m_strTypeVec;
    }

    IFeatureLayer* WFSDriver::createFeatureLayer(const std::string& strFeatureType)
    {
        FeatureLayer* pLayer = new FeatureLayer;
        pLayer->initialize(m_strUrl,m_strVersion,strFeatureType);
        m_mapFeatureLayer[strFeatureType] = pLayer;
        return pLayer;
    }

    void WFSDriver::removeFeatureLayer(const std::string& strFeatureType)
    {
        std::map<std::string,IFeatureLayer*>::iterator pItr = m_mapFeatureLayer.find(strFeatureType);
        if(pItr == m_mapFeatureLayer.end())
        {
            return;
        }
        IFeatureLayer* pLayer = m_mapFeatureLayer[strFeatureType];
        m_mapFeatureLayer.erase(strFeatureType);
        delete pLayer;
    }

    std::map<std::string,IFeatureLayer*> WFSDriver::getFeatureLayer() const
    {
        return m_mapFeatureLayer;
    }

    unsigned short WFSDriver::getDataSetCode() const
    {
        return cmm::createHashMD2(m_strUrl.c_str(),m_strUrl.length());
    }

	bool WFSDriver::createLogicalFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter)
	{
		if(strFilterVec.size() < 2)
		{
			return false;
		}
		std::string strType = strFilterVec[1];
		FilterType fType;
		if(strType == "And")
		{
			fType = Logical_And;
		}
		else if(strType == "Or")
		{
			fType = Logical_Or;
		}
		else if(strType == "EndAnd")
		{
			fType = Logical_EndAnd;
		}
		else if(strType == "EndOr")
		{
			fType = Logical_EndOr;
		}
		else if(strType == "Not")
		{
			fType = Logical_Not;
		}
		else if(strType == "EndNot")
		{
			fType = Logical_EndNot;
		}
		else
		{
			return false;
		}

		IOGCFilter* pSubFilter = createFilter(fType);
		pSubFilter->setFilterType(fType);
		pFilter->addFilter(pSubFilter);
		return true;
	}
	
	bool WFSDriver::createCompareFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter)
	{
		if(strFilterVec.size() < 4)
		{
			return false;
		}
		std::string strType = strFilterVec[2];
		FilterType fType;
		if(strType == "EqualTo")
		{
			fType = Compare_EqualTo;
		}
		else if(strType == "NotEqualTo")
		{
			fType = Compare_NotEqualTo;
		}
		else if(strType == "LessThan")
		{
			fType = Compare_LessThan;
		}
		else if(strType == "GreaterThan")
		{
			fType = Compare_GreaterThan;
		}
		else if(strType == "LessThanEqualTo")
		{
			fType = Compare_LessThanEqualTo;
		}
		else if(strType == "GreaterThanEqualTo")
		{
			fType = Compare_GreaterThanEqualTo;
		}
		else if(strType == "Like")
		{
			fType =  Compare_Like;
		}
		else if(strType == "Between")
		{
			fType = Compare_Between;
			if(strFilterVec.size() < 5)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		IOGCFilter* pSubFilter = createFilter(fType);
		ICompareFilter* pCompareFilter = dynamic_cast<ICompareFilter*>(pSubFilter);
		pCompareFilter->setFilterType(fType);
		pCompareFilter->setPropertyName(strFilterVec[1]);
		pCompareFilter->setLiteral(strFilterVec[3]);
		if(strType == "Between")
		{
			pCompareFilter->setLiteral2(strFilterVec[4]);
		}
		pFilter->addFilter(pCompareFilter);
		return true;
	}

	bool WFSDriver::createBBoxFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter)
	{
		if(strFilterVec.size() < 5)
		{
			return false;
		}

		IOGCFilter* pSubFilter = createFilter(BBOX);
		IBBoxFilter* pBBoxFilter = dynamic_cast<IBBoxFilter*>(pSubFilter);
		pBBoxFilter->setFilterType(BBOX);
		pBBoxFilter->setBBox(atof(strFilterVec[1].c_str()),atof(strFilterVec[2].c_str()),atof(strFilterVec[3].c_str()),atof(strFilterVec[4].c_str()));
		pFilter->addFilter(pBBoxFilter);
		return true;
	}

	bool WFSDriver::convertFilter(const std::string& strFilter,std::string& strFilterOut)
	{
		strFilterOut = "";
		std::vector<std::string> strFilterVec;
		char* chValue = (char*)strFilter.c_str();
		char* chRes = strtok(chValue,";");
		while(chRes)
		{
			strFilterVec.push_back(chRes);
			chRes = strtok(NULL,";");
		}

		if(strFilterVec.empty())
		{
			return false;
		}

		IOGCFilter* pFilter = new OGCFilter();

		for(unsigned n = 0;n < strFilterVec.size();n++)
		{
			std::vector<std::string> strTempVec;
			char* chValue1 = (char*)strFilterVec[n].c_str();
			char* chRes1 = strtok(chValue1," ");
			while(chRes1)
			{
				strTempVec.push_back(chRes1);
				chRes1 = strtok(NULL," ");
			}

			if(strTempVec.empty())
			{
				continue;
			}
			std::string strType = strTempVec[0];
			if(strType == "Logical")
			{
				if(!createLogicalFilter(strTempVec,pFilter))
				{
					return false;
				}
			}
			else if(strType == "Compare")
			{
				if(!createCompareFilter(strTempVec,pFilter))
				{
					return false;
				}
			}
			else if(strType == "BBox")
			{
				if(!createBBoxFilter(strFilterVec,pFilter))
				{
					return false;
				}
			}
			else
			{
				continue;
			}
		}

		strFilterOut = pFilter->toString();
		return true;
	}
}