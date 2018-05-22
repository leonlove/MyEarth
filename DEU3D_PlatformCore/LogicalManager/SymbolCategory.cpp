#include "SymbolCategory.h"


#include <DEUDBProxy/IDEUDBProxy.h>

#include <IDProvider/Definer.h>

#pragma warning (disable:4172)

namespace logical
{
    SymbolManager* SymbolCategory::s_pSymbolManager = NULL;

    SymbolCategory::SymbolCategory(ID& id)
    {
        m_id = id;
    }


    SymbolCategory::~SymbolCategory(void)
    {
    }


    const std::string&  SymbolCategory::getName(void) const
    {
        return m_strName;
    }


    const ID& SymbolCategory::getID(void) const
    {
        return m_id;
    }


    const std::string& SymbolCategory::getDescription(void) const
    {
        return m_strDesc;
    }


    void SymbolCategory::setDescription(const std::string& strDesc)
    {
        m_strDesc = strDesc;
    }


    void SymbolCategory::setName(const std::string &strName)
    {
        this->m_strName = strName;
    }


    unsigned SymbolCategory::getSymbolsCount(void) const
    {
        return m_vecSymbols.size();
    }


    const ID& SymbolCategory::getSymbol(unsigned nIndex) const
    {
        return m_vecSymbols[nIndex];
    }


    bool SymbolCategory::addSymbol(const ID &id)
    {
        std::vector<ID>::const_iterator itorFind = std::find_if(m_vecSymbols.begin(), m_vecSymbols.end(), Finder(id));
        if(itorFind != m_vecSymbols.end())
        {
            return false;
        }

        m_vecSymbols.push_back(id);

        return true;
    }


    bool SymbolCategory::removeSymbol(const ID &id)
    {
        std::vector<ID>::iterator it= m_vecSymbols.begin();
        for(; it != m_vecSymbols.end(); it++)
        {
            if((*it) == id)
            {
                m_vecSymbols.erase(it);
                return true;
            }
        }

        return false;
    }


    bool SymbolCategory::addSymbolDB(const std::string &strDB)
    {
        bool isExist = false;
        std::string  strTemp = strDB;
        std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::toupper);

        //检查重复
        ItMapDB _ItMapDB(m_mapDBs.begin());
        for(;_ItMapDB != m_mapDBs.end(); _ItMapDB++)
        {
            if(_ItMapDB->first == strTemp)
            {
                isExist = true;
            }
        }

        //不存在重复的情况下追加入DB路径
        if(!isExist){
            OpenSP::sp<deudbProxy::IDEUDBProxy> pDB = deudbProxy::createDEUDBProxy();
            if(pDB->openDB(strTemp))
            {            
                m_mapDBs.insert(std::make_pair<std::string, OpenSP::sp<deudbProxy::IDEUDBProxy>>(strTemp, pDB));
            }
        }

