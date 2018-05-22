#include "DEULocalLayerRoot.h"
#include <comutil.h>
#include <Network/IDEUNetwork.h>

extern OpenSP::sp<deunw::IDEUNetwork> g_pNetwork;

namespace dk
{

	IDEULocalLayerRoot* createDEULocalLayerRoot(void)
	{
		OpenSP::sp<CDEULocalLayerRoot> pLocalLayerRoot = new CDEULocalLayerRoot;
		return pLocalLayerRoot.release();
	}

	CDEULocalLayerRoot::CDEULocalLayerRoot(void)
	{
		m_IDEUNetwork = NULL;
		m_pLayerMan = NULL;
		m_pPropMan = NULL;
		m_LogicRootLayer = NULL;
		m_TerrRootLayer = NULL;
	}


	CDEULocalLayerRoot::~CDEULocalLayerRoot(void)
	{
	}

	int CDEULocalLayerRoot::init(std::string strHost, std::string strPort)
	{
		m_strHost =strHost;
		m_strApachePort = strPort;
		if(m_IDEUNetwork == NULL)
		{
			m_IDEUNetwork = deunw::createDEUNetwork();
			if(!m_IDEUNetwork->initialize(m_strHost, m_strApachePort))
			{
				return false;
			}
		}

		if(m_pPropMan ==NULL)
		{
			m_pPropMan = dynamic_cast<deuPM::IPropertyManager*>(deuPM::createPropertyManager());
			m_pPropMan->setDEUNetwork(m_IDEUNetwork);
		}

		if(m_pLayerMan == NULL)
		{
			m_pLayerMan = deuLM::createLayerManager();
			if(!m_pLayerMan->initialize(m_pPropMan))
			{
				return false;
			}
		}

		return true;
	}

	int CDEULocalLayerRoot::getLogicalRLayer(ILayer** pLayer)
	{	
		if(m_IDEUNetwork == NULL)
		{
			return -1;
		}
		if(m_pPropMan ==NULL)
		{
			return -1;	
		}
		if(m_pLayerMan == NULL)
		{
			return -1;
		}
		m_LogicRootLayer = m_pLayerMan->getLogicalRootLayer();
		*pLayer = m_LogicRootLayer;
		std::string strLayerName;
		strLayerName = ((IObject*)m_LogicRootLayer)->getName();
		//unsigned k = m_LogicRootLayer->getChildrenCount();
		//int lLayeCnt = m_LogicRootLayer->getChildrenCount();
		//IObject* pObj = m_LogicRootLayer->getChild(0);
		//const std::string strName = pObj->getName();
		return S_OK;
	}

	int CDEULocalLayerRoot::getTerrainRLayer(ILayer** pLayer)
	{
		if(m_IDEUNetwork == NULL)
		{
			return -1;
		}
		if(m_pPropMan ==NULL)
		{
			return -1;	
		}
		if(m_pLayerMan == NULL)
		{
			return -1;
		}
		
		m_TerrRootLayer = m_pLayerMan->getTerrainRootLayer();
		*pLayer = m_TerrRootLayer;
		return S_OK;
	}

	int CDEULocalLayerRoot::getLogicalFirstLayer( std::vector<IObject*>& vect_Layer )
	{
		vect_Layer.clear();
		int lLayeCnt = m_LogicRootLayer->getChildrenCount();
		int i = 0;
		IObject* pObj = NULL;
		ILayer* pLayer = NULL;
		for (i=0; i<lLayeCnt; i++)
		{
			pObj = m_LogicRootLayer->getChild(i);
			pLayer = pObj->asLayer();
			if(pLayer)
			{
				vect_Layer.push_back(pLayer);
			}
		}
		return S_OK;
	}

	int CDEULocalLayerRoot::getTerrainFirstLayer( std::vector<IObject*>& vect_Layer )
	{
		vect_Layer.clear();
		int lLayeCnt = m_TerrRootLayer->getChildrenCount();
		int i = 0;
		IObject* pObj = NULL;
		//ILayer* pLayer = NULL;
		for (i=0; i<lLayeCnt; i++)
		{
			pObj = m_TerrRootLayer->getChild(i);
			if(pObj)
			{
				std::string strName = pObj->getName();
				DeuObjectIDType objType = pObj->getType();
				if (objType == TERRAIN_DEM_LAYER_ID || objType == TERRAIN_DEM_ID)
				{
					vect_Layer.push_back(pObj);
				}
				//pLayer = pObj->asLayer();
			}
			
		}
		return S_OK;
	}

	int CDEULocalLayerRoot::getDomRLayer( ILayer** ppLayer )
	{
		return getTerrainRLayer(ppLayer);
	}

