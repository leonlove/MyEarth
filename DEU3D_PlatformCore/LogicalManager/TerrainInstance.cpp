#include "TerrainInstance.h"

#include <OpenSP/sp.h>
#include "LayerManager.h"

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

namespace logical
{

TerrainInstance::TerrainInstance(const ID &id) :
    Object(id)
{
    m_nDataSetCode = ~0u;
}


TerrainInstance::~TerrainInstance(void)
{
}


bool TerrainInstance::init(void)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return false;
    }

    PropertyManager *pPropertyMgr = dynamic_cast<PropertyManager *>(pLayerManager->getPropertyManager());
    if (pPropertyMgr == NULL)
    {
        return false;
    }

    void *pPropData = NULL;
    unsigned nDataLen = 0u;
    pPropertyMgr->fetchPropertyData(m_id, pPropData, nDataLen);
    if(pPropData == NULL || nDataLen < 1u)
    {
        return true;
    }

    bson::bsonDocument bsonDoc;
    bsonDoc.FromBsonStream(pPropData, nDataLen);

    const bool bRet = fromBson(bsonDoc);
    delete[] pPropData;
    return bRet;
}


bool TerrainInstance::fromBson(const bson::bsonDocument &bsonDoc)
{
    if(!__super::fromBson(bsonDoc))
    {
        return false;
    }

    const bson::bsonElement *pIDElement = bsonDoc.GetElement("ID");
    if(!pIDElement)
    {
        return false;
    }

    const bson::bsonElement *pTileElement = bsonDoc.GetElement("ChildrenID");
    if(!pTileElement)
    {
        return false;
    }

    const bson::bsonArrayEle *pTileArrayElement = dynamic_cast<const bson::bsonArrayEle *>(pTileElement);
    if (pTileArrayElement == NULL)
    {
        return false;
    }

    const unsigned nChildrenCount = pTileArrayElement->ChildCount();
    for(unsigned n = 0u; n < nChildrenCount; n++)
    {
        const bson::bsonDocumentEle *pOneTileEle = dynamic_cast<const bson::bsonDocumentEle *>(pTileArrayElement->GetElement(n));
        if(!pOneTileEle) continue;

        const bson::bsonDocument &oneTileDoc = pOneTileEle->GetDoc();
        const bson::bsonElement *pChild_Sub = oneTileDoc.GetElement(0u);

        const std::string strID = pChild_Sub->EName();
        const unsigned nLevel = pChild_Sub->Int32Value();

        m_mapTerrainData[ID::ID(strID)] = nLevel;
    }

    return true;
}


void TerrainInstance::setState(const std::string &strStateName, bool bState)
{
}


unsigned TerrainInstance::getCode(void) const
{
    return m_nDataSetCode;
}


unsigned TerrainInstance::getTilesCount(void) const
{
    return m_mapTerrainData.size();
}


bool TerrainInstance::getTileInfo( int index , ID& id, int& lLevel)
{
    int i = 0;
    bool b = false;
    for (std::map<ID, unsigned>::iterator it = m_mapTerrainData.begin(); it != m_mapTerrainData.end(); it++)
    {
        if (i == index)
        {
            id = it->first;
            lLevel = it->second;
            b = true;
            break;
        }
        else
            i++;
    }
    return b;
}

}