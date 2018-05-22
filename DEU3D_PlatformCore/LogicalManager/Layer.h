#ifndef LOGICAL_LAYER_H_4E99214C_945B_460E_A271_1F00558D25CD_INCLUDE
#define LOGICAL_LAYER_H_4E99214C_945B_460E_A271_1F00558D25CD_INCLUDE

#include "Object.h"
#include "ILayer.h"

#include <OpenSP/sp.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/Thread>
#include <vector>
#include <set>
#include <map>
#include <Network/IDEUNetwork.h>
#include <Common/DEUBson.h>
#include <common/IStateQuerier.h>

namespace logical
{

class Layer : public Object, virtual public ILayer
{
public:
    explicit Layer(const ID &id);
    virtual ~Layer(void);

public: // methods from ILayer
    virtual ILayer         *createSubLayer(void);

    virtual bool            addChild(const ID &id);
    virtual bool            removeChild(const ID &id);
    virtual unsigned        getChildrenCount(void) const;

    virtual const IObject  *getChild(const ID &id) const;
    virtual IObject        *getChild(const ID &id);
    virtual const IObject  *getChild(unsigned nIndex) const;
    virtual IObject        *getChild(unsigned nIndex);

    virtual ID              getChildID(unsigned nIndex);
    virtual unsigned        getChildrenLayerCount(void) const;
    virtual ILayer         *getChildLayer(unsigned nIndex);

    virtual bool            hasChild(const ID &id) const;

    virtual ILayer         *asLayer(void)    {return this;}
    virtual bool            isDirty(void) const {return m_bBoundingSphereDirty;}

    virtual void            setState(const std::string &strStateName, bool bState);
    virtual bool            getState(const std::string &strStateName) const;
    virtual void            getAllState(std::map<std::string, bool> &objectstats) const;
    virtual bool            getChildAllState(const ID& idChild, std::map<std::string, bool> &objectstats) const;

    virtual const           cmm::math::Sphered &getBound(void) const;

protected:
    friend  class   InitializationThread;
    friend  class   LayerManager;

    friend  class   InitializationThread;
    virtual bool    init(void);
    virtual bool    fromBson(const bson::bsonDocument &bsonDoc);
    virtual void    setState2(const std::string &strStateName, bool bState);
    virtual bool    getState2(const std::string &strStateName, bool bInherit) const;

public:
    bool    getAllInstances(IDList &instances, const std::string &strStateName, bool bState) const;
    bool    addChild2(const ID &id);
    bool    removeChild2(const ID &id);
    bool    setChildState(const ID& id, const std::string& strStateName, bool bState);
    bool    getChildState(const ID& id, const std::string& strStateName) const;

protected:
    Object   *createObject(unsigned nIndex);
    void      removeChildren(void);

protected:

    typedef std::vector<ID>                     ChildrenList;
    typedef std::vector<ID>                     ChildrenLayerList;
    typedef std::map<ID, OpenSP::sp<Layer> >    ChildrenLayerMap;

    ChildrenList                m_vecChildrenIDs;
    ChildrenLayerList           m_vecChildrenLayerIDs;
    ChildrenLayerMap            m_mapChildrenLayer;
    mutable OpenThreads::Mutex  m_mtxChildren;

    std::map<std::string, bool> m_mapLayerState;

    mutable bool                m_bBoundingSphereDirty;
    std::map<std::string, std::set<ID> > m_mapDifferents;

    bool    m_bInitialized;
};

}

#endif
