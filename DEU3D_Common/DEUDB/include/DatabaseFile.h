#ifndef DATA_BASE_FILE_H_3F5858CC_4703_4509_AC2C_EBC7735901BA_INCLUDE
#define DATA_BASE_FILE_H_3F5858CC_4703_4509_AC2C_EBC7735901BA_INCLUDE

#include "Export.h"
#include <list>
#include <vector>
#include <deque>
#include <OpenThreads/Mutex>
#include <OpenThreads/Block>
#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>

#include "DataStruct.h"

#pragma warning( disable : 4996 )

namespace deudb
{

    enum ActionType{AT_WRITE_GAP, AT_RELEASE_GAP};

    class DatabaseFile;
    struct ActionItem
    {
        DatabaseFile   *m_pDatabaseFile;
        ActionType      m_eActionType;
        FileGap         m_gap;
        void           *m_pMemory;
    };


    class DatabaseFile : public OpenSP::Ref
    {
    public:
        explicit DatabaseFile(void);
        virtual ~DatabaseFile(void);

    public:
        bool            init(const std::string &strFilePath, const std::vector<FileGap> &vecWhiteGap = std::vector<FileGap>());
        void           *readBlock(const FileGap &gap);
        bool            writeBlock(const FileGap &gap, const void *pDataBlock);
        UINT_64         allocBlock(unsigned nLength);
        void            releaseBlock(const FileGap &gap);
        void            closeFile(void);
        void            applyAction(const ActionItem &actionItem);
    private:
        UINT_64         getFileLength();
        bool            chFileSize(UINT_64 nSize);

    protected:
        const static UINT_64    m_nFileSizeLimited;

        FILE                   *m_pFile;
        OpenThreads::Mutex      m_mtxFile;
        UINT_64                 m_nCurrentFileSize;

        std::list<FileGap>      m_listBlackGap;
        OpenThreads::Mutex      m_mtxBlackGap;

        UINT_64                 m_nAllocFileSize;
    };

}

#endif
