#ifndef _I_WMS_DRIVER_H_00FDCD96_1595_4B6D_A267_4408DCDCC193_
#define _I_WMS_DRIVER_H_00FDCD96_1595_4B6D_A267_4408DCDCC193_

#include "Export.h"
#include "IDriver.h"
#include "ITileSet.h"
#include "DEUDefine.h"

namespace deues
{
	class IWMSDriver: virtual public IDriver
	{
	public:
		virtual ITileSet* createTileSet(const std::map<std::string,std::string>& strLayerMap,const std::string& strCRS) = 0;
		virtual void getLayerInfo(std::vector<DEULayerInfo>& arrLayerInfo) = 0;		
	};

	DEUES_EXPORT IWMSDriver* createWMSDriver(void);
	//DEUES_EXPORT void freeMemory(void *p);
}
#endif