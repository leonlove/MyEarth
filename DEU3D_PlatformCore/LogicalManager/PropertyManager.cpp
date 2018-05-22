#include "PropertyManager.h"
#include "Property.h"
#include <algorithm>

#include <Common/DEUBson.h>

#include <IDProvider/Definer.h>

#include "time.h"

#include "DEUCheck.h"
DEUCheck    checker(18, 3);

#if _DEBUG
#include <sstream>
#include <iostream>
#endif

namespace logical
{

IPropertyManager *createPropertyManager(void)
{
    OpenSP::sp<PropertyManager> pPropertyManager = new PropertyManager;
    return pPropertyManager.release();
}


PropertyManager::PropertyManager(void) :
    m_pDEUNetwork(NULL)
{
}


PropertyManager::~PropertyManager(void)
{
    closeLocalDB();
}


void PropertyManager::closeLocalDB(void)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPropertyServers);
    for(std::map< std::string, OpenSP::sp<deudbProxy::IDEUDBProxy> >::iterator itor = m_mapPropertyServer.begin();
        itor != m_mapPropertyServer.end(); ++itor)
    {
        itor->second->closeDB();
        itor->second = NULL;
    }

    m_mapPropertyServer.clear();
}


bool PropertyManager::login(const std::string &strAuthHost, const std::string &strAuthPort, const std::string &strUserName, const std::string &strUserPwd)
{
    if(!m_pDEUNetwork.valid())
    {
        m_pDEUNetwork = deunw::createDEUNetwork();
    }

    if(!m_pDEUNetwork->login(strAuthHost, strAuthPort, strUserName, strUserPwd))
    {
        m_pDEUNetwork = NULL;
        return false;
    }

    return true;
}


bool PropertyManager::logout()
{
    if(m_pDEUNetwork.valid())
    {
        return m_pDEUNetwork->logout();
    }
    return false;
}


bool PropertyManager::initialize(const std::string &strHost, const std::string &strPort, const std::string &strLocalCache)
{
    //std::cout << "sizeof property = " << sizeof(Property) << std::endl;
    //std::cout << "sizeof variant_data = " << sizeof(cmm::variant_data) << std::endl;
    if(!m_pDEUNetwork.valid())
    {
        m_pDEUNetwork = deunw::createDEUNetwork();
    }

    //把本地临时DB加进去
    {
        const std::string strTempDBFile = cmm::genLocalTempDB();
        addLocalPropertyServer(strTempDBFile);
    }

    m_pDEUNetwork->initialize(strHost, strPort, true, strLocalCache);

    return true;
}


bool PropertyManager::addLocalPropertyServer(const std::string &strServer)
{
    std::string strServer1 = strServer;
    std::transform(strServer.begin(), strServer.end(), strServer1.begin(), ::toupper);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPropertyServers);
    if(m_mapPropertyServer.find(strServer1) == m_mapPropertyServer.end())
    {
        OpenSP::sp<deudbProxy::IDEUDBProxy> pDEUDB = deudbProxy::createDEUDBProxy();
        const UINT_64 nBufferSize = cmm::getLocalTempDBBufferSize();
        if(pDEUDB->openDB(strServer1, nBufferSize, nBufferSize))
        {
            if (strServer == cmm::genLocalTempDB())
            {
                pDEUDB->setClearFlag(true);
            }
            m_mapPropertyServer[strServer1] = pDEUDB;
            return true;
        }
    }

    return false;
}


bool PropertyManager::removeLocalPropertyServer(const std::string &strServer)
{
    std::string strServer1 = strServer;
    std::transform(strServer.begin(), strServer.end(), strServer1.begin(), ::toupper);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPropertyServers);
    std::map< std::string, OpenSP::sp<deudbProxy::IDEUDBProxy> >::iterator itor = m_mapPropertyServer.find(strServer1);
    if(itor != m_mapPropertyServer.end())
    {
        itor->second->closeDB();
        m_mapPropertyServer.erase(itor);
    }

    return true;
}


