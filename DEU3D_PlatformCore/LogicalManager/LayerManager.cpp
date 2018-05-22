#include "LayerManager.h"
#include "Layer.h"
#include "IPropertyManager.h"
#include <iostream>
#include <common/Common.h>

#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventFilter.h>
#include <common/StateDefiner.h>
#include <time.h>

namespace logical
{

void LayerManager::fastInsert(std::vector<CacheItem>::iterator itorPos, const CacheItem &item)
{
    if(itorPos == m_vecObjectCache.end())
    {
        if(m_vecObjectCache.capacity() > m_vecObjectCache.size())
        {
            m_vecObjectCache.push_back(item);
        }
        else
        {
            std::vector<CacheItem> vecTempCache;
            vecTempCache.reserve(m_vecObjectCache.size() + 16u * 1024u);
            vecTempCache.resize(m_vecObjectCache.size() + 1u);

            const unsigned nSize = (const unsigned char *)(&vecTempCache.back()) - (const unsigned char *)(vecTempCache.data());
            memcpy(vecTempCache.data(), m_vecObjectCache.data(), nSize);
            vecTempCache[m_vecObjectCache.size()] = item;
            m_vecObjectCache.swap(vecTempCache);
        }
    }
    else
    {
        if(m_vecObjectCache.capacity() > m_vecObjectCache.size())
        {
            m_vecObjectCache.resize(m_vecObjectCache.size() + 1u);
            const unsigned nPos = itorPos - m_vecObjectCache.begin();
            unsigned char *pPos = (unsigned char *)&m_vecObjectCache[nPos];
            const unsigned char *pEnd = (const unsigned char *)(&m_vecObjectCache.back());

            const unsigned nSize = pEnd - pPos;

            memcpy(&m_vecObjectCache[nPos + 1], &m_vecObjectCache[nPos], nSize);

            m_vecObjectCache[nPos] = item;
        }
        else
        {
            std::vector<CacheItem>    vecTempCache;
            vecTempCache.reserve(m_vecObjectCache.size() + 16u * 1024u);
            vecTempCache.resize(m_vecObjectCache.size() + 1u);

            unsigned char *pFirst = (unsigned char *)m_vecObjectCache.data();
            unsigned char *pPos   = (unsigned char *)&*itorPos;
            const unsigned nSize1 = pPos - pFirst;
            if(nSize1 > 0u)
            {
                memcpy(vecTempCache.data(), pFirst, nSize1);
            }

            const int nPos = itorPos - m_vecObjectCache.begin();
            const unsigned char *pEnd = (const unsigned char *)(&m_vecObjectCache.back() + 1u);
            const unsigned nSize2 = pEnd - pPos;
            if(nSize2 > 0u)
            {
                memcpy(vecTempCache.data() + nPos + 1, pPos, nSize2);
            }

            vecTempCache[nPos] = item;

            m_vecObjectCache.swap(vecTempCache);
        }
    }
}


void LayerManager::fastInsert_back(const std::vector<CacheItem> &vecItems)
{
    if(m_vecObjectCache.capacity() > m_vecObjectCache.size() + vecItems.size())
    {
        if(m_vecObjectCache.size() > 0u)
        {
            unsigned char *pPos = (unsigned char *)(&m_vecObjectCache.back() + 1);
            m_vecObjectCache.resize(m_vecObjectCache.size() + vecItems.size());

            const unsigned nSize = vecItems.size() * sizeof(CacheItem);
            memcpy(pPos, vecItems.data(), nSize);
        }
        else
        {
            m_vecObjectCache.resize(m_vecObjectCache.size() + vecItems.size());
            unsigned char *pPos = (unsigned char *)&m_vecObjectCache.front();

            const unsigned nSize = vecItems.size() * sizeof(CacheItem);
            memcpy(pPos, vecItems.data(), nSize);
        }
    }
    else
    {
        unsigned n = sizeof(CacheItem);
        std::vector<CacheItem> vecTempCache;
        const unsigned nSize0 = m_vecObjectCache.size() + vecItems.size();
        vecTempCache.reserve(nSize0 * 2u);
        vecTempCache.resize(nSize0);

        const unsigned nSize1 = m_vecObjectCache.size() * sizeof(CacheItem);
        memcpy(vecTempCache.data(), m_vecObjectCache.data(), nSize1);

        const unsigned nSize2 = vecItems.size() * sizeof(CacheItem);
        memcpy(vecTempCache.data() + m_vecObjectCache.size(), vecItems.data(), nSize2);

        m_vecObjectCache.swap(vecTempCache);
    }
}


ILayerManager *createLayerManager(void)
{
    static OpenSP::sp<LayerManager> pLayerManager = NULL;
    if(!pLayerManager.valid())
    {
        pLayerManager = new LayerManager;
        Object::ms_pLayerManager = pLayerManager.get();
    }

    return pLayerManager.release();
}


LayerManager::LayerManager(void) :
    m_pPropertyManager(NULL),
    m_pCultureRootLayer(NULL),
    m_pTerrainDEMRootLayer(NULL),
    m_pTerrainDOMRootLayer(NULL),
    m_pStateRefreshingThread(NULL),
    m_bInitialized(false)
{
}


LayerManager::~LayerManager(void)
{
    unInitialize();
}


void LayerManager::addLayerToCache(const ID &id, Layer *pLayer)
{
    if(!pLayer)    return;
    if(id.ObjectID.m_nType != CULTURE_LAYER_ID &&
        id.ObjectID.m_nType != TERRAIN_DEM_LAYER_ID &&
        id.ObjectID.m_nType != TERRAIN_DOM_LAYER_ID)
    {
        return;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxLayerCache);
    m_mapLayerCache[id] = pLayer;
}


void LayerManager::removeLayerFromCache(const ID &id)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxLayerCache);
    m_mapLayerCache.erase(id);
}


