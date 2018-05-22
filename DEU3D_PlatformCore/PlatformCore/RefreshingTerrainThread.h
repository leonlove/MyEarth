#ifndef REFRESHING_TERRAIN_THREAD_H_9DDC8AC8_C6A4_4B65_8B35_1A1373AF365D_INCLUDE
#define REFRESHING_TERRAIN_THREAD_H_9DDC8AC8_C6A4_4B65_8B35_1A1373AF365D_INCLUDE

#include <osg/Node>
#include <osgTerrain/TerrainTile>
#include <vector>
#include "RefreshingThread.h"
#include "FileReadInterceptor.h"
#include "SceneGraphOperator.h"

class RefreshingTerrainThread : public RefreshingThread
{
public:
    explicit RefreshingTerrainThread(void);
    virtual ~RefreshingTerrainThread(void);

public:
    void initialize(osgViewer::ViewerBase *pViewer, osg::Node *pTerrainNode, const FileReadInterceptor *pTileReader, SceneGraphOperator *pSceneGraphOperator);
    void doRefresh(void);

protected:
    void    dropWork(void);

protected:
    virtual void run(void);

protected:

protected:
    osg::ref_ptr<const FileReadInterceptor> m_pTileReader;
    osg::ref_ptr<osg::Node>                 m_pTerrainNode;
    OpenThreads::Atomic                     m_bDropped;
    osg::ref_ptr<SceneGraphOperator>        m_pSceneGraphOperator;
};


#endif