	int CDEULocalLayerRoot::getDomFirstLayer( std::vector<IObject*>& vect_Layer )
	{
		vect_Layer.clear();
		int lLayeCnt = m_TerrRootLayer->getChildrenCount();
		int i = 0;
		IObject* pObj = NULL;
		//ILayer* pLayer = NULL;
		for (i=0; i<lLayeCnt; i++)
		{
			pObj = m_TerrRootLayer->getChild(i);
			if(pObj)
			{
				std::string strName = pObj->getName();
				DeuObjectIDType objType = pObj->getType();
				if (objType == TERRAIN_DOM_LAYER_ID || objType == TERRAIN_DOM_ID)
				{
					vect_Layer.push_back(pObj);
				}				
			}

		}
		return S_OK;
	}

	int CDEULocalLayerRoot::getSubLayers(const std::string strID, std::vector<IObject*>& vect_Layer)
	{
		if(m_pLayerMan == NULL)
			return -1;
		int hr = -1;
		vect_Layer.clear();
		ID deuID;
		deuID.fromString(strID);
		IObject* pObj = m_pLayerMan->findObject(deuID);
		if (pObj!=NULL)
		{
			ILayer* pLayer = pObj->asLayer();
			IObject* PObjSub = NULL;
			if (pLayer != NULL)
			{
				int i = 0;
				int lLayerCnt = pLayer->getChildrenCount();
				for (i=0; i < lLayerCnt; i++ )
				{
					PObjSub = pLayer->getChild( i );
					vect_Layer.push_back( PObjSub );
				}			
			}
		}
		return S_OK;
	}

	int CDEULocalLayerRoot::getSubIDs(const std::string strID, std::vector<std::string>& vect_strIDs)
	{
		vect_strIDs.clear();
		OpenSP::sp<deuPM::IProperty> pProperty = m_pPropMan->findProperty(ID::genIDfromString(strID));
		if (pProperty==NULL)
		{
			return -1;
		}
		OpenSP::sp<deuPM::IProperty> pProperty_sub = pProperty->getChild("ChildrenID");
		if (pProperty_sub != NULL)
		{
			long lsz = pProperty_sub->getChildrenCount();
		std::string strTmp;
		for (int i=0; i < lsz; i++)
		{			
			OpenSP::sp<deuPM::IProperty> pProperty_sub2 = pProperty_sub->getChild(i);
			long lsz2 = pProperty_sub2->getChildrenCount();
			if (lsz2 > 0)
			{
				for (int j = 0; j< lsz2; j++)
				{
					OpenSP::sp<deuPM::IProperty> pProperty_sub3 = pProperty_sub2->getChild(i);
					if(pProperty_sub3!=NULL)
						strTmp = pProperty_sub3->getTitle();
				}
			}
			else
			{
				//strTmp = pProperty_sub2->getTitle();
				strTmp = pProperty_sub2->getValueAsString();
			}			
			vect_strIDs.push_back( strTmp );
			
		}
		}
		
		return S_OK;
	}

	int CDEULocalLayerRoot::getParLayers( const std::string strID, std::vector<IObject*>& vect_Layer )
	{ 	
		if(m_pLayerMan == NULL)
			return -1;
		int hr = -1;
		vect_Layer.clear();
		ID deuID;
		deuID.fromString(strID);
		IObject* pObj = m_pLayerMan->findObject(deuID);
		if ( pObj == NULL )
		{
			return S_OK;
		}
		ILayer* pLayer = pObj->asLayer();
		IObject* PObjSub = NULL;
		if (pLayer != NULL)
		{
			int i = 0;
			int lLayerCnt = pLayer->getParentCount();
			for (i=0; i < lLayerCnt; i++ )
			{
				PObjSub = pLayer->getParent( i );
				vect_Layer.push_back( PObjSub );
			}			
		}
		return S_OK;		
	}

	//DEUid为父ID
	int CDEULocalLayerRoot::addNewLayer(const ID DEUid, ILayer** ppLayer)  
	{
		int hr = -1;
		*ppLayer = NULL;
		DeuObjectIDType objType; // = (DeuObjectIDType)DEUid.TileID.m_nType;
		//int layerType = 1; //1地形  2影像   3模型
		//deuLM::ILayer* pTopLayer;
		//if (objType==1)
		//{
		//	pTopLayer = m_TerrRootLayer;
		//}
		//else if (layerType == LOGICAL_LAYER_ID)
		//{
		//	pTopLayer = m_LogicRootLayer;
		//}
		//else
		//	hr = -1;
		deuLM::ILayer *pTerrainRootLayer = m_pLayerMan->getTerrainRootLayer();
		deuLM::IObject* pObj = m_pLayerMan->findObject(DEUid);			    //, pTerrainRootLayer
		if (pObj)
		{
			deuLM::ILayer* pParLayer = pObj->asLayer();
			if (pParLayer)
			{
				objType = pParLayer->getType();
				*ppLayer = pParLayer->createSubLayer(objType);
				hr = S_OK;
			}
			else
				hr = -1;
		}

		return hr;
	}

