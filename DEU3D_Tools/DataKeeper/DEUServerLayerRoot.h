#pragma once
//#include "IDEULocalLayerRoot.h"
//#include "DEUDefine.h"
//#include "export.h"
//#include "DEUDataService.h"
class CDEUServerLayerRoot :	public OpenSP::Ref
{
public:
	CDEUServerLayerRoot(void);
	~CDEUServerLayerRoot(void);
	int init(std::string strHost, std::string strPort);
	int getLogicalRLayer( ILayer** pLayer );
	int getTerrainRLayer( ILayer** pLayer );
	//int getLayer(ID DEUid, ILayer** pLayer);
	int submit();
	int deleteID(ID DEUid);
	int deleteAll();

private:
	std::string m_strHost;
	std::string m_strApachePort;
	ILayer* m_LogicRootLayer;
	ILayer* m_TerrRootLayer;
	OpenSP::sp<deuLM::ILayerManager> m_pLayerMan;
	OpenSP::sp<deuPM::IPropertyManager>  m_pPropMan;
	//DEULayerPtrMap m_layerMap;
};

