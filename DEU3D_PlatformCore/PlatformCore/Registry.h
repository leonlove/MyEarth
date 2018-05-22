#ifndef REGISTRY_H_BF250A12_CFC9_4189_9E6E_1A59935EFFED_INCLUDE
#define REGISTRY_H_BF250A12_CFC9_4189_9E6E_1A59935EFFED_INCLUDE

#include <string>
#include <osg/ref_ptr>

#include "Capabilities.h"

class ParmRectifyThreadPool;

class Registry : public OpenSP::Ref
{
public:
    Registry(void);
    virtual ~Registry(void);

public:
    /** Access the global Registry singleton. */
    static Registry* instance(bool erase = false);

public:
    const Capabilities &getCapabilities() const;
    void setParmRectifyThreadCount(unsigned int nCount = 2) { m_nParmRectifyThreadCount = nCount; }
    unsigned int getParmRectifyThreadCount(void) { return m_nParmRectifyThreadCount; }
    ParmRectifyThreadPool *getParmRectifyThreadPool();

    void setUseShadow(bool bUseShadow) {    m_bUseShadow = bUseShadow;  }
    bool getUseShadow(void) {   return m_bUseShadow;    }
protected:
    void initCapabilities()
    {
        if(!m_pCapabilities.valid())
            m_pCapabilities = new Capabilities();
    }

    void initParmRectifyThreadPool();


protected:
    osg::ref_ptr<Capabilities>          m_pCapabilities;
    OpenSP::sp<ParmRectifyThreadPool>   m_pThreadPool;
    unsigned int                        m_nParmRectifyThreadCount;
    bool                                m_bUseShadow;
};

#endif