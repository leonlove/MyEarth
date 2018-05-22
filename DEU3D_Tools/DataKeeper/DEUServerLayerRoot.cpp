#include "DEUServerLayerRoot.h"
#include <Network/IDEUNetwork.h>


CDEUServerLayerRoot::CDEUServerLayerRoot(void)
{
}


CDEUServerLayerRoot::~CDEUServerLayerRoot(void)
{
}

int CDEUServerLayerRoot::init(std::string strHost, std::string strPort)
{
	m_strHost =strHost;
	m_strApachePort = strPort;
	return 1;
}

int CDEUServerLayerRoot::getLogicalRLayer(ILayer** pLayer)
{
	OpenSP::sp<ILayerManager>  _layer_man;
	_layer_man = createLayerManager();
	OpenSP::sp<deunw::IDEUNetwork> p_IDEUNetwork;
	p_IDEUNetwork = deunw::createDEUNetwork();
	
	p_IDEUNetwork->initialize(m_strHost, m_strApachePort);
	//_layer_man->setDEUNetwork(p_IDEUNetwork);
	m_LogicRootLayer = _layer_man->getLogicalRootLayer();
	*pLayer = m_LogicRootLayer;

	return 0;
}

int CDEUServerLayerRoot::getTerrainRLayer(ILayer** pLayer)
{
	OpenSP::sp<ILayerManager>  _layer_man;
	_layer_man = createLayerManager();
	OpenSP::sp<deunw::IDEUNetwork> p_IDEUNetwork;
	p_IDEUNetwork = deunw::createDEUNetwork();

	p_IDEUNetwork->initialize(m_strHost, m_strApachePort);
	//_layer_man->setDEUNetwork(p_IDEUNetwork);	
	m_TerrRootLayer = _layer_man->getTerrainRootLayer();
	*pLayer = m_TerrRootLayer;

	return 0;
}
//int CDEUServerLayerRoot::getLayer(ID DEUid, ILayer** pLayer)
//{
//	return S_OK;
//}
int CDEUServerLayerRoot::submit()
{
	return 0;
}

int CDEUServerLayerRoot::deleteID(ID DEUid)
{
	return 0;
}

int CDEUServerLayerRoot::deleteAll()
{
	return 0;
}
