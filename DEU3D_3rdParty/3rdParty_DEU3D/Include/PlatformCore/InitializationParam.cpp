#include "InitializationParam.h"
#include <common/Common.h>

InitializationParam::InitializationParam(void)
{
    Default();
}


InitializationParam::InitializationParam(const InitializationParam &param)
{
    setParam(param);
}


void InitializationParam::Default(void)
{
    m_hWnd = NULL;
    m_nFetchingThreadCount      = 1u;
    m_bBifurcateFetchingThread  = false;
    m_nAntiAliasTimes           = 16u;
    m_bUseShadow                = false;

    m_nInitializationWndWidth = 8u;
    m_nInitializationWndHeight = 8u;

    m_nLoadingCountPerFrame   = 0u;
}


void InitializationParam::setParam(const InitializationParam &param)
{
    if(this == &param)  return;

    m_hWnd            = param.m_hWnd;

    m_nFetchingThreadCount      = param.m_nFetchingThreadCount;
    m_bBifurcateFetchingThread  = param.m_bBifurcateFetchingThread;
    m_nAntiAliasTimes           = param.m_nAntiAliasTimes;

    m_nInitializationWndWidth  = param.m_nInitializationWndWidth;
    m_nInitializationWndHeight = param.m_nInitializationWndHeight;

    m_nLoadingCountPerFrame    = param.m_nLoadingCountPerFrame;
}


const InitializationParam &InitializationParam::operator=(const InitializationParam &param)
{
    setParam(param);
    return *this;
}


bool InitializationParam::isEqual(const InitializationParam &param) const
{
    if(this == &param)  return true;

    if(m_hWnd != param.m_hWnd)                      return false;

    if(m_nFetchingThreadCount     != param.m_nFetchingThreadCount)      return false;
    if(m_bBifurcateFetchingThread != param.m_bBifurcateFetchingThread)  return false;
    if(m_nAntiAliasTimes          != param.m_nAntiAliasTimes)           return false;

    if(m_nInitializationWndWidth  != param.m_nInitializationWndWidth)   return false;
    if(m_nInitializationWndHeight != param.m_nInitializationWndHeight)  return false;

    if(m_nLoadingCountPerFrame != param.m_nLoadingCountPerFrame)        return false;

    return true;
}


bool InitializationParam::operator==(const InitializationParam &param) const
{
    return isEqual(param);
}


bool InitializationParam::operator!=(const InitializationParam &param) const
{
    return !isEqual(param);
}