Layer *LayerManager::findLayerInCache(const ID &id)
{
    if(id.ObjectID.m_nType != CULTURE_LAYER_ID &&
        id.ObjectID.m_nType != TERRAIN_DEM_LAYER_ID &&
        id.ObjectID.m_nType != TERRAIN_DOM_LAYER_ID)
    {
        return NULL;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxLayerCache);
    std::map<ID, Layer *>::iterator itorFind = m_mapLayerCache.find(id);
    if(itorFind == m_mapLayerCache.end())
    {
        return NULL;
    }
    return itorFind->second;
}


void LayerManager::addObjectToCache(const ID &id, const Layer *pLayer)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectCache);
    std::vector<CacheItem>::iterator itorPos = std::lower_bound(m_vecObjectCache.begin(), m_vecObjectCache.end(), id, LowerBound_CacheItem());
    CacheItem item;
    item.m_id = id;
    item.m_pParent = pLayer;
    fastInsert(itorPos, item);
}


void LayerManager::addObjectsToCache(const std::vector<CacheItem> &vecCacheItems)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectCache);
    fastInsert_back(vecCacheItems);
    std::sort(m_vecObjectCache.begin(), m_vecObjectCache.end());
}


void LayerManager::removeObjectFromCache(const ID &id, const Layer *pLayer)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectCache);
    std::vector<CacheItem>::iterator itorPos = std::lower_bound(m_vecObjectCache.begin(), m_vecObjectCache.end(), id, LowerBound_CacheItem());
    if(itorPos == m_vecObjectCache.end())
    {
        return;
    }

    const CacheItem &item = *itorPos;
    if(item.m_id != id)
    {
        return;
    }

    std::vector<CacheItem>::iterator itor = itorPos;
    while(itor != m_vecObjectCache.end())
    {
        CacheItem &item = *itor;
        if(item.m_id != id)
        {
            break;
        }

        if(item.m_pParent == pLayer)
        {
            itor = m_vecObjectCache.erase(itor);
            continue;
        }
        ++itor;
    }
}


void LayerManager::findParentsOfObject(const ID &id, std::vector<const Layer *> &vecParents) const
{
    vecParents.clear();
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectCache);
    std::vector<CacheItem>::const_iterator itorPos = std::lower_bound(m_vecObjectCache.begin(), m_vecObjectCache.end(), id, LowerBound_CacheItem());
    if(itorPos == m_vecObjectCache.end())
    {
        return;
    }

    const CacheItem &item = *itorPos;
    if(item.m_id != id)
    {
        return;
    }

    std::vector<CacheItem>::const_iterator itor = itorPos;
    while(itor != m_vecObjectCache.end())
    {
        const CacheItem &item = *itor;
        if(item.m_id != id)
        {
            break;
        }

        vecParents.push_back(item.m_pParent);
        ++itor;
    }
}


bool LayerManager::isExistInLayer(const ID &id, const Layer *pLayer) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxObjectCache);
    std::vector<CacheItem>::const_iterator itorPos = std::lower_bound(m_vecObjectCache.begin(), m_vecObjectCache.end(), id, LowerBound_CacheItem());
    if(itorPos == m_vecObjectCache.end())
    {
        return false;
    }

    const CacheItem &item = *itorPos;
    if(item.m_id != id)
    {
        return false;
    }

    std::vector<CacheItem>::const_iterator itor = itorPos;
    while(itor != m_vecObjectCache.end())
    {
        const CacheItem &item = *itor;
        if(item.m_id != id)
        {
            break;
        }

        if(item.m_pParent == pLayer)
        {
            return true;
        }
        ++itor;
    }

    return false;
}


