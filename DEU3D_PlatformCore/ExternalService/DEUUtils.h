#ifndef _EXTERNAL_DEUUTILS_H_980457D6_DD78_43A4_9163_5D5C513BD9DD_
#define _EXTERNAL_DEUUTILS_H_980457D6_DD78_43A4_9163_5D5C513BD9DD_


#include <string>
#include <atlbase.h>
#include "DEUDefine.h"

namespace deues
{
    class DEUUtils
    {
    public:
        DEUUtils(void);
        ~DEUUtils(void);
		static std::string urlEncode(const std::string& str);
        static bool getWMTSMetaInfo(const void* chXML,DEUMetaData& metaData,int& nError);
        static bool getFeatureTypes(const void* chXML,std::vector<std::string>& strTypeVec);
        static bool getProperties(const void* chXML,const std::string& strFeatureType,std::vector<std::string>& strPropertyVec);
		//WMS
		static bool getWMSMetaData(const char* pStrXml, std::vector<DEULayerInfo>* parrLayerInfo);
    private:
        static bool readLayer    (IXMLDOMDocument* pXMLDoc,DEUMetaData& metaData);
        static bool readNodeText (IXMLDOMNode* pNode,std::string& strLayerID);
        static bool readStyleID  (IXMLDOMNode* pNode,std::string& strStyleID);
        static bool readBBox     (IXMLDOMNode* pNode,DEUMetaData& metaData);
        static bool readMatrixSet(IXMLDOMDocument* pXMLDoc,DEUMetaData& metaData);
        static bool readMatrix(IXMLDOMNode* pNode,DEUMatrixInfo& tInfo);
        static bool readFeatureTypes(IXMLDOMDocument* pXMLDoc,std::vector<std::string>& strTypeVec);
        static bool readFeatureType(IXMLDOMNode* pNode,std::string& strName);

        static bool readElements(IXMLDOMDocument* pXMLDoc,const std::string& strNameSpace,const std::string& strFeatureName,std::vector<std::string>& strPropertyVec);
        //WMS
		static bool readWMSLayer(IXMLDOMDocument* pXmlDoc, std::vector<DEULayerInfo>* parrLayerInfo);
		static bool isLayerNameExists(IXMLDOMNode* pLayer);
		//static bool readWMSStyleInfo(IXMLDOMDocument);
		static unsigned char ToHex(unsigned char x);
    };
}

#endif

