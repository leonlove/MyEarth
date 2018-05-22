#include "Layer.h"

#include <Common/Common.h>
#include <OpenSP/sp.h>
#include <OpenThreads/ScopedLock>
#include <EventAdapter/IEventObject.h>
#include <EventAdapter/IEventAdapter.h>
#include "../VirtualTileManager/IVirtualTileManager.h"
#include "IPropertyManager.h"

#include <algorithm>

#include <IDProvider/Definer.h>
#include <time.h>
#include <iostream>

#include "LayerManager.h"
#include "Instance.h"
#include "TerrainInstance.h"
#include "WinError.h"
#include "InitializationThread.h"

namespace logical
{

Layer::Layer(const ID &id) : Object(id)
{
    m_bInitialized = false;
    m_bBoundingSphereDirty = true;

    OpenSP::sp<LayerManager>    pLayerManager;
    if(ms_pLayerManager.lock(pLayerManager))
    {
        pLayerManager->addLayerToCache(m_id, this);
    }
}


Layer::~Layer(void)
{
    m_vecChildrenIDs.clear();

    m_vecChildrenLayerIDs.clear();
    m_mapChildrenLayer.clear();

    OpenSP::sp<LayerManager>    pLayerManager;
    if(ms_pLayerManager.lock(pLayerManager))
    {
        pLayerManager->removeLayerFromCache(m_id);
    }
}


bool Layer::init(void)
{
    if(m_bInitialized)  return true;

    PropertyManager *pPropertyMgr = NULL;
    OpenSP::sp<LayerManager>    pLayerManager;
    if(ms_pLayerManager.lock(pLayerManager))
    {
        pPropertyMgr = dynamic_cast<PropertyManager *>(pLayerManager->getPropertyManager());
    }

    if (pPropertyMgr == NULL)
    {
        return false;
    }

    void *pPropData = NULL;
    unsigned nDataLen = 0u;
    const PropertyManager::FindTarget eFindTarget = PropertyManager::FT_Auto;
    pPropertyMgr->fetchPropertyData(m_id, pPropData, nDataLen, eFindTarget);
    if(pPropData == NULL || nDataLen < 1u)
    {
        m_bInitialized = true;
        return true;
    }

    bson::bsonDocument bsonDoc;
    if(!bsonDoc.FromBsonStream(pPropData, nDataLen))
    {
        delete[] pPropData;
        m_bInitialized = false;
        return false;
    }
    delete[] pPropData;

    const bool bRet = fromBson(bsonDoc);

    m_bInitialized = bRet;
    return bRet;
}


bool Layer::fromBson(const bson::bsonDocument &bsonDoc)
{
    if(!__super::fromBson(bsonDoc))
    {
        return false;
    }

    const bson::bsonArrayEle *pChildrenElement = dynamic_cast<const bson::bsonArrayEle *>(bsonDoc.GetElement("ChildrenID"));
    if(!pChildrenElement)
    {
        return true;
    }

    m_bBoundingSphereDirty = false;

    const unsigned nChildrenCount = pChildrenElement->ChildCount();

    std::vector<LayerManager::CacheItem>  vecCacheItems;
    vecCacheItems.reserve(nChildrenCount);
    m_vecChildrenIDs.reserve(nChildrenCount);

    for(unsigned n = 0u; n < nChildrenCount; n++)
    {
        const bson::bsonElement *pIDElement = pChildrenElement->GetElement(n);
        if(!pIDElement) continue;

        bool bFound = false;
        ID idChild;
        if(!bFound)
        {
            const bson::bsonBinaryEle *pBinIDElement = dynamic_cast<const bson::bsonBinaryEle *>(pIDElement);
            if(pBinIDElement)
            {
                const unsigned __int64 *pID = (const unsigned __int64 *)(pBinIDElement->BinData());
                idChild.m_nLowBit  = pID[0];
                idChild.m_nMidBit  = pID[1];
                idChild.m_nHighBit = pID[2];
                bFound = true;
            }
        }

        if(!bFound)
        {
            const bson::bsonStringEle *pStrIDElement = dynamic_cast<const bson::bsonStringEle *>(pIDElement);
            if(pStrIDElement)
            {
                idChild.fromString(pStrIDElement->StrValue());
                bFound = true;
            }
        }

        if(!bFound) continue;

        if(addChild2(idChild))
        {
            vecCacheItems.push_back(LayerManager::CacheItem(idChild, this));
        }
    }

    if (vecCacheItems.size() <= 0)
    {
        return true;
    }

    std::sort(vecCacheItems.begin(), vecCacheItems.end());

    OpenSP::sp<LayerManager>    pLayerManager;
    if(ms_pLayerManager.lock(pLayerManager))
    {
        pLayerManager->addObjectsToCache(vecCacheItems);
    }

    return true;
}


ILayer *Layer::createSubLayer(void)
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(ms_pLayerManager.lock(pLayerManager))
        {
            pLayerManager->getInitializationThreadPool()->raisePriority(this);
        }
        else return NULL;
    }

    ID idChild = ID::genNewID();
    idChild.ObjectID.m_nDataSetCode = 6;
    idChild.ObjectID.m_nType = m_id.ObjectID.m_nType;

    addChild(idChild);

    return dynamic_cast<ILayer *>(getChild(idChild));
}


