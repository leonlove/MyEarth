#ifndef _I_WFS_FILTER_BUILDER_H_69D49B37_B3C7_45AA_9316_73016C23F3A1_
#define _I_WFS_FILTER_BUILDER_H_69D49B37_B3C7_45AA_9316_73016C23F3A1_

#include <ExternalService/IWFSDriver.h>
#include <IDProvider/ID.h>
#include "export.h"
#include "WFSDefine.h"

namespace wfsb
{
    class IWFSFilterBuilder : public OpenSP::Ref
    {
    public:   
        virtual bool initialize(deues::IWFSDriver* pDriver) = 0;
        virtual void unInitialize() = 0;
		virtual bool convertFilter(const std::string& strFilter,std::string& strFilterOut) = 0;
		virtual bool getFeatureByFilter(const std::string& strFilter,std::vector<ID>& idVec) = 0;
    };

    WFSBUILDER_API IWFSFilterBuilder* createWFSFilterBuilder();
}
#endif //_I_WFS_FILTER_BUILDER_H_69D49B37_B3C7_45AA_9316_73016C23F3A1_