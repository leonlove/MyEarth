#ifndef _I_WFS_DRIVER_H_16D8EB0C_0119_47A3_AB4F_1D9812A521A1_
#define _I_WFS_DRIVER_H_16D8EB0C_0119_47A3_AB4F_1D9812A521A1_

#include "IDriver.h"
#include "IFeatureLayer.h"
#include "Export.h"

namespace deues
{
    class IWFSDriver : virtual public IDriver
    {
    public:
        virtual std::string getMetaData() const = 0;
        virtual std::vector<std::string> getFeatureType() const = 0;
        virtual IFeatureLayer* createFeatureLayer(const std::string& strFeatureType) = 0;
        virtual void removeFeatureLayer(const std::string& strFeatureType) = 0;
        virtual std::map<std::string,IFeatureLayer*> getFeatureLayer() const = 0;
        virtual unsigned short getDataSetCode() const = 0;
		virtual bool convertFilter(const std::string& strFilter,std::string& strFilterOut) = 0;
    };

    DEUES_EXPORT IWFSDriver* createWFSDriver(void);
    DEUES_EXPORT void freeMemory(void *p);
}


#endif//_I_WFS_DRIVER_H_16D8EB0C_0119_47A3_AB4F_1D9812A521A1_