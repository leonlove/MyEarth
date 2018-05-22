#ifndef _I_WFS_BUILDER_H_B450C2D4_1719_4436_9A13_1A59404FB211_
#define _I_WFS_BUILDER_H_B450C2D4_1719_4436_9A13_1A59404FB211_

#include <LogicalManager/ILayerManager.h>
#include <ExternalService/IWFSDriver.h>
#include "export.h"
#include "WFSDefine.h"

typedef std::map<std::string,deues::IFeatureLayer*> FEATURELAYERMAP;
typedef std::map<std::string,logical::ILayer*>      LOGICALLAYERMAP;

namespace wfsb
{
    class IWFSBuilder : public OpenSP::Ref
    {
    public:   
        virtual bool initialize(deues::IWFSDriver*    pDriver,
            logical::ILayerManager* pLayerManager) = 0;
        virtual void unInitialize() = 0;
        virtual bool downloadFeature(FEATURELAYERMAP& fMap) = 0;
        virtual bool getLayerID(const std::string& strLayerName,std::string& strID) = 0;
    };

    WFSBUILDER_API IWFSBuilder* createWFSBuilder();
}


#endif  //_I_WFS_BUILDER_H_B450C2D4_1719_4436_9A13_1A59404FB211_