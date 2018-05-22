#include "RefreshingThread.h"
#include <osgViewer/View>


RefreshingThread::RefreshingThread(void)
{
    setStackSize(128u * 1024u);
}


RefreshingThread::~RefreshingThread(void)
{
}