Object *Layer::createObject(unsigned nIndex)
{
    Object *pObject = NULL;

    const ID &id = m_vecChildrenIDs[nIndex];
    if(CULTURE_LAYER_ID == id.ObjectID.m_nType ||
        TERRAIN_DEM_LAYER_ID == id.ObjectID.m_nType ||
        TERRAIN_DOM_LAYER_ID == id.ObjectID.m_nType )
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return NULL;
        }

        Layer *pLayer = dynamic_cast<Layer *>(pLayerManager->findLayerInCache(id));
        if(!pLayer)
        {
            pLayer = new Layer(id);
            pLayerManager->getInitializationThreadPool()->addTask(pLayer);
        }

        pObject = pLayer;
    }
    else if(TERRAIN_DEM_ID == id.ObjectID.m_nType ||
        TERRAIN_DOM_ID == id.ObjectID.m_nType)
    {
        pObject = new TerrainInstance(id);
        pObject->init();
    }
    else if(PARAM_POINT_ID == id.ObjectID.m_nType ||
        PARAM_LINE_ID == id.ObjectID.m_nType ||
        PARAM_FACE_ID == id.ObjectID.m_nType)
    {
        pObject = new Instance(id);
        pObject->init();
    }

    if(!pObject)    return NULL;

    pObject->addParent(this);

    return pObject;
}


bool Layer::addChild(const ID &id)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }
    if(!m_bInitialized)
    {
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(hasChild(id))
    {
        return false;
    }

    if(!addChild2(id))
    {
        return false;
    }

    pLayerManager->addObjectToCache(id, this);
    return true;
}


bool Layer::addChild2(const ID &id)
{
    if(m_id.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID)
    {
        if(id.ObjectID.m_nType != TERRAIN_DOM_LAYER_ID && id.ObjectID.m_nType != TERRAIN_DOM_ID)
        {
            return false;
        }
    }
    else if(m_id.ObjectID.m_nType == TERRAIN_DEM_LAYER_ID)
    {
        if(id.ObjectID.m_nType != TERRAIN_DEM_LAYER_ID && id.ObjectID.m_nType != TERRAIN_DEM_ID)
        {
            return false;
        }
    }
    else if(m_id.ObjectID.m_nType == CULTURE_LAYER_ID)
    {
       if(id.ObjectID.m_nType != CULTURE_LAYER_ID &&
          id.ObjectID.m_nType != PARAM_POINT_ID &&
          id.ObjectID.m_nType != PARAM_LINE_ID &&
          id.ObjectID.m_nType != PARAM_FACE_ID)
        {
            return false;
        }
    }
    else    return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    m_vecChildrenIDs.push_back(id);

    //模型ID和参数化ID需要写入本地虚拟瓦片
    if((id.ObjectID.m_nType == PARAM_POINT_ID ||
        id.ObjectID.m_nType == PARAM_LINE_ID ||
        id.ObjectID.m_nType == PARAM_FACE_ID))
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return false;
        }

        PropertyManager *pPropertyManager = dynamic_cast<PropertyManager *>(pLayerManager->getPropertyManager());
        if (pPropertyManager == NULL)
        {
            return false;
        }

        const PropertyManager::FindTarget eFindTarget = PropertyManager::FT_LocalOnly;
        void *pData = NULL;
        unsigned nLength = 0u;
        pPropertyManager->fetchPropertyData(id, pData, nLength, eFindTarget);
        if(pData && nLength > 0u)
        {
            cmm::math::Sphered sphere;
            if(fetchBoundInfo(pData, nLength, sphere, true))
            {
                vcm::IVirtualCubeManager *pVirtualCubeManager = pLayerManager->getVirtualCubeManager();
                pVirtualCubeManager->addObject(id, sphere);
            }
            delete[] pData;
        }
    }

    if(id.ObjectID.m_nType == CULTURE_LAYER_ID ||
       id.ObjectID.m_nType == TERRAIN_DEM_LAYER_ID ||
       id.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID)
    {
        Object *pObject = createObject(m_vecChildrenIDs.size() - 1u);

        m_mapChildrenLayer[id] = dynamic_cast<Layer *>(pObject);
        std::map<ID, OpenSP::sp<Layer> >::const_iterator itorInsert = m_mapChildrenLayer.find(id);
        m_vecChildrenLayerIDs.push_back(id);
    }

    m_bBoundingSphereDirty = true;
    return true;
}


