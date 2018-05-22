#ifndef LAYER_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE
#define LAYER_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE

#include "ILayerManager.h"
#include "Layer.h"
#include "TerrainInstance.h"
#include <algorithm>
#include <OpenThreads/Mutex>
#include <OpenSP/op.h>
#include <Common/IStateImplementer.h>
#include "../VirtualTileManager/IVirtualCubeManager.h"
#include "IDProvider/Definer.h"
#include "InitializationThread.h"
#include "StateRefreshingThread.h"

namespace logical
{

class LayerManager : public ILayerManager
{
public:
    explicit LayerManager(void);
    virtual ~LayerManager(void);

public:     // methods from ILayerManager
    virtual bool        login(const std::string& strAuthHost, const std::string& strAuthPort, const std::string& strUserName, const std::string& strUserPwd);
    virtual bool        logout();
    virtual bool        initialize(const std::string &strHost, const std::string &strPort, const std::string &strLocalCache);
    virtual void        unInitialize(void);
    virtual ILayer     *getCultureRootLayer(void)   {   return m_pCultureRootLayer.get();   }
    virtual ILayer     *getTerrainDEMRootLayer(void){   return m_pTerrainDEMRootLayer.get();}
    virtual ILayer     *getTerrainDOMRootLayer(void){   return m_pTerrainDOMRootLayer.get();}

    virtual IObject    *findObject(const ID &id);

    virtual IPropertyManager *getPropertyManager(void)              {   return m_pPropertyManager.get();    }
    virtual const IPropertyManager *getPropertyManager(void) const  {   return m_pPropertyManager.get();    }

public:
    virtual cmm::StateValue     queryObjectState(const ID &id, const std::string &strStateName) const;
    virtual void                getObjectStates(const ID &id, std::map<std::string, bool> &objectstates) const;
    virtual void                setStateImplementer(cmm::IStateImplementer *pStateImplementer);
    virtual vcm::IVirtualCubeManager *getVirtualCubeManager()       {   return m_pLocalVCubeManager;    }

protected:
    void    getParents(const ID &id, IDList &listParents) const;
    virtual IObject *findObject(const ID &id, ILayer *pLayer, bool &bFound);

public:
    void pushIntoRefreshingQueue(const ID &id);
    void refreashObjectState(const ID &id, const std::string &strStateName, bool eState);

    void addLayerToCache(const ID &id, Layer *pLayer);
    void removeLayerFromCache(const ID &id);
    Layer *findLayerInCache(const ID &id);

    struct CacheItem
    {
        ID              m_id;
        const Layer    *m_pParent;

        CacheItem(void) : m_pParent(NULL){}
        CacheItem(const CacheItem &item) : m_id(item.m_id), m_pParent(item.m_pParent){}
        CacheItem(const ID &id, const Layer *pLayer) : m_id(id), m_pParent(pLayer){}
        const CacheItem &operator=(const CacheItem &item)
        {
            if(this == &item)   return *this;
            m_id = item.m_id;
            m_pParent = item.m_pParent;
            return *this;
        }
        bool operator<(const CacheItem &item) const
        {
            return m_id < item.m_id;
        }
    };

    struct LowerBound_CacheItem
    {
        bool operator()(const CacheItem &itor, const ID &id) const
        {
            return itor.m_id < id;
        }
    };

    void addObjectToCache(const ID &id, const Layer *pLayer);
    void addObjectsToCache(const std::vector<CacheItem> &vecCacheItems);
    void removeObjectFromCache(const ID &id, const Layer *pLayer);
    void findParentsOfObject(const ID &id, std::vector<const Layer *> &vecParents) const;
    bool isExistInLayer(const ID &id, const Layer *pLayer) const;

    void fastInsert(std::vector<CacheItem>::iterator itorPos, const CacheItem &item);
    void fastInsert_back(const std::vector<CacheItem> &vecItems);

    InitializationThreadPool *getInitializationThreadPool(void) {   return m_pInitThreadPool.get(); }

protected:
    OpenSP::sp<PropertyManager>     m_pPropertyManager;

    OpenSP::sp<Layer>               m_pCultureRootLayer;
    OpenSP::sp<Layer>               m_pTerrainDEMRootLayer;
    OpenSP::sp<Layer>               m_pTerrainDOMRootLayer;

    // –Èƒ‚Õﬂ∆¨π‹¿Ì∆˜
    OpenSP::sp<vcm::IVirtualCubeManager>    m_pLocalVCubeManager;

    mutable OpenThreads::Mutex              m_mtxObjectCache;
    std::vector<CacheItem>                  m_vecObjectCache;

    OpenThreads::Mutex                      m_mtxLayerCache;
    std::map<ID, Layer *>                   m_mapLayerCache;

    OpenSP::sp<InitializationThreadPool>    m_pInitThreadPool;
    OpenSP::sp<StateRefreshingThread>       m_pStateRefreshingThread;
    bool                            m_bInitialized;
};

}

#endif

