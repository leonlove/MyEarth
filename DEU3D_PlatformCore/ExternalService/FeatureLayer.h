#ifndef _FEATURE_LAYER_H_DFBE7BC3_0B69_4E31_A06D_D67CD468E09C_
#define _FEATURE_LAYER_H_DFBE7BC3_0B69_4E31_A06D_D67CD468E09C_

#include "IFeatureLayer.h"

namespace deues
{
    class FeatureLayer : public IFeatureLayer
    {
    public:
        explicit FeatureLayer(void);
        virtual ~FeatureLayer(void);

        virtual void        initialize(const std::string strUrl,           //服务链接URL
            const std::string& strVersion,      //服务版本号
            const std::string& strFeatureType); //FeatureType名称
        virtual std::string describeFeatureType() const;
        virtual void        setIDProperty(const std::string& strIDProperty) { m_strID = strIDProperty; }
        virtual void        setGeometryProperty(const std::string& strGeometryProperty) { m_strGeometry = strGeometryProperty; }
        virtual std::string getIDProperty() const { return m_strID; }
        virtual std::string getGeometryProperty() const { return m_strGeometry; }

        virtual std::string getUrl() const { return m_strUrl; }
        virtual std::string getVersion() const { return m_strVersion; }
        virtual std::string getFeatureType() const { return m_strFeatureType; }
        virtual std::vector<std::string> getProperties() const;

        virtual std::string getAllFeature(const std::vector<std::string>& strPropertyList);
        virtual std::string getFeatureByBBox(double dxmin,double dymin,double dxmax,double dymax,const std::vector<std::string>& strPropertyList);
        virtual std::string getFeatureByID(const std::vector<std::string>& strIDList,const std::vector<std::string>& strPropertyList);
        virtual std::string getFeatureByFilter(const std::string& strFilter,const std::vector<std::string>& strPropertyList);
        
    private:
        std::string m_strUrl;
        std::string m_strVersion;
        std::string m_strFeatureType;
        std::string m_strID;
        std::string m_strGeometry;
        std::string m_strDescribeFeature;
        std::vector<std::string> m_strPropertyVec;
    };
}


#endif //_FEATURE_LAYER_H_DFBE7BC3_0B69_4E31_A06D_D67CD468E09C_
