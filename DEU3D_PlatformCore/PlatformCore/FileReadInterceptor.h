#ifndef FILE_READ_INTERCEPTOR_H_20E33712_3100_4974_B24D_DE3ED656109D_INCLUDE
#define FILE_READ_INTERCEPTOR_H_20E33712_3100_4974_B24D_DE3ED656109D_INCLUDE

#include <map>
#include <EventAdapter/IEventReceiver.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>
#include <OpenThreads/Mutex>
#include <EventAdapter/IEventAdapter.h>
#include <Common/Common.h>
#include <Common/Pyramid.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <OpenThreads/Block>
#include <osgDB/Callbacks>
#include <algorithm>

#include <Network/IDEUNetwork.h>

#include "StateManager.h"
#include "TextCenterLayouter.h"
#include "VirtualCubeReaderWriter.h"
#include "TerrainModificationManager.h"
#include "SharedObjectPool.h"

class FileReadInterceptor : public osgDB::ReadFileCallback
{
public:
    explicit FileReadInterceptor(void);
protected:
    virtual ~FileReadInterceptor(void);

public:
    bool    login(const std::string& strAuthHost, const std::string& strAuthPort, const std::string& strUserName, const std::string& strUserPwd);
    bool    logout();
    bool    initialize(bool bBifurcateThread, const std::string &strHost, const std::string &strPort, const std::string &strLocalCache, cmm::IStateQuerier *pStateQuerier);

    void    setTerrainLayersOrder(bool bDEM, const IDList &vecTerrainOrder);
    void    getTerrainLayersOrder(bool bDEM, IDList &vecTerrainOrder) const;

    bool    addLocalDatabase(const std::string &strDB);
    bool    removeLocalDatabase(const std::string &strDB);

    void    setStateManager(StateManager *pStateManager)
    {
        m_pStateManager = pStateManager;
    }

    void    setTerrainModificationManager(TerrainModificationManager *pTerrainModificationManager)
    {
        m_pTerrainModificationManager = pTerrainModificationManager;
    }
    ITerrainModificationManager *getTerrainModificationManager(void)
    {
        return m_pTerrainModificationManager.get();
    }
    const ITerrainModificationManager *getTerrainModificationManager(void) const
    {
        return m_pTerrainModificationManager.get();
    }

    osgDB::ReaderWriter::ReadResult readSimpleTileByID(const ID &id, const osgDB::Options *pOptions) const;

    vcm::IVirtualCube *readRemoteVirtualCubeByID(const ID &id) const;