bool Layer::removeChild(const ID &id)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }

    if(!m_bInitialized)
    {
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(!hasChild(id))
    {
        return false;
    }

    if(!removeChild2(id))
    {
        return false;
    }

    pLayerManager->removeObjectFromCache(id, this);
    return true;
}


void Layer::removeChildren(void)
{
    const unsigned nCount = m_vecChildrenIDs.size();
    for(size_t i = 0u; i < nCount; i++)
    {
        removeChild(m_vecChildrenIDs.back());
    }
}


bool Layer::removeChild2(const ID &id)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    ChildrenList::const_iterator itorFind = std::find(m_vecChildrenIDs.begin(), m_vecChildrenIDs.end(), id);
    m_vecChildrenIDs.erase(itorFind);

    if(id.ObjectID.m_nType == CULTURE_LAYER_ID ||
        id.ObjectID.m_nType == TERRAIN_DEM_LAYER_ID ||
        id.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID)
    {
        itorFind = std::find(m_vecChildrenLayerIDs.begin(), m_vecChildrenLayerIDs.end(), id);
        m_vecChildrenLayerIDs.erase(itorFind);

        ChildrenLayerMap::iterator itorFindLayer = m_mapChildrenLayer.find(id);
        OpenSP::sp<Layer> pChildLayer = itorFindLayer->second;
        pChildLayer->removeChildren();
        m_mapChildrenLayer.erase(itorFindLayer);
        pChildLayer->removeParent(this);
        pLayerManager->removeLayerFromCache(id);
    }
    else if(id.ObjectID.m_nType == PARAM_POINT_ID ||
        id.ObjectID.m_nType == PARAM_LINE_ID ||
        id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        // 从虚拟瓦片中删除
        PropertyManager *pPropertyManager = dynamic_cast<PropertyManager *>(pLayerManager->getPropertyManager());
        if (pPropertyManager == NULL)
        {
            return false;
        }

        const PropertyManager::FindTarget eFindTarget = PropertyManager::FT_LocalOnly;
        void *pData = NULL;
        unsigned nLength = 0u;
        pPropertyManager->fetchPropertyData(id, pData, nLength, eFindTarget);
        if(pData && nLength > 0u)
        {
            cmm::math::Sphered sphere;
            if(fetchBoundInfo(pData, nLength, sphere, true))
            {
                vcm::IVirtualCubeManager *pVirtualCubeManager = pLayerManager->getVirtualCubeManager();
                pVirtualCubeManager->removeObject(id, sphere);
            }
            delete[] pData;
        }
    }

    //在状态列表中查找，找到则删除
    for(std::map<std::string, std::set<ID> >::iterator it1 = m_mapDifferents.begin(); it1 != m_mapDifferents.end(); ++it1)
    {
        std::set<ID>    &setHitStateIDs = it1->second;
        setHitStateIDs.erase(id);
    }

    m_bBoundingSphereDirty = true;
    return true;
}


IObject *Layer::getChild(const ID &id)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }

    if(!m_bInitialized)
    {
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    ChildrenList::iterator itorFind = std::find(m_vecChildrenIDs.begin(), m_vecChildrenIDs.end(), id);
    if(itorFind == m_vecChildrenIDs.end())
    {
        return NULL;
    }

    const unsigned nIndex = itorFind - m_vecChildrenIDs.begin();
    return getChild(nIndex);
}


const IObject *Layer::getChild(const ID &id) const
{
    return const_cast<Layer *>(this)->getChild(id);
}


bool Layer::hasChild(const ID &id) const
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }

    if(!m_bInitialized)
    {
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(pLayerManager->isExistInLayer(id, this))
    {
        return true;
    }
    return false;
}


const IObject *Layer::getChild(unsigned nIndex) const
{
    return const_cast<Layer *>(this)->getChild(nIndex);
}


IObject *Layer::getChild(unsigned nIndex)
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return NULL;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(nIndex >= getChildrenCount())
    {
        return NULL;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    const ID &id = m_vecChildrenIDs[nIndex];
    if(id.ObjectID.m_nType == CULTURE_LAYER_ID ||
        id.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID ||
        id.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID)
    {
        return m_mapChildrenLayer[id].get();
    }

    return createObject(nIndex);
}


