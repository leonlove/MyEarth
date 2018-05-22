#ifndef ROUTINE_MANAGER_H_92F59AEE_713E_4516_A5E9_331646AFF754_INCLUDE
#define ROUTINE_MANAGER_H_92F59AEE_713E_4516_A5E9_331646AFF754_INCLUDE

#include "OpenSP/Ref.h"
#include "OpenThreads/Thread"
#include "OpenThreads/Mutex"
#include "OpenThreads/Block"
#include "OpenThreads/Atomic"
#include <list>
#include "DataStruct.h"
#include "WorkingThreads.h"

namespace deudb
{
    class DataBase;
    class RoutineManager : public OpenSP::Ref
    {
    public:
        explicit RoutineManager(void);
        virtual ~RoutineManager(void);

    public:
        bool    init(const std::string &strIndexFile, DataBase *pDataBase, UINT_64 nWriteBufferLimited);
        bool    addRoutine(const ID &id, const Routine &routine);
        void   *readRoutineBlock(const ID &id) const;

    protected:
        bool        doAction(void);

    protected:
        class RoutineThread : public WorkingThread
        {
        public:
            RoutineThread(RoutineManager *pRoutineMgr) : m_pRoutineMgr(pRoutineMgr){}
            ~RoutineThread(void){}

        protected:
            virtual void run(void);
            RoutineManager     *m_pRoutineMgr;
        };
        friend class RoutineThread;

    protected:
        RoutineThread              *m_pRoutineThread;

        typedef std::pair<ID, Routine>  RoutineTask;
        std::list<RoutineTask>      m_queueRoutines;
        OpenThreads::Mutex          m_mtxRoutines;

        UINT_64                     m_nWriteBufferLimited;
        UINT_64                     m_nWriteBufferSize;
        OpenThreads::Block          m_blockWriteBuffer;

        FILE       *m_pIndexFile;

        DataBase   *m_pDataBase;
    };

}

#endif

