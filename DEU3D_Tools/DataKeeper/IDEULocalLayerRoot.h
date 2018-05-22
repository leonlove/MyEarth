#include "export.h"
#include <OpenSP/Ref.h>
#include "IDProvider/ID.h"
#include <LayerManager/ILayer.h>
#include <LayerManager/IObject.h>
#include <LayerManager/IInstance.h>
#include <LayerManager/ILayerManager.h>
#include <PropertyManager/IPropertyManager.h>	
#include <PropertyManager/IProperty.h>
using namespace deuLM;

namespace dk
{
	class IDEULocalLayerRoot :	public OpenSP::Ref
	{
	public:
		virtual int init(std::string strHost, std::string strPort) = 0;
		virtual int getLogicalRLayer( ILayer** pLayer ) = 0;
		virtual int getLogicalFirstLayer( std::vector<IObject*>& vect_Layer ) = 0;
		virtual int getTerrainRLayer( ILayer** pLayer ) = 0;
		virtual int getTerrainFirstLayer( std::vector<IObject*>& vect_Layer ) = 0;
		virtual int getDomRLayer( ILayer** pLayer ) = 0;
		virtual int getDomFirstLayer( std::vector<IObject*>& vect_Layer ) = 0;
		virtual int getSubLayers(const std::string strID, std::vector<IObject*>& vect_Layer) = 0;
		virtual int getParLayers( const std::string strID, std::vector<IObject*>& vect_Layer ) = 0;
		virtual int getSubIDs(const std::string strID, std::vector<std::string>& vect_strIDs)=0;
		virtual int getLayerName(const std::string strID, std::string& strName) = 0;
		virtual int getLayerBound(const std::string strID, double** d_LayerBound) = 0;
		virtual int getLayerBound(const std::string strID, double* px , double* py, double* pz, double* pr ) = 0;
		virtual int getObject(const std::string strID, IObject** ppObject) = 0;

		virtual int addNewLayer(const ID id, ILayer** pLayer) = 0;
		virtual int addNewLayer(ILayer* pParLayer, ILayer** ppLayer,  DeuObjectIDType objType) = 0;  
		virtual int addExistLayer(const std::string strPath, ILayer** pLayer) = 0;	
		virtual int removeLayer(std::string strPath) = 0;
		virtual int removeLayer2(const ID DEUid, ILayer* pParentLayer) = 0;
		virtual int save() = 0;
		virtual int saveAs() = 0;
		virtual int submit(const ID& id, bson::bsonDocument& bsonDoc) = 0;
		virtual int submitAll() = 0;
	};


	DATAKEEPER_API IDEULocalLayerRoot* createDEULocalLayerRoot(void);
}