	int CDEULocalLayerRoot::addNewLayer(ILayer* pParLayer, ILayer** ppLayer, DeuObjectIDType objType)
	{
		//DeuObjectIDType objType_ = pParLayer->getType();
		if (pParLayer!= NULL)
		{			
			if (objType == UNKNOWN_TPYE)
			{
				objType = pParLayer->getType();
			}			
			*ppLayer = pParLayer->createSubLayer(objType);			
		}
		return S_OK;
	}

	int CDEULocalLayerRoot::addExistLayer(const std::string strPath, ILayer** ppLayer)
	{
		return S_OK;
	}


	int CDEULocalLayerRoot::removeLayer(std::string strPath)
	{		
		return S_OK;
	}
	int CDEULocalLayerRoot::removeLayer2(const ID DEUid, ILayer* pParentLayer)
	{
		bool b = false;
		if (pParentLayer!= NULL)
		{			
			b = pParentLayer->removeChild( DEUid );			
		}
		return b?S_OK:E_FAIL;
	}
	int CDEULocalLayerRoot::save()
	{
		return true;
	}
	int CDEULocalLayerRoot::saveAs()
	{
		return true;
	}

	int CDEULocalLayerRoot::submit(const ID& id, bson::bsonDocument& bsonDoc)
	{
#ifdef _DEBUG
		std::string strid = id.toString();
#endif
		std::string strJsonString;
		bsonDoc.JsonString(strJsonString);

		bson::bsonStream ss;
		bsonDoc.Write(&ss);
		void* pBuffer = ss.Data();
		unsigned nBufLen = ss.DataLen();
		std::vector<std::string> errVec;
		int nErrorCode = 1;
		bool b = g_pNetwork->addLayer(id,pBuffer,nBufLen,errVec,&nErrorCode);
		return b?S_OK:-1;
	}
	int CDEULocalLayerRoot::submitAll()
	{		
		bool b = false;
		if(m_pLayerMan != NULL)
		{
			b = m_pLayerMan->update();
		}

		return b?S_OK:E_FAIL;
	}
	
	int CDEULocalLayerRoot::getLayerName(const std::string strID, std::string& strName)
	{
		int hr = -1;
		if(m_pLayerMan == NULL)
			return hr;				
		ID deuID;
		deuID.fromString(strID);
		IObject* pObj = m_pLayerMan->findObject(deuID);		
		if (pObj != NULL)
		{
			strName = pObj->getName();
			hr = S_OK;
		}
		return hr ;
	}

	//获得服务器某个layer的bound
	int CDEULocalLayerRoot::getLayerBound(const std::string strID, double** pp_dLayerBound)
	{
		//*pp_dLayerBound = new double[4];		
		int hr = -1;
		if(m_pLayerMan != NULL)
		{
			IObject* pObj = NULL;
			ID deuID;
			deuID.fromString(strID);			
			pObj = m_pLayerMan->findObject(deuID);		
			
			if (pObj != NULL)
			{
				ILayer* pLayer = pObj->asLayer();
				if (pLayer)
				{
					cmm::math::Sphered s = pLayer->getBound();
					cmm::math::Point3<double> p = s.getCenter();
					double rad = s.getRadius();
					(*pp_dLayerBound)[0] = p.x();
					(*pp_dLayerBound)[1] = p.y();
					(*pp_dLayerBound)[2] = p.z();
					(*pp_dLayerBound)[3] = rad;					
					hr = S_OK;
				}				
			}
		}
		return hr ;
	}

	int CDEULocalLayerRoot::getLayerBound(const std::string strID, double* px , double* py, double* pz, double* pr )
	{
		//*pp_dLayerBound = new double[4];		
		int hr = -1;
		if(m_pLayerMan != NULL)
		{
			ID deuID;
			deuID.fromString(strID);
			IObject* pObj = m_pLayerMan->findObject(deuID);		
			if (pObj != NULL)
			{
				ILayer* pLayer = pObj->asLayer();
				if (pLayer)
				{
					cmm::math::Sphered s = pLayer->getBound();
					cmm::math::Point3<double> p = s.getCenter();
					double rad = s.getRadius();
					*px = p.x();
					*py = p.y();
					*pz = p.z();
					*pr = rad;					
					hr = S_OK;
				}				
			}
		}
		return hr ;
	}

	int CDEULocalLayerRoot::getObject(const std::string strID, IObject** ppObject)
	{
		if(m_pLayerMan == NULL)
			return -1;
		if (ppObject == NULL)
			return -1;		
		ID deuID;
		deuID.fromString(strID);
		*ppObject = m_pLayerMan->findObject(deuID);
		return S_OK;
	}
}

