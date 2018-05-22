#ifndef _I_DRIVER_H_FB6087B0_7584_48CE_99C4_881CB6F7A2B9_
#define _I_DRIVER_H_FB6087B0_7584_48CE_99C4_881CB6F7A2B9_

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include "DEUDefine.h"

namespace deues
{
    class IDriver : public OpenSP::Ref
    {
    public:
        virtual bool initialize(const std::string& strUrl,            //服务链接
                                const std::string& strVersion) = 0;   //版本号
        virtual std::string    getVersion() const = 0;
        virtual std::string    getUrl() const = 0;
    };
}

#endif