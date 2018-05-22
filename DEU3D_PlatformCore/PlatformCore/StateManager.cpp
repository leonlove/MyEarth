#include "StateManager.h"

#include "IState.h"
#include "VisibleState.h"
#include "ColorState.h"
#include "SmokeState.h"
#include "OutLineState.h"
#include "ViewState.h"
#include "RippleState.h"
#include "DirectionState.h"
#include "TopmostState.h"
#include "WireFrameState.h"
#include "ShadowState.h"

StateManager::StateManager(void)
{
    OpenSP::sp<VisibleState> pVisibleState = new VisibleState("Visible");
    m_mapStates["Visible"] = pVisibleState;
}

//此为零时添加解决烟雾内存问题的，待正式解决后需要删除
StateManager::StateManager(SceneGraphOperator *pSceneGraphOperator)
{
    OpenSP::sp<VisibleState> pVisibleState = new VisibleState("Visible");
    m_mapStates["Visible"] = pVisibleState;
    m_pSceneGraphOperator = pSceneGraphOperator;
}
//


StateManager::~StateManager(void)
{
}

IState *StateManager::createRenderState(const std::string &strTypeName, const std::string &strStateName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxStates);

    //STATE_VISIBLE对应的IState是唯一的，不可多次创建
    if(strTypeName.compare(cmm::STATE_VISIBLE) == 0)
    {
        return m_mapStates["Visible"].get();
    }

    std::map<std::string, OpenSP::sp<IState> >::iterator itor = m_mapStates.find(strStateName);
    if(itor != m_mapStates.end())
    {
        if(itor->second->getType() == strTypeName)
        {
            return itor->second.get();
        }
        return NULL;
    }

    if(strTypeName  == cmm::STATE_OUTLINE)
    {
        OpenSP::sp<OutLineState> pOutLineState = new OutLineState(strStateName);
        m_mapStates[strStateName] = pOutLineState;
        return pOutLineState.get();
    }
    else if(strTypeName == cmm::STATE_COLOR)
    {
        OpenSP::sp<ColorState> pColorState = new ColorState(strStateName);
        m_mapStates[strStateName] = pColorState;
        return pColorState.get();
    }
    else if(strTypeName == cmm::STATE_SMOKE)
    {
        OpenSP::sp<SmokeState> pSmokeState = new SmokeState(strStateName, m_pSceneGraphOperator);
        m_mapStates[strStateName] = pSmokeState;
        return pSmokeState.get();
    }
    else if(strTypeName == cmm::STATE_VIEWS)
    {
        OpenSP::sp<ViewState> pViewState = new ViewState(strStateName);
        m_mapStates[strStateName] = pViewState;
        return pViewState.get();
    }
    else if(strTypeName == cmm::STATE_RIPPLE)
    {
        OpenSP::sp<RippleState> pRippleState = new RippleState(strStateName);
        m_mapStates[strStateName] = pRippleState;
        return pRippleState.get();
    }
    else if(strTypeName.compare(cmm::STATE_DIRECTION) == 0)
    {
        OpenSP::sp<DirectionState> pDirectionState = new DirectionState(strStateName);
        m_mapStates[strStateName] = pDirectionState;
        return pDirectionState.get();
    }
    else if(strTypeName.compare(cmm::STATE_TOPMOST) == 0)
    {
        OpenSP::sp<TopmostState> pDepthState = new TopmostState(strStateName);
        m_mapStates[strStateName] = pDepthState;
        return pDepthState.get();
    }
    else if(strTypeName.compare(cmm::STATE_WIRE_FRAME) == 0)
    {
        OpenSP::sp<WireFrameState>  pWireFrameState = new WireFrameState(strStateName);
        m_mapStates[strStateName] = pWireFrameState;
        return pWireFrameState.get();
    }
    else if(strTypeName.compare(cmm::STATE_SHADOW) == 0)
    {
        {
            OpenSP::sp<ShadowState>  pShadowState = new ShadowState(strStateName);
            m_mapStates[strStateName] = pShadowState;
            return pShadowState.get();
        }
    }
    return NULL;
}


IState *StateManager::getRenderState(const std::string &strStateName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxStates);

    if(strStateName.compare(cmm::STATE_VISIBLE) == 0)
    {
        return m_mapStates["Visible"];
    }
    std::map<std::string, OpenSP::sp<IState> >::iterator itor = m_mapStates.find(strStateName);
    if(itor != m_mapStates.end())
    {
        return itor->second.get();
    }
    return NULL;
}

std::vector<IState *> StateManager::getRenderStateList(const std::string &strTypeName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxStates);
    std::vector<IState *> stateList;
    for(std::map<std::string, OpenSP::sp<IState> >::iterator itor = m_mapStates.begin(); itor != m_mapStates.end(); ++itor)
    {
        if(itor->second->getType().compare(strTypeName) == 0)
        {
            stateList.push_back(itor->second.get());
        }
    }

    return stateList;
}