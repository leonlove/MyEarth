#ifndef _I_FEATURE_LAYER_H_783770E4_551C_4366_B2CB_B0EF945CB114_
#define _I_FEATURE_LAYER_H_783770E4_551C_4366_B2CB_B0EF945CB114_

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <string>
#include <vector>

namespace deues
{
    class IFeatureLayer : public OpenSP::Ref
    {
    public:
        virtual void        initialize(const std::string strUrl,               //服务链接URL
                                       const std::string& strVersion,          //服务版本号
                                       const std::string& strFeatureType) = 0; //FeatureType名称
        
        virtual void        setIDProperty(const std::string& strIDProperty) = 0;
        virtual void        setGeometryProperty(const std::string& strGeometryProperty) = 0;
        virtual std::string getIDProperty() const = 0;
        virtual std::string getGeometryProperty() const = 0;
        
        virtual std::string  describeFeatureType() const = 0;
        virtual std::string  getUrl() const = 0;
        virtual std::string  getVersion() const = 0;
        virtual std::string  getFeatureType() const = 0;
        virtual std::vector<std::string> getProperties()const  = 0;

        virtual std::string getAllFeature(const std::vector<std::string>& strPropertyList = std::vector<std::string>()) = 0;
        virtual std::string getFeatureByBBox(double dxmin,double dymin,double dxmax,double dymax,
                                            const std::vector<std::string>& strPropertyList = std::vector<std::string>()) = 0;
        virtual std::string getFeatureByID(const std::vector<std::string>& strIDList,
                                           const std::vector<std::string>& strPropertyList = std::vector<std::string>()) = 0;
        virtual std::string getFeatureByFilter(const std::string& strFilter,
                                               const std::vector<std::string>& strPropertyList = std::vector<std::string>()) = 0;
        
    };
}

#endif //_I_FEATURE_LAYER_H_783770E4_551C_4366_B2CB_B0EF945CB114_