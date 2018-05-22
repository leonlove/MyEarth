#ifndef VTILE_CHANGING_LISTENER_H_5C7D43B3_7924_4836_8D3E_0CA409D8F003_INCLUDE
#define VTILE_CHANGING_LISTENER_H_5C7D43B3_7924_4836_8D3E_0CA409D8F003_INCLUDE

#include <OpenThreads/Thread>
#include <OpenSP/sp.h>
#include <VirtualTileManager/IVirtualCubeManager.h>
#include "FileReadInterceptor.h"
#include "SceneGraphOperator.h"

class VCubeChangingListener : public OpenThreads::Thread, public OpenSP::Ref
{
public:
    explicit VCubeChangingListener(void);
    virtual ~VCubeChangingListener(void);

public:
    bool    initialize(vcm::IVirtualCubeManager *pVCubeManager, FileReadInterceptor *pVTileReader, SceneGraphOperator *pOperator);
    void    finishMission(void);

protected:
    virtual void run(void);

protected:
    OpenSP::sp<vcm::IVirtualCubeManager>    m_pVCubeManager;
    OpenSP::sp<FileReadInterceptor>         m_pVCubeReader;
    OpenSP::sp<SceneGraphOperator>          m_pSceneGraphOperator;
    OpenThreads::Block                      m_blockDelay;
};

#endif


