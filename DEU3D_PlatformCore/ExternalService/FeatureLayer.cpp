#include "FeatureLayer.h"
#include "CSimpleHttpClient.h"
#include "DEUUtils.h"
#include <sstream>

namespace deues
{
    FeatureLayer::FeatureLayer(void)
    {
        m_strFeatureType = "";
    }


    FeatureLayer::~FeatureLayer(void)
    {
    }

    void FeatureLayer::initialize(const std::string strUrl,const std::string& strVersion,const std::string& strFeatureType)
    {
        m_strUrl = strUrl;
        m_strVersion = strVersion;
        m_strFeatureType = strFeatureType;

        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<"&REQUEST=DescribeFeatureType&TypeName="<<m_strFeatureType;

        SimpleHttpClient sc;
        char* szRespBuf = NULL;
        long nResplen = 0;
        int nRet = sc.Request(GetMethod,oss.str().c_str(),&szRespBuf,&nResplen);
        if(nRet == 0 || szRespBuf != NULL)
        {
            m_strDescribeFeature = szRespBuf;
            sc.FreeResponse(szRespBuf);
        }

        DEUUtils::getProperties(m_strDescribeFeature.c_str(),strFeatureType,m_strPropertyVec);
    }
    std::string FeatureLayer::describeFeatureType() const
    {
        return m_strDescribeFeature;
    }


    std::vector<std::string> FeatureLayer::getProperties() const
    {
        return m_strPropertyVec;
    }
    std::string FeatureLayer::getAllFeature(const std::vector<std::string>& strPropertyList)
    {
        std::string strGML = "";
        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<"&REQUEST=GetFeature&TypeName="<<m_strFeatureType;

        SimpleHttpClient sc;
        char* szRespBuf = NULL;
        long nResplen = 0;
        int nRet = sc.Request(GetMethod,oss.str().c_str(),&szRespBuf,&nResplen);
        if(nRet == 0 || szRespBuf != NULL)
        {
            strGML = szRespBuf;
            sc.FreeResponse(szRespBuf);
        }
        return strGML;
    }
    std::string FeatureLayer::getFeatureByBBox(double dxmin,double dymin,double dxmax,double dymax,const std::vector<std::string>& strPropertyList)
    {
        std::string strGML = "";
        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<
            "&REQUEST=GetFeature&TypeName="<<m_strFeatureType
            <<"&BBOX="<<dxmin<<","<<dymin<<","<<dxmax<<","<<dymax;

        SimpleHttpClient sc;
        char* szRespBuf = NULL;
        long nResplen = 0;
        int nRet = sc.Request(GetMethod,oss.str().c_str(),&szRespBuf,&nResplen);
        if(nRet == 0 || szRespBuf != NULL)
        {
            strGML = szRespBuf;
            sc.FreeResponse(szRespBuf);
        }
        return strGML;
    }
    std::string FeatureLayer::getFeatureByID(const std::vector<std::string>& strIDList,const std::vector<std::string>& strPropertyList)
    {
        std::string strGML = "";
        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<
            "&REQUEST=GetFeature&TypeName="<<m_strFeatureType
            <<"&FEATUREID=";

        for(unsigned n = 0;n < strIDList.size();n++)
        {
            oss<<strIDList[n];
            if(n != strIDList.size() - 1)
            {
                oss<<",";
            }
        }

        SimpleHttpClient sc;
        char* szRespBuf = NULL;
        long nResplen = 0;
        int nRet = sc.Request(GetMethod,oss.str().c_str(),&szRespBuf,&nResplen);
        if(nRet == 0 || szRespBuf != NULL)
        {
            strGML = szRespBuf;
            sc.FreeResponse(szRespBuf);
        }
        return strGML;
    }
    std::string FeatureLayer::getFeatureByFilter(const std::string& strFilter,const std::vector<std::string>& strPropertyList)
    {
        std::string strGML = "";
        std::ostringstream oss;
        oss<<m_strUrl<<"?SERVICE=WFS&VERSION="<<m_strVersion<<
            "&REQUEST=GetFeature&TypeName="<<m_strFeatureType
            <<"&FILTER="<<strFilter;

        SimpleHttpClient sc;
        char* szRespBuf = NULL;
        long nResplen = 0;
        int nRet = sc.Request(GetMethod,oss.str().c_str(),&szRespBuf,&nResplen);
        if(nRet == 0 || szRespBuf != NULL)
        {
            strGML = szRespBuf;
            sc.FreeResponse(szRespBuf);
        }
        return strGML;
    }

}