ID Layer::getChildID(unsigned nIndex)
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return ID();
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    if (nIndex >= getChildrenCount())
    {
        return ID();
    }

    return m_vecChildrenIDs[nIndex];
}


unsigned Layer::getChildrenCount(void) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return 0u;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    return m_vecChildrenIDs.size();
}


unsigned Layer::getChildrenLayerCount(void) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return 0u;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    return m_vecChildrenLayerIDs.size();
}


ILayer* Layer::getChildLayer(unsigned nIndex)
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return NULL;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(nIndex >= getChildrenLayerCount())
    {
        return NULL;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    const ID &id = m_vecChildrenLayerIDs[nIndex];
    return m_mapChildrenLayer[id].get();
}


void Layer::setState(const std::string &strStateName, bool bState)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return;
    }

    setState2(strStateName, bState);

    if(m_bInitialized)
    {
        pLayerManager->refreashObjectState(m_id, strStateName, bState);
    }
}


void Layer::setState2(const std::string &strStateName, bool bState)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    std::map<std::string, bool>::iterator itor = m_mapLayerState.find(strStateName);
    if(itor != m_mapLayerState.end())
    {
        itor->second = bState;
    }
    else
    {
        m_mapLayerState[strStateName] = bState;
    }

    std::map<std::string, std::set<ID> >::iterator itor_Different = m_mapDifferents.find(strStateName);
    if(itor_Different == m_mapDifferents.end())
    {
        m_mapDifferents[strStateName] = std::set<ID>();
    }
    else
    {
        itor_Different->second.clear();
    }
}


bool Layer::getAllInstances(IDList &instances, const std::string &strStateName, bool bState) const
{
    if(!m_bInitialized)
    {
        return false;
    }

    bool bRetValue = true;
    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    const std::set<ID> *pDiffChildrenSet = NULL;
    std::map<std::string, std::set<ID> >::const_iterator itorDiffChildren = m_mapDifferents.find(strStateName);
    if(itorDiffChildren != m_mapDifferents.end())
    {
        pDiffChildrenSet = &itorDiffChildren->second;
    }

    for(std::vector<ID>::const_iterator itor = m_vecChildrenIDs.begin(); itor != m_vecChildrenIDs.end(); ++itor)
    {
        const ID &child_ID = *itor;
        if(child_ID.ObjectID.m_nType == CULTURE_LAYER_ID)
        {
            ChildrenLayerMap::const_iterator itorFind = m_mapChildrenLayer.find(child_ID);
            if(itorFind == m_mapChildrenLayer.end())
            {
                continue;
            }

            Layer *pChildLayer = m_mapChildrenLayer.find(child_ID)->second;
            if(!pChildLayer)
            {
                continue;
            }

            if(!pChildLayer->getAllInstances(instances, strStateName, bState))
            {
                bRetValue = false;
                break;
            }
        }
        else if(child_ID.ObjectID.m_nType == PARAM_POINT_ID ||
            child_ID.ObjectID.m_nType == PARAM_LINE_ID ||
            child_ID.ObjectID.m_nType == PARAM_FACE_ID)
        {
            if(pDiffChildrenSet)
            {
                std::set<ID>::const_iterator itorFind = pDiffChildrenSet->find(child_ID);
                if(itorFind == pDiffChildrenSet->end())
                {
                    instances.push_back(child_ID);
                }
            }
            else
            {
                instances.push_back(child_ID);
            }
        }
    }
    return bRetValue;
}


const cmm::math::Sphered &Layer::getBound(void) const
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return m_BoundingSphere;
    }

    if(!m_bInitialized)
    {
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(!m_bBoundingSphereDirty)
    {
        return m_BoundingSphere;
    }

    PropertyManager *pPropertyManager = dynamic_cast<PropertyManager *>(pLayerManager->getPropertyManager());

    m_BoundingSphere.setRadius(-1.0);
    m_BoundingSphere.setCenter(cmm::math::Point3d(0.0, 0.0, 0.0));

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    for(ChildrenList::const_iterator itorChild = m_vecChildrenIDs.begin(); itorChild != m_vecChildrenIDs.end(); ++itorChild)
    {
        const ID &idChild = *itorChild;
        if(idChild.ObjectID.m_nType == CULTURE_LAYER_ID ||
            idChild.ObjectID.m_nType == TERRAIN_DOM_LAYER_ID ||
            idChild.ObjectID.m_nType == TERRAIN_DEM_LAYER_ID)
        {
            const Layer *pChildLayer = m_mapChildrenLayer.find(idChild)->second.get();
            const cmm::math::Sphered sphere = pChildLayer->getBound();
            m_BoundingSphere.expandBy(sphere);
            continue;
        }

        // 这里下载地物的属性，获取包围球
        const PropertyManager::FindTarget eFindTarget = PropertyManager::FT_LocalOnly;
        void *pData = NULL;
        unsigned nLength = 0u;
        pPropertyManager->fetchPropertyData(idChild, pData, nLength, eFindTarget);
        if(!pData || nLength < 1u)
        {
            continue;
        }

        cmm::math::Sphered sphere;
        if(!fetchBoundInfo(pData, nLength, sphere, false))
        {
            continue;
        }
        delete[] pData;

        m_BoundingSphere.expandBy(sphere);
    }
    m_bBoundingSphereDirty = false;

    return m_BoundingSphere;
}


