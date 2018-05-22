#include "ConvertGML.h"
#include <comutil.h>
#include <common/md2.h>
#include <IDProvider/Definer.h>

namespace wfsb
{
ConvertGML::ConvertGML(void)
{
    m_gmlBuilder = NULL;
    m_bReverse = true;
    m_strUrl = m_strLayerName = "";
    m_nDSCode = 0;
}


ConvertGML::~ConvertGML(void)
{
    m_gmlBuilder = NULL;
}

void ConvertGML::initialize(IModelBuilder* pModelBuilder,unsigned nDSCode,const std::string& strVersion,const std::string& strUrl)
{
    if(strVersion == "1.0.0")
    {
        m_bReverse =false;
    }
    else
    {
        m_bReverse = true;
    }

    m_strUrl = strUrl;
    m_nDSCode = nDSCode;

    if(pModelBuilder != NULL)
    {
        m_gmlBuilder = new BuildGML();
        m_gmlBuilder->initialize(pModelBuilder,m_nDSCode,m_strUrl,m_bReverse);
    }
}

bool ConvertGML::gml2Parameter(logical::ILayer* pLayer,const std::string strGML,const std::string& strID,const std::string& strGeometry)
{
    int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, strGML.c_str(), -1, NULL, 0);

    LPWSTR pText = new WCHAR[nTextLen];
    memset(pText,'\0',nTextLen);
    ::MultiByteToWideChar(CP_UTF8, 0, strGML.c_str(), -1,pText, nTextLen);
    //1. initialize com
    HRESULT hr = CoInitialize(NULL); 
    if (FAILED(hr)) 
        return false;

    IXMLDOMDocument * pXMLDoc = NULL;
    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
        IID_IXMLDOMDocument, (void**)&pXMLDoc);
    if (FAILED(hr)) 
        return false;
    //2. load xml
    VARIANT_BOOL bIsSuccessful;
    hr = pXMLDoc->loadXML(pText,&bIsSuccessful);
    if(FAILED(hr))
    {
        pXMLDoc->Release();
        pXMLDoc = NULL;
        return false;
    }
    //3. read featuremember
    return readFeatureMembers(pLayer,pXMLDoc,strID,strGeometry);
}

bool ConvertGML::gml2ID(const std::string strGML,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec)
{
    int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, strGML.c_str(), -1, NULL, 0);

    LPWSTR pText = new WCHAR[nTextLen];
    memset(pText,'\0',nTextLen);
    ::MultiByteToWideChar(CP_UTF8, 0, strGML.c_str(), -1,pText, nTextLen);
    //1. initialize com
    HRESULT hr = CoInitialize(NULL); 
    if (FAILED(hr)) 
        return false;

    IXMLDOMDocument * pXMLDoc = NULL;
    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
        IID_IXMLDOMDocument, (void**)&pXMLDoc);
    if (FAILED(hr)) 
        return false;
    //2. load xml
    VARIANT_BOOL bIsSuccessful;
    hr = pXMLDoc->loadXML(pText,&bIsSuccessful);
    if(FAILED(hr))
    {
        pXMLDoc->Release();
        pXMLDoc = NULL;
        return false;
    }
    //3. read featuremember
    return readFeatureMembers(pXMLDoc,strLayer,strID,strGeometry,idVec);
}

bool ConvertGML::readFeatureMembers(logical::ILayer* pLayer,IXMLDOMDocument * pXMLDoc,const std::string& strID,const std::string& strGeometry)
{
    //1. get layer element
    IXMLDOMNodeList * pFeatureMemberList = NULL;
    CComBSTR  memberName("gml:featureMember");
    HRESULT hr = pXMLDoc->getElementsByTagName(memberName,&pFeatureMemberList);
    if(FAILED(hr))
    {
        return false;
    }
    long nListLength = 0;
    hr = pFeatureMemberList->get_length(&nListLength);
    if(FAILED(hr) || nListLength <= 0)
    {
        pFeatureMemberList->Release();
        pFeatureMemberList = NULL;
        return false;
    }

    for(long n = 0;n < nListLength;n++)
    {
        IXMLDOMNode *pFeatureMember = NULL;
        hr = pFeatureMemberList->get_item(n, &pFeatureMember);
        if(FAILED(hr) || pFeatureMember == NULL)
        {
            continue;
        }
        readFeatureMember(pLayer,pFeatureMember,strID,strGeometry);
        pFeatureMember->Release();
        pFeatureMember = NULL;
    }
    pFeatureMemberList->Release();
    pFeatureMemberList = NULL;
    return true;
}

