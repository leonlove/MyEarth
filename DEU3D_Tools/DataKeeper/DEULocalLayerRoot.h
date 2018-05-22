#pragma once
#include "IDEULocalLayerRoot.h"
//#include "DEUDefine.h"
//#include "export.h"
//#include "DEUDataService.h"

namespace dk{
	class CDEULocalLayerRoot :	public IDEULocalLayerRoot
	{
	public:
		CDEULocalLayerRoot(void);
		~CDEULocalLayerRoot(void);

		int init(std::string strHost, std::string strPort);
		int getLogicalRLayer( ILayer** pLayer );
		int getLogicalFirstLayer( std::vector<IObject*>& vect_Layer );
		int getTerrainRLayer( ILayer** pLayer );
		int getTerrainFirstLayer( std::vector<IObject*>& vect_Layer );
		int getDomRLayer( ILayer** pLayer );
		int getDomFirstLayer( std::vector<IObject*>& vect_Layer );
		int getSubLayers(const std::string strID, std::vector<IObject*>& vect_Layer);
		int getParLayers(const std::string strID, std::vector<IObject*>& vect_Layer );
		int getSubIDs(const std::string strID, std::vector<std::string>& vect_strIDs);
		int getLayerName(const std::string strID, std::string& strName);
		int getLayerBound(const std::string strID, double** d_LayerBound);
		int getLayerBound(const std::string strID, double* px , double* py, double* pz, double* pr );
		int getObject(const std::string strID, IObject** ppObject);

		int addNewLayer(const ID id, ILayer** pLayer);
		int addNewLayer(ILayer* pParLayer, ILayer** ppLayer, DeuObjectIDType objType);  
		int addExistLayer(const std::string strPath, ILayer** pLayer);	
		int removeLayer(std::string strPath);
		int removeLayer2(const ID DEUid, ILayer* pParentLayer);
		int save();
		int saveAs();
		int submit( const ID& id, bson::bsonDocument& bsonDoc);
		int submitAll();

	private:
		std::string m_strHost;
		std::string m_strApachePort;
		ILayer* m_LogicRootLayer;
		ILayer* m_TerrRootLayer;
		OpenSP::sp<deunw::IDEUNetwork> m_IDEUNetwork;
		OpenSP::sp<deuLM::ILayerManager> m_pLayerMan;
		OpenSP::sp<deuPM::IPropertyManager>  m_pPropMan;
		
		//DEULayerPtrMap m_layerMap;
	};

}