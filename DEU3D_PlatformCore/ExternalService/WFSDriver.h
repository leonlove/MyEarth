#ifndef _WFS_DRIVER_H_C31AEC4F_2549_4CBF_B4EB_11AD33D00F90_
#define _WFS_DRIVER_H_C31AEC4F_2549_4CBF_B4EB_11AD33D00F90_

#include "IWFSDriver.h"
#include "IOGCFilter.h"

namespace deues
{
    class WFSDriver : public IWFSDriver
    {
    public:
        explicit WFSDriver(void);
        virtual ~WFSDriver(void);
    public:
        virtual bool initialize(const std::string& strUrl,const std::string& strVersion);
        virtual IFeatureLayer* createFeatureLayer(const std::string& strFeatureType);
        virtual void removeFeatureLayer(const std::string& strFeatureType);

        virtual std::string getMetaData() const;
        virtual std::vector<std::string> getFeatureType() const;        
        virtual std::map<std::string,IFeatureLayer*> getFeatureLayer() const;
        virtual unsigned short getDataSetCode() const;
        virtual std::string    getVersion() const { return m_strVersion; }
        virtual std::string    getUrl() const {return m_strUrl; }

		virtual bool convertFilter(const std::string& strFilter,std::string& strFilterOut);
	private:
		bool createLogicalFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter);
		bool createCompareFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter);
		bool createBBoxFilter(const std::vector<std::string>& strFilterVec,IOGCFilter*& pFilter);
    private:
        std::string m_strUrl;
        std::string m_strVersion;
        std::string m_strMetaData;
        std::vector<std::string> m_strTypeVec;
        std::map<std::string,IFeatureLayer*> m_mapFeatureLayer;
    };
}

#endif //_WFS_DRIVER_H_C31AEC4F_2549_4CBF_B4EB_11AD33D00F90_