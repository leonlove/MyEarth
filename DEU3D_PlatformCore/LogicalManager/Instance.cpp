#include "Instance.h"

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

Instance::Instance(const ID &id) :
    Object(id)
{
}


Instance::~Instance(void)
{
}


void Instance::setState(const std::string &strStateName, bool bState)
{
    std::vector<Layer *>::iterator itor = m_vecParents.begin();
    for( ; itor != m_vecParents.end(); ++itor)
    {
        Layer *pParent = *itor;

        if (pParent == NULL)
        {
            continue;
        }

        pParent->setChildState(m_id, strStateName, bState);
    }

    OpenSP::sp<LayerManager>    pLayerManager;
    if(ms_pLayerManager.lock(pLayerManager))
    {
        pLayerManager->refreashObjectState(m_id, strStateName, bState);
    }
}

bool Instance::getState(const std::string &strStateName) const
{
    unsigned int nParentNum = const_cast<Instance *>(this)->getParentCount();
    for(unsigned int i = 0; i < nParentNum; i++)
    {
        Layer *pParent = dynamic_cast<Layer *>(const_cast<Instance *>(this)->getParent(i));

        if (pParent == NULL)
        {
            continue;
        }

        bool bState = pParent->getChildState(m_id, strStateName);
        if(bState) return true;
    }
    return false;
}


}