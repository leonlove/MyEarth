#ifndef VIRTUAL_CUBE_NODE_H_B2E31D0B_1353_4340_87AB_C948ADEEE1E5_INCLUDE
#define VIRTUAL_CUBE_NODE_H_B2E31D0B_1353_4340_87AB_C948ADEEE1E5_INCLUDE

#include <osg/Group>
#include <osg/observer_ptr>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <map>
#include <algorithm>
#include <Common/IStateQuerier.h>
#include <Common/Pyramid.h>
#include <VirtualTileManager/IVirtualCubeManager.h>

typedef std::vector<IDList>         ObjectRow;
typedef std::vector<ObjectRow>      ObjectMatrix;
typedef std::vector<ObjectMatrix>   ObjectCube;

class VCubeFragmentNode : public osg::Group
{
public:
    VCubeFragmentNode(void);

    VCubeFragmentNode(const VCubeFragmentNode &proxynode, const osg::CopyOp &copyop);

    META_Node(osg, VCubeFragmentNode);

    virtual void                traverse(osg::NodeVisitor &nv);
    virtual bool                addChild(osg::Node *pChild);

    void                        setDatabaseOptions(osg::Referenced *options)    {   m_pDatabaseOptions = options;       }
    osg::Referenced            *getDatabaseOptions(void)                        {   return m_pDatabaseOptions.get();    }
    const osg::Referenced      *getDatabaseOptions(void) const                  {   return m_pDatabaseOptions.get();    }

    bool                        setVisible(const ID &id, bool bVisible);
    void                        setVisible(const std::set<ID> &listIDs, bool bVisible);

    bool                        addChildObject(const ID &id);
    bool                        removeChildObject(const ID &id);
    unsigned                    getChildObjectCount(void) const                  {   return m_vecChildrenInfo.size();    }

    bool                        getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const;
    void                        getObjects(IDList &listIDs) const;

public:
    void getInstances(IDList &vecInstances) const
    {
        //OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxChildren);
        vecInstances.reserve(m_vecChildrenInfo.size());
        for(std::vector<TileChildInfo>::const_iterator itor = m_vecChildrenInfo.begin(); itor != m_vecChildrenInfo.end(); ++itor)
        {
            const TileChildInfo &info = *itor;
            vecInstances.push_back(info.m_id);
        }
    }

    static void setStateQuerier(cmm::IStateQuerier *pStateQuerier)
    {
        ms_pStateQuerier = pStateQuerier;
        //m_pQueryThread->setStateQuerier(pStateQuerier);
    }

    static void setLoadingCountPerFrame(unsigned nCount)
    {
        ms_nLoadingCountPerFrame = osg::clampAbove(nCount, 1u);
    }

    static unsigned getReadCountPerFrame(void)
    {
        return ms_nLoadingCountPerFrame;
    }

protected:
    virtual                    ~VCubeFragmentNode(void);

protected:
    osg::ref_ptr<osg::Referenced>       m_pDatabaseOptions;
    unsigned                            m_nNextRequestChild;

    typedef struct TileChildInfo
    {
        ID                                  m_id;
        osg::observer_ptr<osg::Node>        m_pChild;
        cmm::StateValue                     m_eVisible;
        osg::ref_ptr<osg::Referenced>       m_pFileRequest;

        const TileChildInfo &operator=(const TileChildInfo &info)
        {
            if(this == &info)   return *this;
            m_id           = info.m_id;
            m_pChild       = info.m_pChild;
            m_eVisible     = info.m_eVisible;
            m_pFileRequest = m_pFileRequest;
            return *this;
        }
    };

    struct ChildFinder
    {
        const ID        m_id;
        explicit ChildFinder(const ID &id) : m_id(id){}
        bool operator()(const TileChildInfo &child) const
        {
            return m_id == child.m_id;
        }
    };

    std::vector<TileChildInfo>         m_vecChildrenInfo;

protected:
    static OpenSP::op<cmm::IStateQuerier>                   ms_pStateQuerier;
    static unsigned                                         ms_nLoadingCountPerFrame;
};

class LODFixer;
class VCubeFragmentGroup : public osg::Group
{
public:
    VCubeFragmentGroup(void);
    VCubeFragmentGroup(const VCubeFragmentGroup &fragmentGroup, const osg::CopyOp &copyop) : m_dblRadius(-1.0){}
    virtual ~VCubeFragmentGroup(void);
    META_Node(osg, VCubeFragmentGroup);

public:
    virtual osg::BoundingSphere computeBound(void) const;

public:
    bool            create(const vcm::IVirtualCubeManager *pVCManager, const ID &id, const ObjectCube &cubeObjectIDs, LODFixer *pLODFixer);

    inline void     setCenter(const osg::Vec3d& center) {   m_ptCenter = center;    }
    inline void     setRadius(double radius)            {   m_dblRadius = radius;   }

    bool            setVisible(const ID &id, bool bVisible);
    bool            getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const;
    void            getObjects(IDList &listIDs) const;

    VCubeFragmentNode   *getFragmentNode(unsigned nFragmentCount, unsigned nRow, unsigned nCol, unsigned nHeight);

protected:
    osg::Vec3d      m_ptCenter;
    double          m_dblRadius;
    unsigned        m_nFragmentCount;

};


#endif
