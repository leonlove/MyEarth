#include "DEUUtils.h"
#include <comutil.h>
#include <common/Common.h>

namespace deues
{
    DEUUtils::DEUUtils(void)
    {
    }


    DEUUtils::~DEUUtils(void)
    {
    }

	unsigned char DEUUtils::ToHex(unsigned char x)   
	{   
		return  x > 9 ? x + 55 : x + 48;   
	} 

	std::string DEUUtils::urlEncode(const std::string& str)
	{
		std::wstring wstr = cmm::ANSIToUnicode(str);
		std::string struft = cmm::UnicodeToUTF8(wstr);
		std::string strTemp = "";  
		size_t length = struft.length(); 
		unsigned char c;
		for (size_t i = 0; i < length-1; i++)  
		{  
			c = struft[i];
			if (isalnum(c) || c == '-' || c == '.' || c == '~' || c == '_' || c == '%' || c == '*' || c == '!'
				|| c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ' ' || c == '/'
				|| c == '?' || c == '&' || c == '=')
			{
				strTemp += struft[i];  
			}
			else  
			{  
				strTemp += '%';  
				strTemp += ToHex((unsigned char)struft[i] >> 4);  
				strTemp += ToHex((unsigned char)struft[i] % 16);  
			}  
		}  
		return strTemp;  
	}

    bool DEUUtils::getProperties(const void* chXML,const std::string& strFeatureType,std::vector<std::string>& strPropertyVec)
    {
        strPropertyVec.clear();

        std::string strTemp = strFeatureType;
        std::string strNameSpace = strtok((char*)strTemp.c_str(),":");
        std::string strFeatureName = strtok(NULL,":");


        int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1, NULL, 0);

