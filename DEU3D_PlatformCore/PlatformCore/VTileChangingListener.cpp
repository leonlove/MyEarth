#include "VTileChangingListener.h"
#include "VTileChanged_Operation.h"
#include <assert.h>

VTileChangingListener::VTileChangingListener(void)
{
    setStackSize(64u * 1024u);
    m_blockDelay.reset();
}


VTileChangingListener::~VTileChangingListener(void)
{
    m_pVTileManager         = NULL;
    m_pVTileReader          = NULL;
    m_pSceneGraphOperator   = NULL;
}


bool VTileChangingListener::initialize(vtm::IVirtualTileManager *pVTileManager, FileReadInterceptor *pVTileReader, SceneGraphOperator *pOperator)
{
    if(!pVTileManager)
    {
        assert(false);
        return false;
    }
    if(!pVTileReader)
    {
        assert(false);
        return false;
    }
    if(!pOperator)
    {
        assert(false);
        return false;
    }

    m_pVTileManager       = pVTileManager;
    m_pVTileReader        = pVTileReader;
    m_pSceneGraphOperator = pOperator;
    return true;
}


void VTileChangingListener::finishMission(void)
{
    m_blockDelay.release();
    join();
}


void VTileChangingListener::run(void)
{
#if 0
    if(!m_pVTileManager.valid())
    {
        assert(false);
        return;
    }
    if(!m_pVTileReader.valid())
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

        IDList listChangedVTile;
        if(!m_pVTileManager->takeChangedVTile(listChangedVTile))
        {
            break;
        }

        if(listChangedVTile.empty())
        {
            continue;
        }

        VTileChanged_Operation *pOperation = new VTileChanged_Operation(m_pVTileManager->getTileFragmentCount());

        for(IDList::const_iterator itor = listChangedVTile.begin(); itor != listChangedVTile.end(); ++itor)
        {
            const ID &id = *itor;
            OpenSP::sp<vtm::IVirtualTile> pLocalVTile = m_pVTileManager->copyVirtualTile(id);
            OpenSP::sp<vtm::IVirtualTile> pRemoteVTile = m_pVTileReader->readRemoteVirtualTileByID(id);

            OpenSP::sp<vtm::IVirtualTile> pVTile = pRemoteVTile;
            if(pLocalVTile.valid())
            {
                pVTile = pLocalVTile;
                if(pRemoteVTile.valid())
                {
                    pVTile->mergeVTile(pRemoteVTile.get());
                }
            }

            pOperation->addTile(id, pVTile.get());
        }

        m_pSceneGraphOperator->pushOperation(pOperation);
    }
#endif
}
