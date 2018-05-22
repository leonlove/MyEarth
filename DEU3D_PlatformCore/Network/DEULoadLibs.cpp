#include "DEULoadLibs.h"


namespace deunw
{

    DEULoadLibs::DEULoadLibs(void)
    {
        loadLibs();
    }


    DEULoadLibs::~DEULoadLibs(void)
    {
        freeLibs();
    }

    void DEULoadLibs::loadLibs()
    {
        HMODULE hm = LoadLibrary("WSHTCPIP.DLL");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("IPHLPAPI.DLL");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("winnsi.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("nlaapi.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("NapiNSP.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("pnrpnsp.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("winrnr.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("mdnsNSP.dll");
        if(hm != NULL)
            m_hmVec.push_back(hm);

        hm = LoadLibrary("FWPUCLNT.DLL");
        if(hm != NULL)
            m_hmVec.push_back(hm);
    }

    void DEULoadLibs::freeLibs()
    {
        for(size_t i = 0;i < m_hmVec.size();i++)
        {
            FreeLibrary(m_hmVec[i]);
        }
        m_hmVec.clear();
    }
}