IProperty *PropertyManager::findProperty(const ID &id, FindTarget eFindTarget, OpenSP::sp<cmm::IDEUException> pOutExcep)
{
    cmm::variant_data data;
    data.m_eValidate = cmm::variant_data::VT_Null;
    if(!id.isValid())
    {
        return NULL;
    }

    void *pData = NULL;
    unsigned int nDataLen = 0u;

    fetchPropertyData(id, pData, nDataLen, eFindTarget, pOutExcep);
    if(pData == NULL || nDataLen < 1u)
    {
        return NULL;
    }

    OpenSP::sp<IProperty> pProperty = createPropertyByBsonStream(pData, nDataLen);
    delete[] pData;

    return pProperty.release();
}


IProperty *PropertyManager::findProperty(const ID &id, OpenSP::sp<cmm::IDEUException> pOutExcep)
{
    return findProperty(id, FT_Auto, pOutExcep);
}


IProperty *PropertyManager::createPropertyByBsonStream(const void *pBsonStream, unsigned int nLen)
{
    bson::bsonDocument bsonDoc;
    bsonDoc.FromBsonStream(pBsonStream, nLen);

    IProperty *pProperty = analyseProperties(&bsonDoc);

    return pProperty;
}


IProperty *PropertyManager::createPropertyByJsonStream(const std::string &strJsonCode)
{
    bson::bsonDocument bsonDoc;
    bsonDoc.FromJsonString(strJsonCode);
    return analyseProperties(&bsonDoc);
}


Property *PropertyManager::analyseProperties(const bson::bsonDocument *pBsonDoc)
{
    OpenSP::sp<Property> pProperty = new Property;
#if 0
    std::string strJsonString;
    pBsonDoc->JsonString(strJsonString);
    std::cout << strJsonString.c_str() << std::endl;
#endif
    for(unsigned int i = 0; i < pBsonDoc->ChildCount(); i++)
    {
        const bson::bsonElement *pBsonEle = pBsonDoc->GetElement(i);

        OpenSP::sp<Property> pChildProperty = analyseProperties(pBsonEle);
        if(pChildProperty.valid())
        {
            pProperty->m_vecChildrenProperty.push_back(pChildProperty);
        }
    }

    return pProperty.release();
}


Property *PropertyManager::analyseProperties(const bson::bsonElement *pBsonEle)
{
    OpenSP::sp<Property> pProperty = new Property;

    const bson::bsonElementType eType = pBsonEle->GetType();
    pProperty->m_strTitle = pBsonEle->EName();

    switch(eType)
    {
    case bson::bsonDoubleType:
        {
            pProperty->m_bSimpleProperty = true;
            pProperty->m_varValue = pBsonEle->DblValue();
            break;
        }
    case bson::bsonInt32Type:
        {
            pProperty->m_bSimpleProperty = true;
            pProperty->m_varValue = pBsonEle->Int32Value();
            break;
        }
    case bson::bsonInt64Type:
        {
            pProperty->m_bSimpleProperty = true;
            pProperty->m_varValue = pBsonEle->Int64Value();
            break;
        }
    case bson::bsonStringType:
        {
            pProperty->m_bSimpleProperty = true;
            const char *pData = pBsonEle->StrValue();
            std::string strTemp = pData == NULL ? "" : pData;
            pProperty->m_varValue = strTemp;
            break;
        }
    case bson::bsonBoolType:
        {
            pProperty->m_bSimpleProperty = true;
            pProperty->m_varValue = pBsonEle->BoolValue();
            break;
        }
    case bson::bsonBinType:
        {
            pProperty->m_bSimpleProperty = true;
            unsigned int nLen = pBsonEle->BinDataLen();
            const void *pData = pBsonEle->BinData();
            if(pData && nLen > 0)
            {
                pProperty->m_varValue = (void *)new char[nLen];
                memcpy((void *)pProperty->m_varValue, pData, nLen);
            }
            break;
        }
    case bson::bsonArrayType:
    {
        pProperty->m_bSimpleProperty = false;

        const bson::bsonArrayEle *pArrayEle = dynamic_cast<const bson::bsonArrayEle*>(pBsonEle);
        for(unsigned i = 0;i < pArrayEle->ChildCount();i++)
        {
            const bson::bsonElement* pChildEle = pArrayEle->GetElement(i);
            OpenSP::sp<Property> pChildProperty = analyseProperties(pChildEle);
            pProperty->m_vecChildrenProperty.push_back(pChildProperty);
        }
        break;
    }
    case bson::bsonDocType:
    {
        pProperty->m_bSimpleProperty = false;
        const bson::bsonDocumentEle *pDocEle = dynamic_cast<const bson::bsonDocumentEle*>(pBsonEle);
        const bson::bsonDocument *pBsonDocEle = &pDocEle->GetDoc();
        pProperty = analyseProperties(pBsonDocEle);
        break;
    }
    default: return NULL;
    }

    return pProperty.release();
}


