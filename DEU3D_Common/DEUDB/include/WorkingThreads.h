#ifndef WORKING_THREADS_H_02FFB0F7_399E_49D9_8FC8_3DF638BA53E9_INCLUDE
#define WORKING_THREADS_H_02FFB0F7_399E_49D9_8FC8_3DF638BA53E9_INCLUDE

#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>
#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <OpenSP/Ref.h>

#if defined (WIN32) || defined (WIN64)
#include <OpenThreads/Win32ConditionPrivateData.h>
#endif


namespace deudb
{
    class WorkingThread : public OpenThreads::Thread, public OpenSP::Ref
    {
    public:
        explicit WorkingThread(void)
        {
            m_MissionFinished.exchange(0u);
            m_nSuspendCount = 0;
            m_block.release();
        }

    public:
        void suspend(bool bSuspend)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxSuspendCount);
            if(bSuspend)
            {
                ++m_nSuspendCount;
            }
            else
            {
                --m_nSuspendCount;
            }

            m_block.set(m_nSuspendCount <= 0);
        }

        void finishMission(void)
        {
            m_MissionFinished.exchange(1);
            m_block.set(true);
            join();
        }

    protected:
        OpenThreads::Block      m_block;
        OpenThreads::Atomic     m_MissionFinished;

        int                     m_nSuspendCount;
        OpenThreads::Mutex      m_mtxSuspendCount;
    };
}

#endif
