#include "WFSFilterBuilder.h"


typedef std::map<std::string,deues::IFeatureLayer*> FEATURELAYERMAP;

namespace wfsb
{
    WFSFilterBuilder::WFSFilterBuilder(void)
    {
        m_pDriver = NULL;
        m_pConvert = NULL;

        m_bInitialized = false;
    }


    WFSFilterBuilder::~WFSFilterBuilder(void)
    {
        if(m_bInitialized)
        {
            unInitialize();
        }
    }


    IWFSFilterBuilder* createWFSFilterBuilder()
    {
        OpenSP::sp<WFSFilterBuilder> pBuilder = new WFSFilterBuilder;
        return pBuilder.release();
    }

    bool WFSFilterBuilder::initialize(deues::IWFSDriver* pDriver)
    {
        if(m_bInitialized)  return true;
        m_pDriver = pDriver;

		unsigned nDSCode = m_pDriver->getDataSetCode();
		m_pConvert = new ConvertGML();
		m_pConvert->initialize(NULL,nDSCode,pDriver->getVersion(),pDriver->getUrl());

        m_bInitialized = true;
        return true;
    }

    void WFSFilterBuilder::unInitialize()
    {
         if(!m_bInitialized) return;
         m_pConvert = NULL;
         m_bInitialized = false;
    }

	bool WFSFilterBuilder::convertFilter(const std::string& strFilter,std::string& strFilterOut)
	{
		return m_pDriver->convertFilter(strFilter,strFilterOut);
	}

    bool WFSFilterBuilder::getFeatureByFilter(const std::string& strFilter,std::vector<ID>& idVec)
    {
        idVec.clear();
        FEATURELAYERMAP mapFeatureLayer = m_pDriver->getFeatureLayer();
        FEATURELAYERMAP::const_iterator pItor = mapFeatureLayer.cbegin();
        while (pItor != mapFeatureLayer.cend())
        {
            deues::IFeatureLayer* pLayer = pItor->second;
            std::string strFeature = pLayer->getFeatureByFilter(strFilter);
            std::vector<ID> tempVec;
            m_pConvert->gml2ID(strFeature,pLayer->getFeatureType(),pLayer->getIDProperty(),pLayer->getGeometryProperty(),tempVec);
            idVec.insert(idVec.end(),tempVec.begin(),tempVec.end());
            pItor++;
        }
        return true;
    }
}

