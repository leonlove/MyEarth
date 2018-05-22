#include "VCubeChangingListener.h"
#include "VCubeChanged_Operation.h"
#include <assert.h>

VCubeChangingListener::VCubeChangingListener(void)
{
    setStackSize(64u * 1024u);
    m_blockDelay.reset();
}


VCubeChangingListener::~VCubeChangingListener(void)
{
    m_pVCubeManager         = NULL;
    m_pVCubeReader          = NULL;
    m_pSceneGraphOperator   = NULL;
}


bool VCubeChangingListener::initialize(vcm::IVirtualCubeManager *pVCubeManager, FileReadInterceptor *pVCubeReader, SceneGraphOperator *pOperator)
{
    if(!pVCubeManager)
    {
        assert(false);
        return false;
    }
    if(!pVCubeReader)
    {
        assert(false);
        return false;
    }
    if(!pOperator)
    {
        assert(false);
        return false;
    }

    m_pVCubeManager       = pVCubeManager;
    m_pVCubeReader        = pVCubeReader;
    m_pSceneGraphOperator = pOperator;
    return true;
}


void VCubeChangingListener::finishMission(void)
{
    m_blockDelay.release();
    join();
}


void VCubeChangingListener::run(void)
{
    if(!m_pVCubeManager.valid())
    {
        assert(false);
        return;
    }
    if(!m_pVCubeReader.valid())
    {
        assert(false);
        return;
    }
    if(!m_pSceneGraphOperator.valid())
    {
        assert(false);
        return;
    }

    while(true)
    {
        m_blockDelay.block(100u);

        IDList listChangedVCube;
        if(!m_pVCubeManager->takeChangedVCube(listChangedVCube))
        {
            break;
        }

        if(listChangedVCube.empty())
        {
            continue;
        }

        VCubeChanged_Operation *pOperation = new VCubeChanged_Operation(m_pVCubeManager->getCubeFragmentCount());

        for(IDList::const_iterator itor = listChangedVCube.begin(); itor != listChangedVCube.end(); ++itor)
        {
            const ID &id = *itor;
            OpenSP::sp<vcm::IVirtualCube> pLocalVCube = m_pVCubeManager->copyVirtualCube(id);
            OpenSP::sp<vcm::IVirtualCube> pRemoteVCube = m_pVCubeReader->readRemoteVirtualCubeByID(id);

            OpenSP::sp<vcm::IVirtualCube> pVCube = pRemoteVCube;
            if(pLocalVCube.valid())
            {
                pVCube = pLocalVCube;
                if(pRemoteVCube.valid())
                {
                    pVCube->mergeVCube(pRemoteVCube.get());
                }
            }

            pOperation->addCube(id, pVCube.get());
        }

        m_pSceneGraphOperator->pushOperation(pOperation);
    }
}
