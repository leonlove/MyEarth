#ifndef _DEU_CONVERT_H_59CE4509_A844_41A6_BED2_4E4836632BE4_
#define _DEU_CONVERT_H_59CE4509_A844_41A6_BED2_4E4836632BE4_

#include <string>
#include <vector>
#include <atlbase.h>
#include "..\ModelBuilder\IModelBuilder.h"
//#include <ModelBuilder/IModelBuilder.h>
#include "WFSDefine.h"
#include "BuildGML.h"

namespace wfsb
{
    class ConvertGML : public OpenSP::Ref
    {
    public:
        ConvertGML(void);
        ~ConvertGML(void);
        void initialize(IModelBuilder* pModelBuilder,unsigned nDSCode,const std::string& strVersion,const std::string& strUrl);
        bool gml2Parameter(logical::ILayer* pLayer,const std::string strGML,const std::string& strID,const std::string& strGeometry);
        bool gml2ID(const std::string strGML,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec);
    protected:
        bool readFeatureMembers(logical::ILayer* pLayer,IXMLDOMDocument * pXMLDoc,const std::string& strID,const std::string& strGeometry);
        bool readFeatureMembers(IXMLDOMDocument * pXMLDoc,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec);
        bool readFeatureMember(logical::ILayer* pLayer,IXMLDOMNode* pNode,const std::string& strID,const std::string& strGeometry);
        bool readFeatureMember(IXMLDOMNode* pNode,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec);
        bool readFeature(logical::ILayer* pLayer,IXMLDOMNode* pNode,const std::string& strID,const std::string& strGeometry);
        bool readFeature(IXMLDOMNode* pNode,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,ID& idOut);
        bool readGeometry(logical::ILayer* pLayer,IXMLDOMNode* pNode,std::vector<double>& ptVector,GMType& gmType);
        bool readGeometry(IXMLDOMNode* pNode,GMType& gmType);
        bool readPoint(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readLineString(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readLineRing(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readPolygon(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readMultiSurface(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readMultiCurve(IXMLDOMNode* pNode,std::vector<double>& ptVector);
        bool readCoords(BSTR bstrValue,std::vector<double>& ptVector);

    protected:
        OpenSP::sp<BuildGML>      m_gmlBuilder;
        bool                      m_bReverse;
        std::string               m_strUrl;
        std::string               m_strLayerName;
        unsigned                  m_nDSCode;
        
    };

}
#endif //_DEU_CONVERT_H_59CE4509_A844_41A6_BED2_4E4836632BE4_
