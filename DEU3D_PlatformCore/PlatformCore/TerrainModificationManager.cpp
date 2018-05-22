#include "TerrainModificationManager.h"
#include <algorithm>
#include <IDProvider/Definer.h>
#include "TerrainColorModification.h"
#include "TerrainElevationModification.h"
#include "TerrainDomModification.h"
#include "TerrainDemModification.h"
#include "Utility.h"
#include <iostream>
#include <osgShadow/SoftShadowMap>

#include "Registry.h"

TerrainModificationManager::TerrainModificationManager(ea::IEventAdapter *pEventAdapter) : m_pEventAdapter(pEventAdapter)
{
}


TerrainModificationManager::~TerrainModificationManager(void)
{
}


ITerrainModification *TerrainModificationManager::createModification(const std::string &strType)
{
    OpenSP::sp<TerrainModification> pModification = NULL;
    if(strType.compare(TMT_DEM_MODIFICATION) == 0)
    {
        pModification = new TerrainDemModification(strType, m_pEventAdapter.get());
    }
    else if(strType.compare(TMT_DOM_MODIFICATION) == 0)
    {
        pModification = new TerrainDomModification(strType, m_pEventAdapter.get());
    }
    else if(strType.compare(TMT_COLOR_MODIFICATION) == 0)
    {
        pModification = new TerrainColorModification(strType, m_pEventAdapter.get());
    }
    else if(strType.compare(TMT_ELEVATION_MODIFICATION) == 0)
    {
        pModification = new TerrainElevationModification(strType, m_pEventAdapter.get());
    }

    if(pModification.valid())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);
        m_vecTerrainModifications.push_back(pModification);
    }

    return pModification.release();
}


bool TerrainModificationManager::removeModification(unsigned nIndex)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    if(nIndex >= m_vecTerrainModifications.size())
    {
        return false;
    }

    m_vecTerrainModifications[nIndex]->setApply(false);
    m_vecTerrainModifications.erase(m_vecTerrainModifications.begin() + nIndex);
    return true;
}


bool TerrainModificationManager::removeModification(ITerrainModification *pModification)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    std::vector<OpenSP::sp<TerrainModification> >::iterator itor = m_vecTerrainModifications.begin();
    for(; itor != m_vecTerrainModifications.end(); ++itor)
    {
        TerrainModification *pFind = itor->get();
        if(pModification == pFind)
        {
            pFind->setApply(false);
            m_vecTerrainModifications.erase(itor);
            return true;
        }
    }
    return false;
}


unsigned TerrainModificationManager::getModificationCount(void) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);
    return m_vecTerrainModifications.size();
}


ITerrainModification *TerrainModificationManager::getModification(unsigned nIndex)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);
    if(nIndex >= getModificationCount())
    {
        return NULL;
    }

    return m_vecTerrainModifications[nIndex].get();
}


const ITerrainModification *TerrainModificationManager::getModification(unsigned nIndex) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);
    if(nIndex >= getModificationCount())
    {
        return NULL;
    }

    return m_vecTerrainModifications[nIndex].get();
}


ITerrainModification *TerrainModificationManager::findModificationByName(const std::string &strName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    std::vector<OpenSP::sp<TerrainModification> >::iterator itor = m_vecTerrainModifications.begin();
    for(; itor != m_vecTerrainModifications.end(); ++itor)
    {
        TerrainModification *pModification = itor->get();
        if(strName == pModification->getName())
        {
            return pModification;
        }
    }
    return NULL;
}


const ITerrainModification *TerrainModificationManager::findModificationByName(const std::string &strName) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    std::vector<OpenSP::sp<TerrainModification> >::const_iterator itor = m_vecTerrainModifications.begin();
    for(; itor != m_vecTerrainModifications.end(); ++itor)
    {
        TerrainModification *pModification = itor->get();
        if(strName == pModification->getName())
        {
            return pModification;
        }
    }
    return NULL;
}


bool TerrainModificationManager::modifyTerrainTile(osg::Node *pTerrainTile) const
{
    if(pTerrainTile == NULL)
    {
        return false;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    std::vector<OpenSP::sp<TerrainModification> >::const_iterator itor = m_vecTerrainModifications.begin();
    for(; itor != m_vecTerrainModifications.end(); ++itor)
    {
        TerrainModification *pTerrainModification = dynamic_cast<TerrainModification *>(itor->get());
        pTerrainModification->modifyTerrainTile(pTerrainTile);
    }
    return true;
}


bool TerrainModificationManager::modifyTerrainTile(osg::Node *pTerrainTile, bool bModifyTexture) const
{
    if(pTerrainTile == NULL)
    {
        return false;
    }

    osgTerrain::TerrainTile *pTile = dynamic_cast<osgTerrain::TerrainTile *>(pTerrainTile);
    if(pTile == NULL)
    {
        return false;
    }

    bool bUseShadow = Registry::instance()->getUseShadow();

    if(!bModifyTexture)
    {
        osgTerrain::HeightFieldLayer *pHeightFieldLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTile->getElevationLayer());
        pHeightFieldLayer->restore();
    }
    else
    {
        osgTerrain::TextureLayer *pTextureLayer = bUseShadow ? dynamic_cast<osgTerrain::TextureLayer *>(pTile->getColorLayer(1u)) : dynamic_cast<osgTerrain::TextureLayer *>(pTile->getColorLayer(7u));

        if(pTextureLayer != NULL)
        {
            pTile->setColorLayer(bUseShadow ? 1u : 7u, NULL);
            osg::StateSet *pStateSet = pTile->getOrCreateStateSet();

            if(bUseShadow)
                osgShadow::SoftShadowMap::setSecondTexture(pStateSet, false);
            else
                EarthLightModel::setSampleStatus(pStateSet, 7u, false);
        }
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxTerrainModifications);

    if(bModifyTexture)
    {
        std::vector<OpenSP::sp<TerrainModification> >::const_iterator itor = m_vecTerrainModifications.begin();
        for(; itor != m_vecTerrainModifications.end(); ++itor)
        {
            TerrainModification *pTerrainModification = dynamic_cast<TerrainModification *>(itor->get());
            const std::string &strType = pTerrainModification->getType();
            const bool bModifyTexture = (strType.compare(TMT_DOM_MODIFICATION) == 0 || strType.compare(TMT_COLOR_MODIFICATION) == 0);
            if(!bModifyTexture)
            {
                continue;
            }

            pTerrainModification->modifyTerrainTile(pTerrainTile);
        }
    }
    else
    {
        std::vector<OpenSP::sp<TerrainModification> >::const_iterator itor = m_vecTerrainModifications.begin();
        for(; itor != m_vecTerrainModifications.end(); ++itor)
        {
            TerrainModification *pTerrainModification = dynamic_cast<TerrainModification *>(itor->get());
            const std::string &strType = pTerrainModification->getType();
            const bool bMidifyTexture = (strType.compare(TMT_DEM_MODIFICATION) == 0 || strType.compare(TMT_ELEVATION_MODIFICATION) == 0);
            if(bModifyTexture)
            {
                continue;
            }
            pTerrainModification->modifyTerrainTile(pTerrainTile);
        }
    }
    return true;
}