bool ConvertGML::readFeatureMembers(IXMLDOMDocument * pXMLDoc,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec)
{
    //1. get layer element
    IXMLDOMNodeList * pFeatureMemberList = NULL;
    CComBSTR  memberName("gml:featureMember");
    HRESULT hr = pXMLDoc->getElementsByTagName(memberName,&pFeatureMemberList);
    if(FAILED(hr))
    {
        return false;
    }
    long nListLength = 0;
    hr = pFeatureMemberList->get_length(&nListLength);
    if(FAILED(hr) || nListLength <= 0)
    {
        pFeatureMemberList->Release();
        pFeatureMemberList = NULL;
        return false;
    }

    for(long n = 0;n < nListLength;n++)
    {
        IXMLDOMNode *pFeatureMember = NULL;
        hr = pFeatureMemberList->get_item(n, &pFeatureMember);
        if(FAILED(hr) || pFeatureMember == NULL)
        {
            continue;
        }
        readFeatureMember(pFeatureMember,strLayer,strID,strGeometry,idVec);
        pFeatureMember->Release();
        pFeatureMember = NULL;
    }
    pFeatureMemberList->Release();
    pFeatureMemberList = NULL;
    return true;
}

bool ConvertGML::readFeatureMember(IXMLDOMNode* pNode,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,std::vector<ID>& idVec)
{
    //1. get child node list
    IXMLDOMNodeList *pChildNodeList = NULL;
    HRESULT hr = pNode->get_childNodes(&pChildNodeList);
    if(FAILED(hr))
    {
        return false;
    }

    long nChildLength = 0;
    hr = pChildNodeList->get_length(&nChildLength);
    if(FAILED(hr) || nChildLength < 1)
    {
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return false;
    }
    pChildNodeList->reset();
    //2. read path
    for(long n = 0;n < nChildLength;n++)
    {
        IXMLDOMNode *pFeatureMember = NULL;
        hr = pChildNodeList->get_item(n, &pFeatureMember);
        if(FAILED(hr) || pFeatureMember == NULL)
        {
            continue;
        }
        ID idOut(0,0,0);
        if(readFeature(pFeatureMember,strLayer,strID,strGeometry,idOut))
		{
			idVec.push_back(idOut);
		}
        pFeatureMember->Release();
        pFeatureMember = NULL;
    }
    pChildNodeList->Release();
    pChildNodeList = NULL;
    return true;
}

bool ConvertGML::readFeatureMember(logical::ILayer* pLayer,IXMLDOMNode* pNode,const std::string& strID,const std::string& strGeometry)
{
    //1. get child node list
    IXMLDOMNodeList *pChildNodeList = NULL;
    HRESULT hr = pNode->get_childNodes(&pChildNodeList);
    if(FAILED(hr))
    {
        return false;
    }

    long nChildLength = 0;
    hr = pChildNodeList->get_length(&nChildLength);
    if(FAILED(hr) || nChildLength < 1)
    {
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return false;
    }
    pChildNodeList->reset();
    //2. read path
    for(long n = 0;n < nChildLength;n++)
    {
        IXMLDOMNode *pFeatureMember = NULL;
        hr = pChildNodeList->get_item(n, &pFeatureMember);
        if(FAILED(hr) || pFeatureMember == NULL)
        {
            continue;
        }
        readFeature(pLayer,pFeatureMember,strID,strGeometry);
        pFeatureMember->Release();
        pFeatureMember = NULL;
    }
    pChildNodeList->Release();
    pChildNodeList = NULL;
    return true;

}

bool ConvertGML::readFeature(IXMLDOMNode* pNode,const std::string& strLayer,const std::string& strID,const std::string& strGeometry,ID& idOut)
{
    //1. get child node list
    IXMLDOMNodeList *pChildNodeList = NULL;
    HRESULT hr = pNode->get_childNodes(&pChildNodeList);
    if(FAILED(hr))
    {
        return false;
    }

    long nChildLength = 0;
    hr = pChildNodeList->get_length(&nChildLength);
    if(FAILED(hr) || nChildLength < 1)
    {
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return false;
    }
    pChildNodeList->reset();

    std::string strUniqueID = "";
    GMType gmType;
    std::vector<double> ptVector;
    bool bID = false,bGeometry = false;

    //2. read path
    std::map<std::string,std::string> attrMap;
    for(long n = 0;n < nChildLength;n++)
    {
        IXMLDOMNode *pChildNode = NULL;
        pChildNodeList->get_item(n,&pChildNode);
        if(pChildNode == NULL)
        {
            continue;
        }
        BSTR bstrName;
        pChildNode->get_nodeName(&bstrName);
        if(wcscmp(bstrName,_bstr_t(strID.c_str())) == 0)//ID
        {
            BSTR bstrValue;
            pChildNode->get_text(&bstrValue);
            strUniqueID = _com_util::ConvertBSTRToString(bstrValue);
            ::SysFreeString(bstrValue);
            bID = true;
        }
        else if(wcscmp(bstrName,_bstr_t(strGeometry.c_str())) == 0)//Geometry数据
        {
            bGeometry = readGeometry(pChildNode,gmType);
        }
        else
        {
            continue;
        }
        ::SysFreeString(bstrName);
        pChildNode->Release();
        pChildNode = NULL;
    }

    if(bID && bGeometry)
    {
		idOut.ObjectID.m_nDataSetCode = m_nDSCode;
		std::string strIDText = strUniqueID + strLayer + m_strUrl;
		cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&idOut.ObjectID.m_UniqueID);
        switch(gmType)
        {
        case GM_POINT:
            idOut.ObjectID.m_nType = PARAM_POINT_ID;
            break;
        case GM_LINE:
            idOut.ObjectID.m_nType = PARAM_LINE_ID;
            break;
        case GM_FACE:
            idOut.ObjectID.m_nType = PARAM_FACE_ID;
            break;
        default:
            {
                pChildNodeList->Release();
                pChildNodeList = NULL;
                return false;
            }
        }
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return true;
    }

    pChildNodeList->Release();
    pChildNodeList = NULL;
    return false;
}