        return false;
    }


    ///////////////////////////////// Sub /////////////////////////////////////////
    ISymbolCategory* SymbolCategory::createSubCategory(const std::string &strName)
    {
        ID id = ID::genNewID();
        id.ObjectID.m_nDataSetCode = 6;
        id.ObjectID.m_nType        = SYMBOL_CATEGORY_ID;
        ISymbolCategory* pSC = new SymbolCategory(id);
        pSC->setName(strName);

        addSubCategory(pSC);

        return pSC;
    }


    bool SymbolCategory::addSubCategory(ISymbolCategory *pCategory)
    {
        if(pCategory)
        {
            std::vector<OpenSP::sp<logical::ISymbolCategory>>::const_iterator itorFind = std::find_if(m_vecSubSymbolCategory.begin(), m_vecSubSymbolCategory.end(), Finder(pCategory));
            if(itorFind != m_vecSubSymbolCategory.end())
            {
                return false;
            }

            m_vecSubSymbolCategory.push_back(pCategory);
            ((SymbolCategory*)pCategory)->addParent(pCategory);
        }

        return true;
    }


    bool SymbolCategory::removeSubCategory(const ISymbolCategory *pCategory)
    {
        std::vector<OpenSP::sp<logical::ISymbolCategory>>::iterator it(m_vecSubSymbolCategory.begin());
        for(;it != m_vecSubSymbolCategory.end(); it++)
        {
            if(*it == pCategory)
            {
                m_vecSubSymbolCategory.erase(it);
                return true;
            }
        }

        return false;
    }


    unsigned SymbolCategory::getSubCategoriesCount(void) const
    {
        return m_vecSubSymbolCategory.size();
    }


    ISymbolCategory* SymbolCategory::getSubCategory(unsigned nIndex)
    {
        if(nIndex < m_vecSubSymbolCategory.size())
        {
            return m_vecSubSymbolCategory[nIndex].get();
        }

        return NULL;
    }


    const ISymbolCategory* SymbolCategory::getSubCategory(unsigned nIndex) const
    {
        if(nIndex < m_vecSubSymbolCategory.size())
        {
            return m_vecSubSymbolCategory[nIndex];
        }

        return NULL;
    }


    //////////////////////////////// Parent //////////////////////////////////////////
    bool SymbolCategory::addParent(ISymbolCategory *pCategory)
    {
        std::vector<OpenSP::sp<logical::ISymbolCategory>>::const_iterator itorFind = std::find_if(m_vecParentSymbolCategory.begin(), m_vecParentSymbolCategory.end(), Finder(pCategory));
        if(itorFind != m_vecParentSymbolCategory.end())
        {
            return false;
        }

        m_vecParentSymbolCategory.push_back(pCategory);

        return true;
    }


    unsigned SymbolCategory::getParentCount(void) const
    {
        return m_vecParentSymbolCategory.size();
    }


    ISymbolCategory* SymbolCategory::getParent(unsigned nIndex)
    {
        if(nIndex < m_vecParentSymbolCategory.size())
        {
            return m_vecParentSymbolCategory[nIndex];
        }

        return NULL;
    }


    const ISymbolCategory* SymbolCategory::getParent(unsigned nIndex) const
    {
        if(nIndex < m_vecParentSymbolCategory.size())
        {
            return m_vecParentSymbolCategory[nIndex];
        }

        return NULL;
    }


    //递归解析bson
    void SymbolCategory::LocalBsonToObj(bson::bsonDocument& Doc)
    {
        bson::bsonElement* ElemID = Doc.GetElement("ID");
        if(ElemID)
        {
            std::string sID;
            ElemID->ValueString(sID, false);
            m_id.fromString(sID);
        }

        bson::bsonElement* ElemName = Doc.GetElement("Name");
        if(ElemName)
        {
            ElemName->ValueString(m_strName, false);
        }

        bson::bsonElement* ElemDesc = Doc.GetElement("Description");
        if(ElemDesc)
        {
            ElemDesc->ValueString(m_strDesc);
        }

        bson::bsonArrayEle* ElemAryChild = dynamic_cast<bson::bsonArrayEle*>(Doc.GetElement("ChildrenID"));
        if(ElemAryChild)
        {
            unsigned int nCount = ElemAryChild->ChildCount();
            for(unsigned n=0; n<nCount; n++)
            {
                bson::bsonElement* p_element = ElemAryChild->GetElement(n);
                bson::bsonElementType type   = p_element->GetType();
                if(type != bson::bsonDocType) //ID
                {
                    std::string strID;
                    p_element->ValueString(strID, false);
                    ID idtmp = ID::genIDfromString(strID);
                    addSymbol(idtmp);
                }
                else //递归
                {
                    bson::bsonDocumentEle* p_elemDoc = dynamic_cast<bson::bsonDocumentEle*>(ElemAryChild->GetElement(n));
                    if(p_elemDoc)
                    {
                        ID id;
                        OpenSP::sp<SymbolCategory> pSubCategory = new SymbolCategory(id);
                        addSubCategory(pSubCategory);

                        pSubCategory->LocalBsonToObj(p_elemDoc->GetDoc());
                    }
                }
            }
        }
    }


    //递归解析数据到bson
    void SymbolCategory::LocalObjToBson(bson::bsonDocument& Doc)
    {
        std::string strID          = m_id.toString();
        std::string strName        = m_strName;
        std::string strDescription = m_strDesc;

        Doc.AddStringElement("ID", strID.c_str());
        Doc.AddStringElement("Name", strName.c_str());
        Doc.AddStringElement("Description", strDescription.c_str());

        bson::bsonArrayEle* pChildAry = dynamic_cast<bson::bsonArrayEle*>(Doc.AddArrayElement("ChildrenID"));
        if(!pChildAry) return;

        int nCount = getSymbolsCount();
        for(int n=0; n<nCount; n++)
        {
            ID id = getSymbol(n);
            pChildAry->AddStringElement(id.toString().c_str());
        }

        nCount = getSubCategoriesCount();
        for(int n=0; n<nCount; n++)
        {
            SymbolCategory* pChildSC = (SymbolCategory*)getSubCategory(n);
            bson::bsonDocumentEle* pChildDocEle = dynamic_cast<bson::bsonDocumentEle*>(pChildAry->AddDocumentElement());
            if(!pChildDocEle) continue;
            pChildSC->LocalObjToBson(pChildDocEle->GetDoc());
        }
    }


    //文本为json格式
    bool SymbolCategory::saveLocalCategory(const std::string &strCategoryFile)
    {
        FILE* pfile;
        fopen_s(&pfile, strCategoryFile.c_str(), "w");
        if(pfile)
        {
            bson::bsonDocument Doc;
            LocalObjToBson(Doc);
            std::string strJsonBuff;
            Doc.JsonString(strJsonBuff);

            fwrite(strJsonBuff.c_str(), 1, strJsonBuff.length(), pfile);
            fclose(pfile);

            return true;
        }

        return false;
    }


    bool SymbolCategory::initNetwork(const ID& id)
    {
        OpenSP::sp<IProperty> pProperty = s_pSymbolManager->getPropertyManager()->findProperty(id);

        if (pProperty == NULL || pProperty->getChildrenCount() == 0)
        {
            return false;
        }

        OpenSP::sp<IProperty> pIDProp = pProperty->findProperty("ID");
        if (pIDProp.valid())
        {
            const cmm::variant_data &varID = pIDProp->getValue();
            if(varID.m_eValidate == cmm::variant_data::VT_ID)
            {
                m_id = (ID)varID;
            }
            else if(varID.m_eValidate == cmm::variant_data::VT_String)
            {
                m_id.fromString((std::string)varID);
            }
        }

        OpenSP::sp<IProperty> pNameProp = pProperty->findProperty("Name");
        if(pNameProp.valid())
        {
            const cmm::variant_data &varName = pNameProp->getValue();
            if(varName.m_eValidate == cmm::variant_data::VT_String)
            {
                m_strName = (std::string)varName;
            }
        }

        OpenSP::sp<IProperty> pDescProp = pProperty->findProperty("Descriptions");
        if(pDescProp.valid())
        {
            const cmm::variant_data &varDesc = pDescProp->getValue();
            if(varDesc.m_eValidate == cmm::variant_data::VT_String)
            {
                m_strDesc = (std::string)varDesc;
            }
        }

        OpenSP::sp<IProperty> pChildrenProp = pProperty->findProperty("ChildrenID");
        if(!pChildrenProp.valid())
        {
            return false;
        }

        const unsigned nChildrenCount = pChildrenProp->getChildrenCount();
        for(unsigned n = 0u; n < nChildrenCount; n++)
        {
            OpenSP::sp<IProperty> pChild = pChildrenProp->getChild(n);
            const cmm::variant_data &varChild = pChild->getValue();

            ID idChild;
            if(varChild.m_eValidate == cmm::variant_data::VT_ID)
            {
                idChild = (ID)varChild;
            }
            else if(varChild.m_eValidate == cmm::variant_data::VT_String)
            {
                idChild.fromString((std::string)varChild);
            }
            else continue;

            if (idChild.ObjectID.m_nType == SYMBOL_ID)
            {
                addSymbol(idChild);
            }
            else if (idChild.ObjectID.m_nType == SYMBOL_CATEGORY_ID)
            {
                OpenSP::sp<ISymbolCategory> pChildCategory   = new SymbolCategory(idChild);

                addSubCategory(pChildCategory);

                ((SymbolCategory*)pChildCategory.get())->initNetwork(idChild);
            }
        }

        return false;
    }
}
