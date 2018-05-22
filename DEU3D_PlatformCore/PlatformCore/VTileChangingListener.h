#ifndef VTILE_CHANGING_LISTENER_H_5C7D43B3_7924_4836_8D3E_0CA409D8F003_INCLUDE
#define VTILE_CHANGING_LISTENER_H_5C7D43B3_7924_4836_8D3E_0CA409D8F003_INCLUDE

#include <OpenThreads/Thread>
#include <OpenSP/sp.h>
#include <VirtualTileManager/IVirtualTileManager.h>
#include "FileReadInterceptor.h"
#include "SceneGraphOperator.h"

class VTileChangingListener : public OpenThreads::Thread, public OpenSP::Ref
{
public:
    explicit VTileChangingListener(void);
    virtual ~VTileChangingListener(void);

public:
    bool    initialize(vtm::IVirtualTileManager *pVTileManager, FileReadInterceptor *pVTileReader, SceneGraphOperator *pOperator);
    void    finishMission(void);

protected:
    virtual void run(void);

protected:
    OpenSP::sp<vtm::IVirtualTileManager>    m_pVTileManager;
    OpenSP::sp<FileReadInterceptor>         m_pVTileReader;
    OpenSP::sp<SceneGraphOperator>          m_pSceneGraphOperator;
    OpenThreads::Block                      m_blockDelay;
};

#endif


