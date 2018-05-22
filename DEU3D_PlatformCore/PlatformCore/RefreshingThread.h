#ifndef REFRESHING_THREAD_H_A690613A_6B47_4926_BE5B_BE7394440C2D_INCLUDE
#define REFRESHING_THREAD_H_A690613A_6B47_4926_BE5B_BE7394440C2D_INCLUDE

#include <OpenThreads/Thread>
#include <OpenSP/Ref.h>
#include <osgViewer/ViewerBase>

class RefreshingThread : public OpenThreads::Thread, public OpenSP::Ref
{
public:
    RefreshingThread(void);
    ~RefreshingThread(void);

protected:
    osg::ref_ptr<osgViewer::ViewerBase>    m_pViewer;
};

#endif

