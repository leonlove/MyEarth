#ifndef DATA_BASE_H_76F8629E_713E_4516_A5E9_3FF256AFF754_INCLUDE
#define DATA_BASE_H_76F8629E_713E_4516_A5E9_3FF256AFF754_INCLUDE

#include "DatabaseFile.h"
#include <map>
#include <string>
#include <vector>
#include <OpenThreads/Mutex>
#include <IDProvider/ID.h>
#include <OpenSP/Ref.h>

namespace deudb
{
    typedef std::map<unsigned, std::vector<FileGap> >    FILE_GAP_MAP;
    class RoutineManager;
    class DataBase : public OpenSP::Ref
    {
    public:
        explicit DataBase(void);
        virtual ~DataBase(void);

    public:
        void    init(const std::string &strDatabase, const FILE_GAP_MAP &mapWhiteGap);
        void    closeDB(void);

        void   *readBlock(const DBBlockInfo &infoDBBlock);
        bool    writeBlock(const DBBlockInfo &infoDBBlock, const void *pDataBlock);
        bool    allocBlock(unsigned nLength, unsigned &nDBFile, UINT_64 &nPosition);
        void    releaseBlock(const DBBlockInfo &infoDBBlock);
         
    protected:
        const static std::string    ms_strDBFileExt;

        std::map<unsigned, OpenSP::sp<DatabaseFile> >   m_mapDatabaseFiles;
        OpenThreads::Mutex      m_mtxDataBase;
        std::string             m_strDataBase;

        RoutineManager         *m_pRoutineManager;
    };
}

#endif
