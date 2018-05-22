#pragma once

#include "Export.h"
#include "IDriver.h"
#include "ITileSet.h"
//#include "DEUDefine.h"

namespace deues
{
	class IMercatorDriver: virtual public IDriver
	{
	public:
		virtual ITileSet* createTileSet(const std::string& strService, const std::string& strMapStyle, const std::string& strDBPath) = 0;
		virtual bool getMapServices(std::vector<DEUMorcatorInfo>& arrServices) = 0;		
	};

	DEUES_EXPORT IMercatorDriver* createMercatorDriver(void);
}