    unsigned int getLastTerrainUpdate(void) const { return (unsigned)m_TerrainUpdate; }
    bool    addWMTSTileSet(deues::ITileSet *pTileSet);
    bool    removeWMTSTileSet(deues::ITileSet *pTileSet);

protected:  // Method from osgDB::ReadFileCallback
    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readNode(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readImage(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readImage(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);

protected:
    osgDB::ReaderWriter::ReadResult        readParamByID(const ID &id, const osgDB::Options *pOptions) const;
    osgDB::ReaderWriter::ReadResult        readDetailByID(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo) const;
    osgDB::ReaderWriter::ReadResult        readImageByID(const ID &id, const osgDB::Options *pOptions) const;
    osgDB::ReaderWriter::ReadResult        readModelByID(const ID &id, const osgDB::Options *pOptions) const;
    osgDB::ReaderWriter::ReadResult        readVirtualCubeByID(const ID &id, const osgDB::Options *pOptions) const;
    osgDB::ReaderWriter::ReadResult        readTerrainTileByID(const ID &id, const osgDB::Options *pOptions) const;
    osgDB::ReaderWriter::ReadResult        readActualTileByID(const ID &id, const osgDB::Options *pOptions) const;

    osgDB::ReaderWriter::ReadResult        readTileFragmentByActualID(const ID &id, const osgDB::Options *pOptions) const;

    osgDB::ReaderWriter::ReadResult        readDEMTileLayerByID(const ID &id, const osgDB::Options *pOptions) const;

    void readDom(const ID &id, std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > > &vecTexture, const osgDB::Options *pOptions) const;
    osg::Texture2D *readDomImage(const ID &id) const;

protected:
    class FetchThread : public OpenThreads::Thread, public OpenSP::Ref
    {
    public:
        typedef osgDB::ReaderWriter::ReadResult (FileReadInterceptor::*FetchFunctor)(const ID &id, const osgDB::Options *pOptions) const;
        explicit FetchThread(const FileReadInterceptor *pInterceptor, const FetchFunctor pFetchFunctor) :
        m_pInterceptor(pInterceptor),
            m_pFetchFunctor(pFetchFunctor)
        {
            setStackSize(65536u);
        }
        virtual ~FetchThread(void)
        {
        }

    public:
        void setID(const ID &id)    {    m_ID = id;    }
        void setOptions(const osgDB::Options *pOptions)    {    m_pOptions = pOptions;    }

        osgDB::ReaderWriter::ReadResult &getFetchResult(void)
        {
            return m_FetchResult;
        }
        const osgDB::ReaderWriter::ReadResult &getFetchResult(void) const
        {
            return m_FetchResult;
        }

    protected:
        virtual void run(void)
        {
            if(m_pFetchFunctor)
            {
                m_FetchResult = (m_pInterceptor->*m_pFetchFunctor)(m_ID, m_pOptions);
            }
        }

    protected:
        const FileReadInterceptor          *m_pInterceptor;
        FetchFunctor                        m_pFetchFunctor;
        ID                                  m_ID;
        const osgDB::Options               *m_pOptions;
        osgDB::ReaderWriter::ReadResult     m_FetchResult;
    };

    struct LocalDB
    {
        OpenSP::sp<deudbProxy::IDEUDBProxy>       m_pDeuDB;
        std::string         m_strDBFile;

        const LocalDB &operator=(const LocalDB &db)
        {
            if(this == &db) return *this;
            m_pDeuDB      = db.m_pDeuDB;
            m_strDBFile   = db.m_strDBFile;
            return *this;
        }
        LocalDB(void) : m_pDeuDB(NULL){}
        LocalDB(deudbProxy::IDEUDBProxy *pDeuDB, const std::string &strDBFile)
            : m_pDeuDB(pDeuDB), m_strDBFile(strDBFile)
        { }
        LocalDB(const LocalDB &db)
        {
            operator=(db);
        }
    };
    class LocalDBFinder
    {
    public:
        LocalDBFinder(const std::string &strTarget)
        {
            m_strFindingTarget = strTarget;
        }

        bool operator()(const LocalDB &db) const
        {
            if(db.m_strDBFile == m_strFindingTarget)
            {
                return true;
            }
            return false;
        }

        bool operator()(const std::pair<unsigned, std::vector<LocalDB> > &db) const
        {
            const std::vector<LocalDB> &vecDBs = db.second;
            std::vector<LocalDB>::const_iterator itorDB = vecDBs.begin();
            for( ; itorDB != vecDBs.end(); ++itorDB)
            {
                const LocalDB &db = *itorDB;
                if(db.m_strDBFile == m_strFindingTarget)
                {
                    return true;
                }
            }
            return false;
        }

    protected:
        std::string     m_strFindingTarget;
    };

protected:
    bool        findNearestIDbyID(const ID &id, bool &bIsBottomTile, ID &nearest_id)const;
    bool        analysisTerrainTileInfo(void);
    void        waitForRequestFinish(void);
    bool        readFromLocalDB(const ID &id, void *&pBuffer, unsigned &nLength) const;
    osg::Image *parseImageFromStream(const void *pBuffer, unsigned nLength, const osgDB::Options *pOptions) const;

    typedef std::map<ID, unsigned>          TerrainTilesInfo;
    bool        fetchTerrainInfo(const ID &idTerrain, TerrainTilesInfo &terrainInfo) const;
    bool        parseTerrainInfo(const void *pBsonData, unsigned nLength, TerrainTilesInfo &terrainInfo) const;

protected:
    struct MissionIncreaser
    {
        MissionIncreaser(OpenThreads::Atomic &status) : m_status(status)
        {
            ++m_status;
        }
        ~MissionIncreaser(void)
        {
            --m_status;
        }
        OpenThreads::Atomic    &m_status;
    };

    osg::ref_ptr<VirtualCubeReaderWriter>   m_pVirtualCubeReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pIveReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pJpgReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pPngReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pBmpReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pTifReaderWriter;
    osg::ref_ptr<osgDB::ReaderWriter>       m_pDdsReaderWriter;

    bool                                    m_bBifurcateThread;

    typedef struct
    {
        unsigned            m_nDatasetCode;
        unsigned __int64    m_nUniqueID;
    }LayerItem;

    typedef struct
    {
        ID              m_id;
        LayerItem       m_LayerItem;
    }TerrainOrderItem;

    struct TerrainItemFlagFetcher
    {
        const LayerItem &operator()(const TerrainOrderItem &item) const
        {
            return item.m_LayerItem;
        }
    };
    std::vector<TerrainOrderItem>           m_vecDemCoverOrder;
    mutable OpenThreads::Mutex              m_mtxDemCoverOrder;
    std::map<unsigned __int64, TerrainTilesInfo>    m_mapDemTopTiles;
    mutable OpenThreads::Mutex              m_mtxDemTopTiles;

    std::vector<TerrainOrderItem>           m_vecDomCoverOrder;
    mutable OpenThreads::Mutex              m_mtxDomCoverOrder;
    std::map<unsigned __int64, TerrainTilesInfo>    m_mapDomTopTiles;
    mutable OpenThreads::Mutex              m_mtxDomTopTiles;


    OpenSP::sp<deunw::IDEUNetwork>          m_pDEUNetwork;
    std::set<std::string>                   m_setDisplay;

    OpenThreads::Atomic                     m_MissionStatus;

    osg::ref_ptr<TextCenterLayouter>        m_pTextCenterLayouter;

    typedef std::map<unsigned, std::vector<LocalDB> >     LocalDBMap;
    LocalDBMap                              m_mapLocalDBs;
    mutable OpenThreads::Mutex              m_mtxLocalDBs;

    OpenSP::sp<deudbProxy::IDEUDBProxy>     m_pLocalTempDB;

    //状态管理器
    OpenSP::op<cmm::IStateQuerier>          m_pStateQuerier;
    OpenSP::sp<IStateManager>               m_pStateManager;

    //
    OpenSP::sp<TerrainModificationManager>  m_pTerrainModificationManager;

    //记录地形更新时间
    mutable OpenThreads::Atomic             m_TerrainUpdate;

    std::map<ID, OpenSP::sp<deues::ITileSet> >  m_mapWMTSTileSet;

    OpenSP::sp<SharedObjectPool>             m_pSharedObjectPool;
};

#endif