bool LayerManager::login(const std::string& strAuthHost, const std::string& strAuthPort, const std::string& strUserName, const std::string& strUserPwd)
{
    if(!m_pPropertyManager.valid())
    {
        m_pPropertyManager = new PropertyManager;
    }
    return m_pPropertyManager->login(strAuthHost, strAuthPort, strUserName, strUserPwd);
}


bool LayerManager::logout()
{
    if(m_pPropertyManager.valid())
    {
        return m_pPropertyManager->logout();
    }
    return false;
}


bool LayerManager::initialize(const std::string &strHost, const std::string &strPort, const std::string &strLocalCache)
{
    m_pCultureRootLayer    = NULL;
    m_pTerrainDEMRootLayer = NULL;
    m_pTerrainDOMRootLayer = NULL;

    if(!m_pPropertyManager.valid())
    {
        m_pPropertyManager = new PropertyManager;
    }

    if(!m_pPropertyManager->initialize(strHost, strPort, strLocalCache))
    {
        return false;
    }

    m_mapLayerCache.clear();
    m_vecObjectCache.clear();
    m_pInitThreadPool = new InitializationThreadPool;
    m_pInitThreadPool->initialize(4u);

    clock_t t0 = clock();
    //ÁãÊ±ÐéÄâÍßÆ¬
    m_pLocalVCubeManager = vcm::createVirtualCubeManager();

    //µØÐÎÍ¼²ã
    const ID idTerrainDEMRoot = ID::getTerrainDEMLayerRootID();
    m_pTerrainDEMRootLayer = new Layer(idTerrainDEMRoot);
    std::cout << "In initialization of LayerManager, init the root layer of terrain DEM." << std::endl;
    //m_pInitThreadPool->addTask(m_pTerrainDEMRootLayer.get());
    m_pTerrainDEMRootLayer->init();

    //Ó°ÏñÍ¼²ã
    const ID idTerrainDOMRoot = ID::getTerrainDOMLayerRootID();
    m_pTerrainDOMRootLayer = new Layer(idTerrainDOMRoot);
    std::cout << "In initialization of LayerManager, init the root layer of terrain DOM." << std::endl;
    //m_pInitThreadPool->addTask(m_pTerrainDOMRootLayer.get());
    m_pTerrainDOMRootLayer->init();

    //Âß¼­Í¼²ã
    const ID idLogicalRoot = ID::getCultureLayerRootID();
    m_pCultureRootLayer = new Layer(idLogicalRoot);
    std::cout << "In initialization of LayerManager, init the root layer of logical." << std::endl;
    m_pInitThreadPool->addTask(m_pCultureRootLayer.get());

    m_bInitialized = true;

    return true;
}


void LayerManager::unInitialize(void)
{
    if(!m_bInitialized) return;

    if(m_pStateRefreshingThread.valid())
    {
        m_pStateRefreshingThread->finishMission();
        m_pStateRefreshingThread = NULL;
    }
    m_pInitThreadPool = NULL;

    if (m_pPropertyManager != NULL)
    {
        m_pPropertyManager->logout();
        m_pPropertyManager = NULL;
    }

    m_pCultureRootLayer = NULL;
    m_pTerrainDEMRootLayer = NULL;
    m_pTerrainDOMRootLayer = NULL;
    m_pLocalVCubeManager = NULL;
    m_bInitialized = false;
}


IObject *LayerManager::findObject(const ID &id)
{
    IObject* pObject = NULL;
    //ÈËÎÄÍ¼²ã
    const ID idCultureRoot    = ID::getCultureLayerRootID();
    //µØÐÎÍ¼²ã
    const ID idTerrainDEMRoot = ID::getTerrainDEMLayerRootID();
    //Ó°ÏñÍ¼²ã
    const ID idTerrainDOMRoot = ID::getTerrainDOMLayerRootID();
    if(idCultureRoot == id)
    {
        pObject = (IObject*)m_pCultureRootLayer;
    }
    else if(idTerrainDEMRoot == id)
    {
        pObject = (IObject*)m_pTerrainDEMRootLayer;
    }
    else if(idTerrainDOMRoot == id)
    {
        pObject = (IObject*)m_pTerrainDOMRootLayer;
    }

    if(pObject)
    {
        return pObject;
    }

    bool bFound = false;
    switch (id.ObjectID.m_nType)
    {
    case CULTURE_LAYER_ID :
    case PARAM_POINT_ID :
    case PARAM_LINE_ID :
    case PARAM_FACE_ID :
        {
            return findObject(id, m_pCultureRootLayer, bFound);
        }
    case TERRAIN_DEM_LAYER_ID :
    case TERRAIN_DEM_ID :
        {
            return findObject(id, m_pTerrainDEMRootLayer, bFound);
        }
    case TERRAIN_DOM_LAYER_ID :
    case  TERRAIN_DOM_ID :
        {
            return findObject(id, m_pTerrainDOMRootLayer, bFound);
        }
    default:
        break;
    }
    return NULL;
}


