#ifndef VIEWER_PARAM_H_A32C9B13_7F8C_435B_882F_B5A71F91C4B8_INCLUDE
#define VIEWER_PARAM_H_A32C9B13_7F8C_435B_882F_B5A71F91C4B8_INCLUDE

#include "Export.h"
#include <string>
#include <IDProvider/ID.h>

struct ViewerParam
{
    explicit ViewerParam(void)
    {
        m_hWnd            = NULL;
        m_dblFovy         = 45.0;
        m_nAntiAliasTimes = 4u;
        m_nInitializationWndWidth  = 8u;
        m_nInitializationWndHeight = 8u;
    }
    ViewerParam(const ViewerParam &param)
    {
        operator=(param);
    }

    const ViewerParam &operator=(const ViewerParam &param)
    {
        if(this == &param)  return *this;
        m_hWnd            = param.m_hWnd;
        m_dblFovy         = param.m_dblFovy;
        m_nAntiAliasTimes = param.m_nAntiAliasTimes;

        m_nInitializationWndWidth  = param.m_nInitializationWndWidth;
        m_nInitializationWndHeight = param.m_nInitializationWndHeight;

        m_idTerrain = param.m_idTerrain;

        return *this;
    }

    bool operator==(const ViewerParam &param) const
    {
        if(this == &param)                  return true;
        if(m_hWnd != param.m_hWnd)          return false;
        if(m_dblFovy != param.m_dblFovy)    return false;
        if(m_nAntiAliasTimes != param.m_nAntiAliasTimes)            return false;

        if(m_nInitializationWndWidth  != param.m_nInitializationWndWidth)   return false;
        if(m_nInitializationWndHeight != param.m_nInitializationWndHeight)  return false;

        if(m_idTerrain != param.m_idTerrain)    return false;

        return true;
    }

    void            *m_hWnd;
    double          m_dblFovy;
    unsigned        m_nAntiAliasTimes;

    unsigned        m_nInitializationWndWidth;
    unsigned        m_nInitializationWndHeight;
    ID              m_idTerrain;
    //ID              m_idCulture;
};


#endif