bool ConvertGML::readFeature(logical::ILayer* pLayer,IXMLDOMNode* pNode,const std::string& strID,const std::string& strGeometry)
{
    //1. get child node list
    IXMLDOMNodeList *pChildNodeList = NULL;
    HRESULT hr = pNode->get_childNodes(&pChildNodeList);
    if(FAILED(hr))
    {
        return false;
    }

    long nChildLength = 0;
    hr = pChildNodeList->get_length(&nChildLength);
    if(FAILED(hr) || nChildLength < 1)
    {
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return false;
    }
    pChildNodeList->reset();

    std::string strUniqueID = "";
    GMType gmType;
    std::vector<double> ptVector;
     bool bID = false,bGeometry = false;

    //2. read path
    std::map<std::string,std::string> attrMap;
    for(long n = 0;n < nChildLength;n++)
    {
        IXMLDOMNode *pChildNode = NULL;
        pChildNodeList->get_item(n,&pChildNode);
        if(pChildNode == NULL)
        {
            continue;
        }
        BSTR bstrName;
        pChildNode->get_nodeName(&bstrName);
        if(wcscmp(bstrName,_bstr_t(strID.c_str())) == 0)//ID
        {
            BSTR bstrValue;
            pChildNode->get_text(&bstrValue);
            strUniqueID = _com_util::ConvertBSTRToString(bstrValue);
            ::SysFreeString(bstrValue);
            bID = true;
        }
        else if(wcscmp(bstrName,_bstr_t(strGeometry.c_str())) == 0)//Geometry数据
        {
            bGeometry = readGeometry(pLayer,pChildNode,ptVector,gmType);
        }
        else//其它属性
        {
            BSTR bstrValue;
            pChildNode->get_text(&bstrValue);
            std::string strName = _com_util::ConvertBSTRToString(bstrName);
            std::string strValue = _com_util::ConvertBSTRToString(bstrValue);
            ::SysFreeString(bstrValue);

            int nFind = strName.find(':');
            if(nFind != -1)
            {
                strName = strName.substr(nFind+1,strName.length()-nFind-1);
            }

            attrMap[strName] = strValue;
        }
        ::SysFreeString(bstrName);
        pChildNode->Release();
        pChildNode = NULL;
    }

    if(bID && bGeometry)
    {
        m_gmlBuilder->convtParameter(pLayer,strUniqueID,ptVector,gmType,attrMap);
    }
    pChildNodeList->Release();
    pChildNodeList = NULL;
    return true;
}

bool ConvertGML::readGeometry(logical::ILayer* pLayer,IXMLDOMNode* pNode,std::vector<double>& ptVector,GMType& gmType)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);
    if(wcscmp(bstrName,_bstr_t("gml:Point")) == 0)
    {
        gmType = GM_POINT;
        return readPoint(pChild,ptVector);
    }
    else if(wcscmp(bstrName,_bstr_t("gml:LineString")) == 0)
    {
        gmType = GM_LINE;
        return readLineString(pChild,ptVector);
    }
    else if(wcscmp(bstrName,_bstr_t("gml:LinearRing")) == 0)
    {
        gmType = GM_FACE;
        return readLineRing(pChild,ptVector);
    }
    else if(wcscmp(bstrName,_bstr_t("gml:Polygon")) == 0)
    {
        gmType = GM_FACE;
        return readPolygon(pChild,ptVector);
    }
    else if(wcscmp(bstrName,_bstr_t("gml:MultiSurface")) == 0)
    {
        gmType = GM_FACE;
        return readMultiSurface(pChild,ptVector);
    }
    else if(wcscmp(bstrName,_bstr_t("gml:MultiCurve")) == 0)
    {
        gmType = GM_LINE;
        return readMultiCurve(pChild,ptVector);
    }
    else
    {
        return false;
    }
}