        LPWSTR pText = new WCHAR[nTextLen];
        memset(pText,'\0',nTextLen);
        ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1,pText, nTextLen);
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
        // CComBSTR bstrXML(pText);
        VARIANT_BOOL bIsSuccessful;
        hr = pXMLDoc->loadXML(pText,&bIsSuccessful);
        if(FAILED(hr))
        {
            pXMLDoc->Release();
            pXMLDoc = NULL;
            return false;
        }
        //3. read elements
        return readElements(pXMLDoc,strNameSpace,strFeatureName,strPropertyVec);
    }

    bool DEUUtils::readElements(IXMLDOMDocument* pXMLDoc,const std::string& strNameSpace,const std::string& strFeatureName,std::vector<std::string>& strPropertyVec)
    {
        //1. get layer element
        IXMLDOMNodeList * pElementList = NULL;
        CComBSTR  listName("xs:element");
        HRESULT hr = pXMLDoc->getElementsByTagName(listName,&pElementList);
        if(FAILED(hr))
        {
            return false;
        }
        long nTypeLength = 0;
        hr = pElementList->get_length(&nTypeLength);
        if(FAILED(hr) || nTypeLength <= 0)
        {
            pElementList->Release();
            pElementList = NULL;
            return false;
        }

        CComBSTR  attrName("name");
        std::string strName = "";
        for(unsigned n = 0;n < nTypeLength;n++)
        {
            IXMLDOMNode *pElementNode = NULL;
            hr = pElementList->get_item(n, &pElementNode);

            if(FAILED(hr) || pElementNode == NULL)
            {
                continue;
            }

            IXMLDOMNamedNodeMap* pAttrMap = NULL;
            hr = pElementNode->get_attributes(&pAttrMap);
            if(FAILED(hr) || pElementNode == NULL)
            {
                pElementNode->Release();
                pElementNode = NULL;
                continue;
            }

            IXMLDOMNode* pNameNode = NULL;
            pAttrMap->getNamedItem(attrName,&pNameNode);
            if(FAILED(hr) || pNameNode == NULL)
            {
                pElementNode->Release();
                pElementNode = NULL;
                continue;
            }
            
            BSTR bstrValue;
            pNameNode->get_text(&bstrValue);
            strName = _com_util::ConvertBSTRToString(bstrValue);
            if(strName != strFeatureName)
            {
                strName = strNameSpace + ":" + strName;
                strPropertyVec.push_back(strName);
            }
            ::SysFreeString(bstrValue);

            pNameNode->Release();
            pNameNode = NULL;

            pAttrMap->Release();
            pAttrMap = NULL;

            pElementNode->Release();
            pElementNode = NULL;
        }

        pElementList->Release();
        pElementList = NULL;

        return true;
        
    }

    bool DEUUtils::getFeatureTypes(const void* chXML,std::vector<std::string>& strTypeVec)
    {
        strTypeVec.clear();
        int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1, NULL, 0);

        LPWSTR pText = new WCHAR[nTextLen];
        memset(pText,'\0',nTextLen);
        ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1,pText, nTextLen);
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
        // CComBSTR bstrXML(pText);
        VARIANT_BOOL bIsSuccessful;
        hr = pXMLDoc->loadXML(pText,&bIsSuccessful);
        if(FAILED(hr))
        {
            pXMLDoc->Release();
            pXMLDoc = NULL;
            return false;
        }
        //3. read feature types
        return readFeatureTypes(pXMLDoc,strTypeVec);
    }

    bool DEUUtils::readFeatureTypes(IXMLDOMDocument* pXMLDoc,std::vector<std::string>& strTypeVec)
    {
        //1. get layer element
        IXMLDOMNodeList * pFeatureTypeList = NULL;
        CComBSTR  listName("wfs:FeatureTypeList");
        HRESULT hr = pXMLDoc->getElementsByTagName(listName,&pFeatureTypeList);
        if(FAILED(hr))
        {
            return false;
        }
        long nTypeLength = 0;
        hr = pFeatureTypeList->get_length(&nTypeLength);
        if(FAILED(hr) || nTypeLength <= 0)
        {
            pFeatureTypeList->Release();
            pFeatureTypeList = NULL;
            return false;
        }
        IXMLDOMNode *pFeatureTypesNode = NULL;
        hr = pFeatureTypeList->get_item(0, &pFeatureTypesNode);
        pFeatureTypeList->Release();
        pFeatureTypeList = NULL;
        if(FAILED(hr) || pFeatureTypesNode == NULL)
        {
            return false;
        }
        //2. get child node list
        IXMLDOMNodeList *pChildNodeList = NULL;
        hr = pFeatureTypesNode->get_childNodes(&pChildNodeList);
        if(FAILED(hr))
        {
            pFeatureTypesNode->Release();
            pFeatureTypesNode = NULL;
            return false;
        }
        pFeatureTypesNode->Release();
        pFeatureTypesNode = NULL;

        long nChildLength = 0;
        hr = pChildNodeList->get_length(&nChildLength);
        if(FAILED(hr) || nChildLength < 1)
        {
            pChildNodeList->Release();
            pChildNodeList = NULL;
            return false;
        }
        pChildNodeList->reset();

        IXMLDOMNode* pChildNode = NULL;
        for(int ii = 0; ii < nChildLength; ii++)
        {
            pChildNodeList->get_item(ii, &pChildNode);
            BSTR bstrName;
            pChildNode->get_nodeName(&bstrName);
            if(wcscmp(bstrName,_bstr_t("wfs:FeatureType")) == 0)
            {   
                std::string strName = "";
                if(readFeatureType(pChildNode,strName))
                {
                    strTypeVec.push_back(strName);
                }
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return true;
    }
    bool DEUUtils::readFeatureType(IXMLDOMNode* pNode,std::string& strName)
    {
        IXMLDOMNodeList* pChildList = NULL;
        HRESULT hr = pNode->get_childNodes(&pChildList);
        if(FAILED(hr))
        {
            return false;
        }
        long nLength = 0;
        pChildList->get_length(&nLength);
        if(FAILED(hr) || nLength <= 0)
        {
            return false;
        }

        IXMLDOMNode* pChildNode = NULL;
        BSTR bstrName = NULL;
        int nCmpRes = 0;
        for(int n = 0;n < nLength;n++)
        {
            pChildList->get_item(n,&pChildNode);
            if(pChildNode == NULL)
                continue;
            pChildNode->get_nodeName(&bstrName);
            if(wcscmp(bstrName,_bstr_t("wfs:Name")) == 0)
            {
                BSTR bstrItemText;
                hr = pChildNode->get_text(&bstrItemText);
                if(SUCCEEDED(hr))
                {
                    strName = _com_util::ConvertBSTRToString(bstrItemText);
                    ::SysFreeString(bstrItemText);
                    ::SysFreeString(bstrName);
                    bstrItemText = NULL;
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildList->Release();
                    pChildList = NULL;
                    return true;
                }
                else
                {
                    ::SysFreeString(bstrItemText);
                    ::SysFreeString(bstrName);
                    bstrItemText = NULL;
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildList->Release();
                    pChildList = NULL;
                    return false;
                }
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pChildList->Release();
        pChildList = NULL;
        return false;
    }
    //http://www.sdmap.gov.cn/tileservice/SDRasterPubMap?service=WMTS&request=GetTile&version=1.0.0&
    //layer=0&style=default&format=image/jpeg&TileMatrixSet=tianditu2013&TileMatrix=1&TileRow=1&TileCol=3

    bool DEUUtils::getWMTSMetaInfo(const void* chXML,DEUMetaData& metaData,int& nError)
    {
        int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1, NULL, 0);

        LPWSTR pText = new WCHAR[nTextLen];
        memset(pText,'\0',nTextLen);
        ::MultiByteToWideChar(CP_UTF8, 0, (char*)chXML, -1,pText, nTextLen);
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
       // CComBSTR bstrXML(pText);
        VARIANT_BOOL bIsSuccessful;
        hr = pXMLDoc->loadXML(pText,&bIsSuccessful);
        if(FAILED(hr))
        {
            pXMLDoc->Release();
            pXMLDoc = NULL;
            return false;
        }
        //3. read layer
        if(!readLayer(pXMLDoc,metaData))
        {
            pXMLDoc->Release();
            pXMLDoc = NULL;
            return false;
        }
        //4. read matrixset and matrix
        if(!readMatrixSet(pXMLDoc,metaData))
        {
            pXMLDoc->Release();
            pXMLDoc = NULL;
            return false;
        }
        pXMLDoc->Release();
        pXMLDoc = NULL;
        return true;
    }

    bool DEUUtils::readMatrixSet(IXMLDOMDocument* pXMLDoc,DEUMetaData& metaData)
    {
        //1. get layer element
        IXMLDOMNodeList * pLayerNodeList = NULL;
        CComBSTR  layerName("TileMatrixSet");
        HRESULT hr = pXMLDoc->getElementsByTagName(layerName,&pLayerNodeList);
        if(FAILED(hr))
        {
            return false;
        }
        long nLayerLength = 0;
        hr = pLayerNodeList->get_length(&nLayerLength);
        if(FAILED(hr) || nLayerLength <= 0)
        {
            pLayerNodeList->Release();
            pLayerNodeList = NULL;
            return false;
        }

        IXMLDOMNode *pLayerNode = NULL;
        hr = pLayerNodeList->get_item(nLayerLength-1, &pLayerNode);
        pLayerNodeList->Release();
        pLayerNodeList = NULL;
        if(FAILED(hr) || pLayerNode == NULL)
        {
            return false;
        }
        //2. get child node list
        IXMLDOMNodeList *pChildNodeList = NULL;
        hr = pLayerNode->get_childNodes(&pChildNodeList);
        if(FAILED(hr))
        {
            pLayerNode->Release();
            pLayerNode = NULL;
            return false;
        }
        pLayerNode->Release();
        pLayerNode = NULL;

        long nChildLength = 0;
        hr = pChildNodeList->get_length(&nChildLength);
        if(FAILED(hr) || nChildLength < 1)
        {
            pChildNodeList->Release();
            pChildNodeList = NULL;
            return false;
        }
        pChildNodeList->reset();

        IXMLDOMNode* pChildNode = NULL;
        for(int ii = 0; ii < nChildLength; ii++)
        {
            pChildNodeList->get_item(ii, &pChildNode);
            BSTR bstrName;
            pChildNode->get_nodeName(&bstrName);
            if(wcscmp(bstrName,_bstr_t("ows:Identifier")) == 0)
            {
                if(!readNodeText(pChildNode,metaData.m_strMatrixSet))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildNodeList->Release();
                    pChildNodeList = NULL;
                    return false;
                }
            }
            else if(wcscmp(bstrName,_bstr_t("TileMatrix")) == 0)
            {
                DEUMatrixInfo tInfo;
                if(readMatrix(pChildNode,tInfo))
                {
                    metaData.m_matrixMap[tInfo.m_dScale] = tInfo;
                    metaData.m_matrixPtrMap[tInfo.m_strMatrix] = &tInfo;
                }
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return true;
    }

    bool DEUUtils::readMatrix(IXMLDOMNode* pNode,DEUMatrixInfo& tInfo)
    {
        IXMLDOMNodeList* pIChildMatrixList = NULL;
        HRESULT hr = pNode->get_childNodes(&pIChildMatrixList);
        if(FAILED(hr))
        {
            return false;
        }

        long nLength = 0;
        pIChildMatrixList->get_length(&nLength);
        if(FAILED(hr) || nLength <= 0)
        {
            return false;
        }

        IXMLDOMNode* pChildNode = NULL;
        BSTR bstrName = NULL;
        int nCmpRes = 0;
        for(int n = 0;n < nLength;n++)
        {
            pIChildMatrixList->get_item(n,&pChildNode);
            if(pChildNode == NULL)
                return false;
            pChildNode->get_nodeName(&bstrName);
            if(wcscmp(bstrName,_bstr_t("ows:Identifier")) == 0)
            {
                if(!readNodeText(pChildNode,tInfo.m_strMatrix))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                
            }
            else if(wcscmp(bstrName,_bstr_t("ScaleDenominator")) == 0)
            {
                std::string strDenominator = "";
                if(!readNodeText(pChildNode,strDenominator))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                double dScale = atof(strDenominator.c_str());
                tInfo.m_dScale = dScale*0.28*0.001/111194.872221777;
            }
            else if(wcscmp(bstrName,_bstr_t("TopLeftCorner")) == 0)
            {
                std::string strText = "";
                if(!readNodeText(pChildNode,strText))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                double dX = 0.0,dY = 0.0;
                sscanf_s(strText.c_str(),"%lf %lf",&dX,&dY);
                if(abs(dX) > abs(dY))
                {
                    tInfo.m_dTopLeftX = dX;
                    tInfo.m_dTopLeftY = dY;
                }
                else
                {
                    tInfo.m_dTopLeftX = dY;
                    tInfo.m_dTopLeftY = dX;
                }
            }
            else if(wcscmp(bstrName,_bstr_t("TileWidth")) == 0)
            {
                std::string strText = "";
                if(!readNodeText(pChildNode,strText))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                tInfo.m_nCol = atoi(strText.c_str());
            }
            else if(wcscmp(bstrName,_bstr_t("TileHeight")) == 0)
            {
                std::string strText = "";
                if(!readNodeText(pChildNode,strText))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                tInfo.m_nRow = atoi(strText.c_str());
            }
            else if(wcscmp(bstrName,_bstr_t("MatrixWidth")) == 0)
            {
                std::string strText = "";
                if(!readNodeText(pChildNode,strText))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                tInfo.m_nWidth = atoi(strText.c_str());
            }
            else if(wcscmp(bstrName,_bstr_t("MatrixHeight")) == 0)
            {
                std::string strText = "";
                if(!readNodeText(pChildNode,strText))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pIChildMatrixList->Release();
                    pIChildMatrixList = NULL;
                    return false;
                }
                tInfo.m_nHeight = atoi(strText.c_str());
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pIChildMatrixList->Release();
        pIChildMatrixList = NULL;
        return true;
    }

    bool DEUUtils::readLayer(IXMLDOMDocument* pXMLDoc,DEUMetaData& metaData)
    {
        //1. get layer element
        IXMLDOMNodeList * pLayerNodeList = NULL;
        CComBSTR  layerName("Layer");
        HRESULT hr = pXMLDoc->getElementsByTagName(layerName,&pLayerNodeList);
        if(FAILED(hr))
        {
            return false;
        }
        long nLayerLength = 0;
        hr = pLayerNodeList->get_length(&nLayerLength);
        if(FAILED(hr) || nLayerLength <= 0)
        {
            pLayerNodeList->Release();
            pLayerNodeList = NULL;
            return false;
        }

        IXMLDOMNode *pLayerNode = NULL;
        hr = pLayerNodeList->get_item(0, &pLayerNode);
        pLayerNodeList->Release();
        pLayerNodeList = NULL;
        if(FAILED(hr) || pLayerNode == NULL)
        {
            return false;
        }
        //2. get child node list
        IXMLDOMNodeList *pChildNodeList = NULL;
        hr = pLayerNode->get_childNodes(&pChildNodeList);
        if(FAILED(hr))
        {
            pLayerNode->Release();
            pLayerNode = NULL;
            return false;
        }
        pLayerNode->Release();
        pLayerNode = NULL;

        long nChildLength = 0;
        hr = pChildNodeList->get_length(&nChildLength);
        if(FAILED(hr) || nChildLength < 1)
        {
            pChildNodeList->Release();
            pChildNodeList = NULL;
            return false;
        }
        pChildNodeList->reset();
        //3. get child node
        bool bID = false;
        bool bStyle = false;
        bool bFormat = false;
        bool bBBox = false;
        IXMLDOMNode* pChildNode = NULL;
        for(int ii = 0; ii < nChildLength; ii++)
        {
            pChildNodeList->get_item(ii, &pChildNode);
            BSTR bstrName;
            pChildNode->get_nodeName(&bstrName);
            _bstr_t chName = bstrName;
            if(wcscmp(chName,_bstr_t("ows:Identifier")) == 0)
            {
                if(!readNodeText(pChildNode,metaData.m_strLayer))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildNodeList->Release();
                    pChildNodeList = NULL;
                    return false;
                }
                bID = true;
            }
            else if(wcscmp(chName,_bstr_t("Style")) == 0)
            {
                if(!readStyleID(pChildNode,metaData.m_strStyle))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildNodeList->Release();
                    pChildNodeList = NULL;
                    return false;
                }
                bStyle = true;
            }
            else if(wcscmp(chName,_bstr_t("Format")) == 0)
            {
                if(!readNodeText(pChildNode,metaData.m_strFormat))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildNodeList->Release();
                    pChildNodeList = NULL;
                    return false;
                }
                bFormat = true;
            }
            else if((wcscmp(chName,_bstr_t("ows:WGS84BoundingBox")) == 0
                  || wcscmp(chName,_bstr_t("ows:BoundingBox")) == 0) && !bBBox)
            {
                if(!readBBox(pChildNode,metaData))
                {
                    ::SysFreeString(bstrName);
                    bstrName = NULL;
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildNodeList->Release();
                    pChildNodeList = NULL;
                    return false;
                }
                bBBox = true;
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return (bID && bStyle && bFormat && bBBox);
    }

    bool DEUUtils::readBBox(IXMLDOMNode* pNode,DEUMetaData& metaData)
    {
        IXMLDOMNodeList *pChildNodeList = NULL;
        HRESULT hr = pNode->get_childNodes(&pChildNodeList);
        if(FAILED(hr))
        {
            return false;
        }

        long nChildLength = 0;
        hr = pChildNodeList->get_length(&nChildLength);
        if(FAILED(hr) || nChildLength <= 0)
        {
            pChildNodeList->Release();
            pChildNodeList = NULL;
            return false;
        }
        pChildNodeList->reset();
        //5. get child node
        IXMLDOMNode* pChildNode = NULL;
        BSTR bstrItemText;
        BSTR bstrName;
        double dX = 0.0,dY = 0.0;
        for(int ii = 0; ii < nChildLength; ii++)
        {
            pChildNodeList->get_item(ii, &pChildNode);
            IXMLDOMNamedNodeMap* boxMap = NULL;
            pChildNode->get_nodeName(&bstrName);
            pChildNode->get_text(&bstrItemText);
            pChildNode->Release();
            pChildNode = NULL;
            //read data range
            if(wcscmp(bstrName,_bstr_t("ows:LowerCorner")) == 0)
            {
                swscanf_s(bstrItemText,_bstr_t("%lf %lf"),&dX,&dY);
                if(abs(dX) > abs(dY))
                {
                    metaData.m_dMinX = dX;
                    metaData.m_dMinY = dY;
                }
                else
                {
                    metaData.m_dMinX = dY;
                    metaData.m_dMinY = dX;
                }
            }
            else if(wcscmp(bstrName,_bstr_t("ows:UpperCorner")) == 0)
            {
                swscanf_s(bstrItemText,_bstr_t("%lf %lf"),&dX,&dY);
                if(abs(dX) > abs(dY))
                {
                    metaData.m_dMaxX = dX;
                    metaData.m_dMaxY = dY;
                }
                else
                {
                    metaData.m_dMaxX = dY;
                    metaData.m_dMaxY = dX;
                }
            }
        }
        ::SysFreeString(bstrItemText);
        ::SysFreeString(bstrName);
        pChildNodeList->Release();
        pChildNodeList = NULL;
        return true;
    }

    bool DEUUtils::readStyleID(IXMLDOMNode* pNode,std::string& strStyleID)
    {
        IXMLDOMNodeList* pChildList = NULL;
        HRESULT hr = pNode->get_childNodes(&pChildList);
        if(FAILED(hr))
        {
            return false;
        }
        long nLength = 0;
        hr = pChildList->get_length(&nLength);
        if(FAILED(hr) || nLength < 1)
        {
            pChildList->Release();
            pChildList = NULL;
            return false;
        }
        pChildList->reset();

        IXMLDOMNode* pChildNode = NULL;
        BSTR bstrName = NULL;
        for(int ii = 0; ii < nLength; ii++)
        {
            pChildList->get_item(ii, &pChildNode);
            pChildNode->get_nodeName(&bstrName);
            _bstr_t chName = bstrName;
            if(wcscmp(chName,_bstr_t("ows:Identifier")) == 0)
            {
                if(readNodeText(pChildNode,strStyleID))
                {
                    ::SysFreeString(bstrName);
                    pChildNode->Release();
                    pChildNode = NULL;
                    pChildList->Release();
                    pChildList = NULL;
                    return true;
                }
            }
            ::SysFreeString(bstrName);
            bstrName = NULL;
            pChildNode->Release();
            pChildNode = NULL;
        }
        pChildList->Release();
        pChildList = NULL;
        return false;
    }

    bool DEUUtils::readNodeText(IXMLDOMNode* pNode,std::string& strID)
    {
        BSTR bstrItemText;
        HRESULT hr = pNode->get_text(&bstrItemText);
        if(FAILED(hr))
        {
            return false;
        }
        strID = _com_util::ConvertBSTRToString(bstrItemText);
        ::SysFreeString(bstrItemText);
        return true;
    }
	    
	bool DEUUtils::getWMSMetaData(const char* pStrXml, std::vector<DEULayerInfo>* parrLayerInfo)
	{
		int nTextLen = ::MultiByteToWideChar(CP_UTF8, 0, (char*)pStrXml, -1, NULL, 0);

		LPWSTR pText = new WCHAR[nTextLen];
		memset(pText,'\0',nTextLen);
		::MultiByteToWideChar(CP_UTF8, 0, (char*)pStrXml, -1, pText, nTextLen);

		HRESULT hr = CoInitialize(NULL);
		if(FAILED(hr))
			return false;
		IXMLDOMDocument *pXmlDoc = NULL;
		hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pXmlDoc);
		if(FAILED(hr))
			return false;
		VARIANT_BOOL bSucceeded;
		hr = pXmlDoc->loadXML(pText, &bSucceeded);
		if(FAILED(hr))
		{
			pXmlDoc->Release();
			pXmlDoc = NULL;
			return false;
		}
		readWMSLayer(pXmlDoc,parrLayerInfo);
		pXmlDoc->Release();
		pXmlDoc = NULL;
		return true;
	}

	bool DEUUtils::isLayerNameExists(IXMLDOMNode* pLayer)
	{
		HRESULT hr;
		IXMLDOMNode *pNode = NULL;
		IXMLDOMNodeList *pChildNode = NULL;
		hr = pLayer->get_childNodes(&pChildNode);
		if(FAILED(hr))
		{
			return false;
		}
		long nNodeNum =0;
		hr = pChildNode->get_length(&nNodeNum);
		if(FAILED(hr) || nNodeNum < 1)
		{
			return false;
		}
		BSTR bstrNodeName;
		for(long i=0; i<nNodeNum; i++)
		{
			pChildNode->get_item(i,&pNode);
			pNode->get_nodeName(&bstrNodeName);
			_bstr_t bNodeName = bstrNodeName;
			if(wcscmp(bNodeName,_bstr_t("Name")) == 0)
			{
				SysFreeString(bstrNodeName);
				bstrNodeName = NULL;
				return true;
			}
			SysFreeString(bstrNodeName);
			bstrNodeName = NULL;
		}
		return false;
	}

	bool DEUUtils::readWMSLayer(IXMLDOMDocument* pXmlDoc, std::vector<DEULayerInfo>* parrLayerInfo)
	{
		IXMLDOMNodeList * pLayerNodeList = NULL;
		CComBSTR  layerName("Layer");
		HRESULT hr = pXmlDoc->getElementsByTagName(layerName,&pLayerNodeList);
		if(FAILED(hr))
			return false;
		long nLayersNum = 0;
		hr = pLayerNodeList->get_length(&nLayersNum);
		if(FAILED(hr) || nLayersNum <= 0)
		{
			pLayerNodeList->Release();
			pLayerNodeList = NULL;
			return false;
		}
		long nChildNodeNum = 0;
		IXMLDOMNode *pLayer = NULL;
		IXMLDOMNodeList *pLayerChildNodeList = NULL;
		for(long i=0; i<nLayersNum; i++)
		{
			DEULayerInfo newLayer;
			hr = pLayerNodeList->get_item(i, &pLayer);
			if(SUCCEEDED(hr) && pLayer != NULL)
			{
				if(!isLayerNameExists(pLayer)) continue;
				hr = pLayer->get_childNodes(&pLayerChildNodeList);
				if(FAILED(hr))
				{
					pLayer->Release();
					pLayer = NULL;
					continue;
				}
				else
				{
					hr = pLayerChildNodeList->get_length(&nChildNodeNum);
					if (FAILED(hr) || nChildNodeNum < 1)
					{
						pLayerChildNodeList->reset();
						pLayerChildNodeList->Release();
						pLayerChildNodeList = NULL;
						continue;
					}
					else
					{
						IXMLDOMNode *pNode = NULL;
						BSTR bstrNodeName,bstrNodeText;
						for(long j=0; j<nChildNodeNum; j++)
						{
							pLayerChildNodeList->get_item(j,&pNode);
							pNode->get_nodeName(&bstrNodeName);
							_bstr_t bNodeName = bstrNodeName;
							if(wcscmp(bNodeName, _bstr_t("Name")) == 0)
							{
								pNode->get_text(&bstrNodeText);
								newLayer.m_strLayerName = _com_util::ConvertBSTRToString(bstrNodeText);
								SysFreeString(bstrNodeText);
							}
							if(wcscmp(bNodeName, _bstr_t("EX_GeographicBoundingBox")) == 0)
							{
								long nCoordNum = 0;
								IXMLDOMNodeList *pCoordList = NULL;
								hr = pNode->get_childNodes(&pCoordList);
								if (FAILED(hr))
								{
									break;
								}
								hr = pCoordList->get_length(&nCoordNum);
								if(FAILED(hr) || nCoordNum < 1)
								{
									break;
								}

								BSTR bstrCoordVal;
								IXMLDOMNode *pCoordNode = NULL;
								for(long k=0; k<nCoordNum; k++)
								{
									pCoordList->get_item(k, &pCoordNode);
									pCoordNode->get_nodeName(&bstrNodeName);
									_bstr_t bCNodeName = bstrNodeName;
									if(wcscmp(bCNodeName, _bstr_t("westBoundLongitude")) == 0)
									{										
										pCoordNode->get_text(&bstrCoordVal);
										std::string strVal = _com_util::ConvertBSTRToString(bstrCoordVal);
										newLayer.m_dMinX = atof(strVal.c_str());
										SysFreeString(bstrCoordVal);
									}
									if(wcscmp(bCNodeName, _bstr_t("eastBoundLongitude")) == 0)
									{
										pCoordNode->get_text(&bstrCoordVal);
										std::string strVal = _com_util::ConvertBSTRToString(bstrCoordVal);
										newLayer.m_dMaxX = atof(strVal.c_str());
										SysFreeString(bstrCoordVal);
									}
									if(wcscmp(bCNodeName, _bstr_t("southBoundLatitude")) == 0)
									{
										pCoordNode->get_text(&bstrCoordVal);
										std::string strVal = _com_util::ConvertBSTRToString(bstrCoordVal);
										newLayer.m_dMinY = atof(strVal.c_str());
										SysFreeString(bstrCoordVal);
									}
									if(wcscmp(bCNodeName, _bstr_t("northBoundLatitude")) == 0)
									{
										pCoordNode->get_text(&bstrCoordVal);
										std::string strVal = _com_util::ConvertBSTRToString(bstrCoordVal);
										newLayer.m_dMaxY = atof(strVal.c_str());
										SysFreeString(bstrCoordVal);
									}
								}
								SysFreeString(bstrNodeName);
								pCoordNode->Release();
								pCoordNode = NULL;
								pCoordList->reset();
								pCoordList->Release();
								pCoordList = NULL;
							}
							if(wcscmp(bNodeName, _bstr_t("CRS")) == 0 || wcscmp(bNodeName, _bstr_t("SRS")) == 0)
							{
								pNode->get_text(&bstrNodeText);
								newLayer.m_vecCRS.push_back(_com_util::ConvertBSTRToString(bstrNodeText));
								SysFreeString(bstrNodeText);
							}
							if(wcscmp(bNodeName, _bstr_t("Style")) == 0)
							{
								DEUStyleInfo newStyleInfo;
								IXMLDOMNodeList *pArrStleyNodeList = NULL;
								hr = pNode->get_childNodes(&pArrStleyNodeList);
								if(FAILED(hr))
								{
									continue;
								}
								long nStyleNodeNum=0;
								hr = pArrStleyNodeList->get_length(&nStyleNodeNum);
								if(FAILED(hr) || nStyleNodeNum < 1)
								{
									pArrStleyNodeList->Release();
									pArrStleyNodeList = NULL;
									continue;
								}
								IXMLDOMNode *pStyleNode = NULL;
								for(long m=0; m<nStyleNodeNum; m++)
								{
									pArrStleyNodeList->get_item(m, &pStyleNode);
									pStyleNode->get_nodeName(&bstrNodeName);
									_bstr_t bNodeName = bstrNodeName;
									if(wcscmp(bNodeName, _bstr_t("Name")) == 0)
									{
										BSTR bstrSyleInfo;
										pStyleNode->get_text(&bstrSyleInfo);
										newStyleInfo.m_strStyleInfo = _com_util::ConvertBSTRToString(bstrSyleInfo);
										SysFreeString(bstrSyleInfo);
									}
									if(wcscmp(bNodeName, _bstr_t("LegendURL")) == 0)
									{
										IXMLDOMNode *pFirstChildNode = NULL;
										pStyleNode->get_firstChild(&pFirstChildNode);
										pFirstChildNode->get_nodeName(&bstrNodeName);
										bNodeName = bstrNodeName;
										if(wcscmp(bNodeName, _bstr_t("Format")) != 0)
										{
											SysFreeString(bstrNodeName);
											continue;
										}
										else
										{
											SysFreeString(bstrNodeName);
											bstrNodeName = NULL;
											pFirstChildNode->get_text(&bstrNodeName);
											newStyleInfo.m_strImageFormat = _com_util::ConvertBSTRToString(bstrNodeName);
											SysFreeString(bstrNodeName);
											bstrNodeName = NULL;
										}
										pFirstChildNode->Release();
										pFirstChildNode = NULL;
									}
									SysFreeString(bstrNodeName);
								}
								newLayer.m_vecStyleInfo.push_back(newStyleInfo);
							}
							else if(wcscmp(bNodeName, _bstr_t("Attribution")) == 0)
							{
								DEUStyleInfo newStyleInfo;
	
								newStyleInfo.m_strStyleInfo = "default";

								newStyleInfo.m_strImageFormat = "image/png";

								newLayer.m_vecStyleInfo.push_back(newStyleInfo);
							}

						}
					}
				}
			}
			pLayerChildNodeList->reset();
			pLayerChildNodeList->Release();
			pLayerChildNodeList = NULL;
			pLayer->Release();
			pLayer = NULL;

			parrLayerInfo->push_back(newLayer);
		}
		return true;
	}
}