bool Layer::setChildState(const ID& id, const std::string& strStateName, bool bState)
{
    if(!hasChild(id))
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);

    //根据StateName查找图层的State，未查到则添加
    bool bLayerState = false;
    std::map<std::string, bool>::iterator itor_LayerState = m_mapLayerState.find(strStateName);
    if(itor_LayerState == m_mapLayerState.end())
    {
        m_mapLayerState[strStateName] = false;
    }
    else
    {
        bLayerState = itor_LayerState->second;
    }

    std::map<std::string, std::set<ID> >::iterator itor_DeffrentState = m_mapDifferents.find(strStateName);
    if(itor_DeffrentState == m_mapDifferents.end())
    {
        m_mapDifferents[strStateName] = std::set<ID>();
        itor_DeffrentState = m_mapDifferents.find(strStateName);
    }

    //状态相同，从different列表中删除，否则，加入different列表中
    if(bLayerState == bState)
    {
        std::set<ID>::iterator itor_InstanceID = itor_DeffrentState->second.find(id);
        if(itor_InstanceID != itor_DeffrentState->second.end())
        {
            itor_DeffrentState->second.erase(itor_InstanceID);
        }
    }
    else
    {
        itor_DeffrentState->second.insert(id);
    }

    return true;
}


bool Layer::getChildState(const ID& id, const std::string& strStateName) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return false;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(!hasChild(id))   return false;

    //根据StateName查找图层的State，未查到则添加
    const bool bLayerState = getState(strStateName);

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    std::map<std::string, std::set<ID> >::const_iterator itor_DiffrentState = m_mapDifferents.find(strStateName);
    if(itor_DiffrentState == m_mapDifferents.end())
    {
        return bLayerState;
    }

    if(itor_DiffrentState->second.end() == itor_DiffrentState->second.find(id))
    {
        return bLayerState;
    }

    return !bLayerState;
}


bool Layer::getChildAllState(const ID& idChild, std::map<std::string, bool> &objectstats) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return false;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    if(!hasChild(idChild))   return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    std::map<std::string, bool>::const_iterator i = m_mapLayerState.begin();
    for(; i != m_mapLayerState.end(); ++i)
    {
        objectstats[i->first] = false;
    }

    std::map<std::string, std::set<ID> >::const_iterator j = m_mapDifferents.begin();
    for(; j != m_mapDifferents.end(); ++j)
    {
        objectstats[j->first] = false;
    }

    std::map<std::string, bool>::iterator k = objectstats.begin();
    for(; k != objectstats.end(); ++k)
    {
        k->second = getChildState(idChild, k->first);
    }

    return true;
}


bool Layer::getState(const std::string &strStateName) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return false;
        }
        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    return getState2(strStateName, true);
}


bool Layer::getState2(const std::string &strStateName, bool bInherit) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    std::map<std::string, bool>::const_iterator it = m_mapLayerState.find(strStateName);
    if(it != m_mapLayerState.end())
    {
        return it->second;
    }

    if(!bInherit)   return false;

    std::vector<Layer *>::const_iterator itorParent = m_vecParents.begin();
    for( ; itorParent != m_vecParents.end(); ++itorParent)
    {
        Layer *pParent = *itorParent;
        const bool bState = pParent->getState2(strStateName, true);
        if(bState)
        {
            return true;
        }
    }

    return false;
}


void Layer::getAllState(std::map<std::string, bool> &objectstats) const
{
    if(!m_bInitialized)
    {
        OpenSP::sp<LayerManager>    pLayerManager;
        if(!ms_pLayerManager.lock(pLayerManager))
        {
            return;
        }

        pLayerManager->getInitializationThreadPool()->raisePriority(this);
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(m_mtxChildren);
    objectstats.insert(m_mapLayerState.begin(), m_mapLayerState.end());
}

}