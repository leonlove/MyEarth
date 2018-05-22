#ifndef DEU_INITIALIZATION_PARAM_5D3F21FB_753B_41DA_82D4_77E829068100_INCLUDE
#define DEU_INITIALIZATION_PARAM_5D3F21FB_753B_41DA_82D4_77E829068100_INCLUDE

#include "Export.h"
#include <IDProvider/ID.h>
#include <OpenSP/Ref.h>
#include <vector>
#include <string>

class PLATFORM_EXPORT InitializationParam : public OpenSP::Ref
{
public:
    explicit InitializationParam(void);
    InitializationParam(const InitializationParam &param);

public:
    void     Default(void);
    void     setParam(const InitializationParam &param);
    const    InitializationParam &operator=(const InitializationParam &param);
    bool     isEqual(const InitializationParam &param) const;
    bool     operator==(const InitializationParam &param) const;
    bool     operator!=(const InitializationParam &param) const;

public:
    void*           m_hWnd;

    unsigned        m_nFetchingThreadCount;
    bool            m_bBifurcateFetchingThread;
    unsigned        m_nAntiAliasTimes;
    bool            m_bUseShadow;

    unsigned        m_nInitializationWndWidth;
    unsigned        m_nInitializationWndHeight;

    unsigned        m_nLoadingCountPerFrame;
};

#endif