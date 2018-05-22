#ifndef _WFS_FILTER_BUILDER_H_0F01980D_FF04_4FB8_8FE3_B61D772A019D_
#define _WFS_FILTER_BUILDER_H_0F01980D_FF04_4FB8_8FE3_B61D772A019D_

#include "IWFSFilterBuilder.h"
#include "ConvertGML.h"

namespace wfsb
{
    class WFSFilterBuilder : virtual public IWFSFilterBuilder
    {
    public:
        explicit WFSFilterBuilder(void);
        virtual ~WFSFilterBuilder(void);

        virtual bool initialize(deues::IWFSDriver* pDriver);
        virtual void unInitialize();
		virtual bool convertFilter(const std::string& strFilter,std::string& strFilterOut);
        virtual bool getFeatureByFilter(const std::string& strFilter,std::vector<ID>& idVec);

    protected:
        bool                               m_bInitialized;
        OpenSP::sp<deues::IWFSDriver>      m_pDriver;
        OpenSP::sp<ConvertGML>             m_pConvert;
    };
}


#endif //_WFS_FILTER_BUILDER_H_0F01980D_FF04_4FB8_8FE3_B61D772A019D_