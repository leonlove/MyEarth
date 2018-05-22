#ifndef  _DEUNETWORK_DEULOADLIBS_H_
#define  _DEUNETWORK_DEULOADLIBS_H_

#include <vector>
#include <Windows.h>

namespace deunw
{
    class DEULoadLibs
    {
    public:
        DEULoadLibs(void);
        ~DEULoadLibs(void);

        void loadLibs();
        void freeLibs();

    private:
        std::vector<HMODULE> m_hmVec;
    };
}

#endif  //_DEUNETWORK_DEULOADLIBS_H_
