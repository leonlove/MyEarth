#include "Registry.h"

#include <OpenSP/sp.h>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#endif

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "ParmRectifyThreadPool.h"

Registry::Registry()
{
    m_nParmRectifyThreadCount = 2;
    m_bUseShadow = false;
}

Registry::~Registry()
{

}

Registry *Registry::instance(bool erase)
{
    static OpenSP::sp<Registry> s_registry = new Registry;
    if (erase) 
    {
        s_registry = 0;
    }

    return s_registry.get(); // will return NULL on erase
}

const Capabilities &Registry::getCapabilities() const
{
    if(!m_pCapabilities.valid())
    {
        const_cast<Registry *>(this)->initCapabilities();
    }

    return *m_pCapabilities.get();
}

ParmRectifyThreadPool *Registry::getParmRectifyThreadPool()
{
    if(!m_pThreadPool.valid())
    {
        const_cast<Registry *>(this)->initParmRectifyThreadPool();
    }

    return m_pThreadPool.get();
}

void Registry::initParmRectifyThreadPool()
{
    if(!m_pThreadPool.valid())
    {
        //m_pThreadPool = new ParmRectifyThreadPool();
    }
}