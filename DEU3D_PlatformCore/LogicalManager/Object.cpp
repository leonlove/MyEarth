#include "Object.h"
#include "Layer.h"

#include <algorithm>
#include <common/StateDefiner.h>
#include "IProperty.h"
#include "LayerManager.h"

namespace logical
{

OpenSP::op<LayerManager> Object::ms_pLayerManager = NULL;

void getBoundFromBson(const bson::bsonArrayEle *pBsonEle, cmm::math::Sphered &sphere)
{
    const unsigned nChildCount = pBsonEle->ChildCount();
    if(nChildCount < 4u)
    {
        return;
    }

    double dblBoundingSphere[4] = {0.0};
    for(unsigned n = 0u; n < nChildCount; n++)
    {
        const bson::bsonElement *pEle = pBsonEle->GetElement(n);
        dblBoundingSphere[n] = pEle->DblValue();
    }

    sphere.setCenter(cmm::math::Point3d(dblBoundingSphere[0], dblBoundingSphere[1], dblBoundingSphere[2]));
    sphere.setRadius(dblBoundingSphere[3]);
}


bool fetchBoundingSphere(const bson::bsonDocument &bsonDoc, cmm::math::Sphered &sphere)
{
    const static char szBoundSphere[] = "BoundingSphere";
    bool bFound = false;
    const unsigned nChildCount = bsonDoc.ChildCount();
    for(unsigned n = 0u; n < nChildCount; n++)
    {
        const bson::bsonElement *pElement = bsonDoc.GetElement(n);
        if(!pElement)   continue;

        if(strcmp(szBoundSphere, pElement->EName()) == 0)
        {
            getBoundFromBson(dynamic_cast<const bson::bsonArrayEle *>(pElement), sphere);
            bFound = true;
            break;
        }

        const bson::bsonElementType eType = pElement->GetType();
        if(eType == bson::bsonDocType)
        {
            const bson::bsonDocumentEle *pDocElement = dynamic_cast<const bson::bsonDocumentEle *>(pElement);
            const bool bRet = fetchBoundingSphere(pDocElement->GetDoc(), sphere);
            if(bRet)
            {
                bFound = true;
                break;
            }
        }
    }

    return bFound;
}


bool fetchMaxRange(const bson::bsonDocument &bsonDoc, double &dblMaxRange)
{
    const static char szMaxRange[] = "MaxRange";
    bool bFound = false;
    const unsigned nChildCount = bsonDoc.ChildCount();
    for(unsigned n = 0u; n < nChildCount; n++)
    {
        const bson::bsonElement *pElement = bsonDoc.GetElement(n);
        if(!pElement)   continue;

        if(strcmp(szMaxRange, pElement->EName()) == 0)
        {
            dblMaxRange = pElement->DblValue();
            bFound = true;
            break;
        }

        const bson::bsonElementType eType = pElement->GetType();
        if(eType == bson::bsonDocType)
        {
            const bson::bsonDocumentEle *pDocElement = dynamic_cast<const bson::bsonDocumentEle *>(pElement);
            const bool bRet = fetchMaxRange(pDocElement->GetDoc(), dblMaxRange);
            if(bRet)
            {
                bFound = true;
                break;
            }
        }
    }

    return bFound;
}


bool fetchBoundInfo(const bson::bsonDocument &bsonDoc, cmm::math::Sphered &sphere, bool bConsiderRange)
{
    const bool bRet1 = fetchBoundingSphere(bsonDoc, sphere);

    double dblMaxRange = -FLT_MAX;
    const bool bRet2 = (bConsiderRange ? fetchMaxRange(bsonDoc, dblMaxRange) : true);

    if(sphere.getRadius() < dblMaxRange)
    {
        sphere.setRadius(dblMaxRange);
    }

    return (bRet1 && bRet2);
}


bool fetchBoundInfo(const void *pBuffer, unsigned nLength, cmm::math::Sphered &sphere, bool bConsiderRange/* = true*/)
{
    if(!pBuffer || nLength < 1u)
    {
        return false;
    }

    bson::bsonDocument bsonDoc;
    bsonDoc.FromBsonStream(pBuffer, nLength);

    return fetchBoundInfo(bsonDoc, sphere, bConsiderRange);
}


Object::Object(const ID &id) :
    m_id(id),
    m_strName("")
{
}


Object::~Object(void)
{
}


bool Object::init(void)
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
        if (m_id == ID::getCultureLayerRootID()    ||
            m_id == ID::getTerrainDEMLayerRootID() ||
            m_id == ID::getTerrainDOMLayerRootID())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bson::bsonDocument bsonDoc;
    bsonDoc.FromBsonStream(pPropData, nDataLen);

    const bool bRet = fromBson(bsonDoc);
    delete[] pPropData;
    return bRet;
}


bool Object::fromBson(const bson::bsonDocument &bsonDoc)
{
    const bson::bsonElement *pNameElement = bsonDoc.GetElement("Name");
    if(pNameElement)
    {
        m_strName = pNameElement->StrValue();
    }

    const bson::bsonElement *pBSElement = bsonDoc.GetElement("BoundingSphere");
    if(pBSElement)
    {
        const bson::bsonArrayEle *pArrayElement = dynamic_cast<const bson::bsonArrayEle *>(pBSElement);
        getBoundFromBson(pArrayElement, m_BoundingSphere);
    }

    return true;
}


void Object::addParent(Layer *pLayer)
{
    std::vector<Layer *>::const_iterator itorFind 
        = std::find(m_vecParents.begin(), m_vecParents.end(), pLayer);
    if(m_vecParents.end() == itorFind)
    {
        m_vecParents.push_back(pLayer);
    }
}


void Object::removeParent(Layer *pLayer)
{
    std::vector<Layer *>::const_iterator itorFind 
        = std::find(m_vecParents.begin(), m_vecParents.end(), pLayer);
    if(m_vecParents.end() != itorFind)
    {
        m_vecParents.erase(itorFind);
    }
}


const std::string &Object::getName(void) const
{
    if(m_strName.empty())
    {
        const_cast<Object &>(*this).getName2();
    }
    return m_strName;
}


void Object::getName2(void)
{
    OpenSP::sp<LayerManager>    pLayerManager;
    if(!ms_pLayerManager.lock(pLayerManager))
    {
        return;
    }

    IPropertyManager *pPropMgr = pLayerManager->getPropertyManager();
    OpenSP::sp<IProperty> pProperty = pPropMgr->findProperty(m_id);
    if(!pProperty.valid())      return;

    OpenSP::sp<IProperty> pNameProperty = pProperty->findProperty("Name");
    if(!pNameProperty.valid())  return;

    m_strName = pNameProperty->getValue();
}


unsigned int Object::getParentCount(void) const
{
    return m_vecParents.size();
}

ILayer *Object::getParent(unsigned nIndex)
{
    if(nIndex >= getParentCount())
    {
        return NULL;
    }
    return m_vecParents[nIndex];
}

const ILayer *Object::getParent(unsigned nIndex) const
{
    if(nIndex >= getParentCount())
    {
        return NULL;
    }
    return m_vecParents[nIndex];
}

}