#ifndef _WMTS_DRIVER_H_556EC408_1031_44A6_A75A_09FA0EAF478E_
#define _WMTS_DRIVER_H_556EC408_1031_44A6_A75A_09FA0EAF478E_

#include "IWMTSDriver.h"

namespace deues
{
    class WMTSDriver : public IWMTSDriver
    {
    public:
        explicit WMTSDriver(void);
        virtual ~WMTSDriver(void);
        virtual bool initialize(const std::string& strUrl,
                                const std::string& strVersion);
        virtual ITileSet* getTileSet(void);

        virtual std::string    getVersion() const { return m_strUrl; }
        virtual std::string    getUrl() const { return m_strVersion; }

    protected:
        std::string getMetaData(void) const;
        std::string getParamDefinition(void) const;

        bool QueryMetaData(std::string& strMetaData,int& nError) const;

    private:
        std::string m_strUrl;
        std::string m_strVersion;
        mutable OpenThreads::Mutex      m_driverMutex;
    };
}

#endif