bool ConvertGML::readGeometry(IXMLDOMNode* pNode,GMType& gmType)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);
    if(wcscmp(bstrName,_bstr_t("gml:Point")) == 0)
    {
        gmType = GM_POINT;
        return true;
    }
    else if(wcscmp(bstrName,_bstr_t("gml:LineString")) == 0)
    {
        gmType = GM_LINE;
        return true;
    }
    else if(wcscmp(bstrName,_bstr_t("gml:LinearRing")) == 0)
    {
        gmType = GM_FACE;
        return true;
    }
    else if(wcscmp(bstrName,_bstr_t("gml:Polygon")) == 0)
    {
        gmType = GM_FACE;
        return true;
    }
    else if(wcscmp(bstrName,_bstr_t("gml:MultiSurface")) == 0)
    {
        gmType = GM_FACE;
        return true;
    }
    else if(wcscmp(bstrName,_bstr_t("gml:MultiCurve")) == 0)
    {
        gmType = GM_LINE;
        return true;
    }
    else
    {
        return false;
    }
}

//<gml:Point>
//    <gml:pos>39.923614999999998 116.38094</gml:pos> 
//    </gml:Point>
bool ConvertGML::readPoint(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);
    if(m_bReverse)
    {
        if(wcscmp(bstrName,_bstr_t("gml:pos")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            return bRead;        
        }
    }
    else
    {
        if(wcscmp(bstrName,_bstr_t("gml:coordinates")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            return bRead;        
        }
    }

    ::SysFreeString(bstrName);
    return false;
}

bool ConvertGML::readCoords(BSTR bstrValue,std::vector<double>& ptVector)
{
    char* chValue = _com_util::ConvertBSTRToString(bstrValue);
    char* chRes = strtok(chValue," ");
    while(chRes)
    {
        double dValue = atof(chRes);
        ptVector.push_back(dValue);
        chRes = strtok(NULL," ");
    }
    delete[] chValue;
    return true;
}

bool ConvertGML::readLineString(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);

    if(m_bReverse)//1.1.0
    {
        if(wcscmp(bstrName,_bstr_t("gml:posList")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            ::SysFreeString(bstrName);
            return bRead;
        }
    }
    else//1.0.0
    {
        if(wcscmp(bstrName,_bstr_t("gml:coordinates")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            ::SysFreeString(bstrName);
            return bRead;
        }
    }
    
    ::SysFreeString(bstrName);
    return false;
}

bool ConvertGML::readLineRing(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);
    if(m_bReverse)
    {
        if(wcscmp(bstrName,_bstr_t("gml:posList")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            return bRead;
        }
    }
    else
    {
        if(wcscmp(bstrName,_bstr_t("gml:coordinates")) == 0)
        {
            BSTR bstrValue;
            pChild->get_text(&bstrValue);
            bool bRead = readCoords(bstrValue,ptVector);
            ::SysFreeString(bstrValue);
            return bRead;
        }
    }
    
    ::SysFreeString(bstrName);
    return false;
}

bool ConvertGML::readPolygon(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);

    while(wcscmp(bstrName,_bstr_t("gml:LinearRing")) != 0)
    {
        pNode = pChild;
        pNode->get_firstChild(&pChild);
        if(pChild == NULL)
        {
            return false;
        }
        pChild->get_nodeName(&bstrName);
    }
    return readLineRing(pChild,ptVector);
}

bool ConvertGML::readMultiSurface(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);

    while(wcscmp(bstrName,_bstr_t("gml:LinearRing")) != 0)
    {
        pNode = pChild;
        pNode->get_firstChild(&pChild);
        if(pChild == NULL)
        {
            return false;
        }
        pChild->get_nodeName(&bstrName);
    }
    return readLineRing(pChild,ptVector);
}

bool ConvertGML::readMultiCurve(IXMLDOMNode* pNode,std::vector<double>& ptVector)
{
    IXMLDOMNode* pChild = NULL;
    pNode->get_firstChild(&pChild);
    if(pChild == NULL)
    {
        return false;
    }
    BSTR bstrName;
    pChild->get_nodeName(&bstrName);

    while(wcscmp(bstrName,_bstr_t("gml:LineString")) != 0)
    {
        pNode = pChild;
        pNode->get_firstChild(&pChild);
        if(pChild == NULL)
        {
            return false;
        }
        pChild->get_nodeName(&bstrName);
    }
    return readLineString(pChild,ptVector);
}

}