void PropertyManager::fetchPropertyData(const ID &id, void *&pData, unsigned int &nDataLen, FindTarget eFindTarget, OpenSP::sp<cmm::IDEUException> pOutExcep/* = NULL*/)
{
    if(TERRAIN_DEM_ID          != id.TileID.m_nType &&
       TERRAIN_DOM_ID          != id.TileID.m_nType &&
       CULTURE_LAYER_ID        != id.TileID.m_nType &&
       TERRAIN_DEM_LAYER_ID    != id.TileID.m_nType &&
       TERRAIN_DOM_LAYER_ID    != id.TileID.m_nType &&
       PARAM_POINT_ID          != id.TileID.m_nType &&
       PARAM_LINE_ID           != id.TileID.m_nType &&
       PARAM_FACE_ID           != id.TileID.m_nType &&
       SYMBOL_CATEGORY_ID      != id.TileID.m_nType)
    {
        return;
    }

    void *pBosnBuff     = NULL;
    //先在本地属性数据库中查找
    if(eFindTarget != FT_RemoteOnly)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPropertyServers);
        for(std::map< std::string, OpenSP::sp<deudbProxy::IDEUDBProxy> >::const_iterator itor = m_mapPropertyServer.begin();
            itor != m_mapPropertyServer.end(); ++itor)
        {
            deudbProxy::IDEUDBProxy *pDeuDBProxy = itor->second.get();
            if (pDeuDBProxy == NULL)
            {
                continue;
            }

            if(!pDeuDBProxy->isExist(id))
            {
                continue;
            }

            void *pBosnBuff     = NULL;
            if(!pDeuDBProxy->readBlock(id, pBosnBuff, nDataLen))
            {
                continue;
            }

            if(nDataLen > 0u)
            {
                pData = new char[nDataLen];
                memcpy(pData, pBosnBuff, nDataLen);
                deudbProxy::freeMemory(pBosnBuff);
            }
            return;
        }
    }

    if(eFindTarget != FT_LocalOnly)
    {
        //在服务器端查找
        if(!m_pDEUNetwork.valid())
        {
            return;
        }

        if(m_pDEUNetwork->queryData(id, pBosnBuff, nDataLen, pOutExcep))
        {
            if(nDataLen > 0u)
            {
                pData = new char[nDataLen];
                memcpy(pData, pBosnBuff, nDataLen);
                deunw::freeMemory(pBosnBuff);
            }
        }
    }

    return;
}


bool PropertyManager::saveProperties2Local(const std::string &strSaveName, IDList &idList)
{
    const UINT_64 nBufferSize = cmm::getLocalTempDBBufferSize();
    OpenSP::sp<deudbProxy::IDEUDBProxy> pSaveDB = deudbProxy::createDEUDBProxy();
    if(!pSaveDB->openDB(strSaveName, nBufferSize, nBufferSize))
    {
        return false;
    }

    for(IDList::iterator itor = idList.begin(); itor != idList.end(); ++itor)
    {
        const ID &idProp = *itor;

        void *pData = NULL;
        unsigned int nDataLen = 0;
        fetchPropertyData(idProp, pData, nDataLen);
        if(pData == NULL)
        {
            continue;
        }
        if(pSaveDB->isExist(idProp))
        {
            delete []pData;
            continue;
        }

        pSaveDB->addBlock(idProp, pData, nDataLen);
        delete []pData;
    }

    pSaveDB->closeDB();

    return true;
}

}