IObject *LayerManager::findObject(const ID &id, ILayer *pLayer, bool &bFound)
{
    assert(pLayer != NULL);

    if(pLayer->getID() == id)
    {
        bFound = true;
        return pLayer;
    }

    if(pLayer->hasChild(id))
    {
        bFound = true;

        Layer *pActLayer = dynamic_cast<Layer *>(pLayer);
        return pActLayer->getChild(id);
    }

    const unsigned nSubLayerCount = pLayer->getChildrenLayerCount();
    for(unsigned nIndex = 0u; nIndex < nSubLayerCount; nIndex++)
    {
        ILayer *pChildLayer = dynamic_cast<ILayer *>(pLayer->getChildLayer(nIndex));
        if(!pChildLayer)    continue;

        IObject *pFound = findObject(id, pChildLayer, bFound);
        if(bFound)          return pFound;
    }

    return NULL;
}


void LayerManager::getObjectStates(const ID &id, std::map<std::string, bool> &objectstates) const
{
    std::vector<const Layer *> listParents;
    findParentsOfObject(id, listParents);

    for(std::vector<const Layer *>::const_iterator itor = listParents.begin(); itor != listParents.end(); ++itor)
    {
        if (id == (*itor)->getID())
        {
            (*itor)->getAllState(objectstates);
        }
        else
        {
            (*itor)->getChildAllState(id, objectstates);
        }
    }

    return;
}


cmm::StateValue LayerManager::queryObjectState(const ID &id, const std::string &strStateName) const
{
    std::vector<const Layer *> listParents;
    findParentsOfObject(id, listParents);

    if(listParents.empty())
    {
        if(m_pInitThreadPool->isBusy())
        {
            return cmm::SV_Unknown;
        }
        else
        {
            return cmm::SV_Disable;
        }
    }

    for(std::vector<const Layer *>::const_iterator itor = listParents.begin(); itor != listParents.end(); ++itor)
    {
        const Layer *pLayer = *itor;

        bool bState = pLayer->getChildState(id, strStateName);
        if(bState)  return cmm::SV_Enable;
    }

    return cmm::SV_Disable;
}


void LayerManager::getParents(const ID &id, IDList &listParents) const
{
    listParents.clear();

    OpenSP::sp<IProperty>    pProperty = m_pPropertyManager->findProperty(id);
    if(!pProperty.valid())  return;

    OpenSP::sp<IProperty>    pParentID = pProperty->findProperty("ParentID");
    if(!pParentID.valid())  return;

    const cmm::variant_data &varParents = pParentID->getValue();
    if(varParents.m_eValidate == cmm::variant_data::VT_IDList)
    {
        const std::vector<ID> &vecIDs = (const std::vector<ID> &)varParents;
        listParents.assign(vecIDs.begin(), vecIDs.end());
    }
    else if(varParents.m_eValidate == cmm::variant_data::VT_String)
    {
        listParents.resize(1);
        const std::string &strID = (std::string)varParents;
        ID &idParent = listParents[0];
        idParent.fromString(strID);
    }
    else if(varParents.m_eValidate == cmm::variant_data::VT_StringList)
    {
        const std::vector<std::string> &vecStrings = (const std::vector<std::string> &)(varParents);
        listParents.resize(vecStrings.size());
        IDList::iterator itorID = listParents.begin();
        std::vector<std::string>::const_iterator itorStrID = vecStrings.begin();
        for( ; itorStrID != vecStrings.end(); ++itorStrID)
        {
            const std::string &strID = *itorStrID;
            ID &idParent = *itorID;
            idParent.fromString(strID);
            if(idParent.isValid())
            {
                ++itorID;
            }
        }
    }
}


void LayerManager::setStateImplementer(cmm::IStateImplementer *pStateImplementer)
{
    if(m_pStateRefreshingThread.valid())
    {
        m_pStateRefreshingThread->finishMission();
        m_pStateRefreshingThread = NULL;
    }

    m_pStateRefreshingThread = new StateRefreshingThread(this, pStateImplementer);
    m_pStateRefreshingThread->startThread();
}


void LayerManager::refreashObjectState(const ID &id, const std::string &strStateName, bool bState)
{
    if(!m_pStateRefreshingThread.valid())
    {
        return;
    }

    OpenSP::sp<StateRefreshingThread::Task>     pTask = new StateRefreshingThread::Task(id, strStateName, bState);
    m_pStateRefreshingThread->pushTask(pTask.get());
}



}