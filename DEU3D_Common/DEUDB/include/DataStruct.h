#ifndef DATA_STRUCT_H_B6615B1E_2E8A_4D16_B78A_4B9DE093F018_INCLUDE
#define DATA_STRUCT_H_B6615B1E_2E8A_4D16_B78A_4B9DE093F018_INCLUDE

#include "IDProvider/ID.h"

namespace deudb
{
#if defined (WIN32) || defined (WIN64)
    typedef unsigned __int64 UINT_64;
#else
    typedef unsigned long long  UINT_64;
#endif

typedef long long              Version_64;
typedef std::vector<unsigned>  VersionList;

#pragma pack(push, 4)

struct FileGap
{
    UINT_64     m_nPosition;
    unsigned    m_nLength;
};


struct DBBlockInfo
{
    unsigned    m_nDBFile;
    FileGap     m_gap;
};

struct BlockInIdx_v2
{
    ID          m_id;
    DBBlockInfo m_infoDBBlock;
    unsigned    m_nVersion;
    unsigned    m_bRemove;
};

typedef struct BlockInIdx_v2    BlockInIdx;

struct BlockInIdx_v1
{
    unsigned m_nPosInIndex;
    BlockInIdx m_blockInIdx;
};

#pragma pack(pop)

struct Routine
{
    enum RoutineType
    {
        RT_ADD,
        RT_UPDATE,
        RT_REMOVE
    };

    RoutineType     m_eRoutineType;
    unsigned        m_nPosInIndex;
    DBBlockInfo     m_infoDBBlock;
    unsigned        m_nVersion;
    void           *m_pDataBlock;
};

class IDVersion
{
public:
    IDVersion(const ID& id,const unsigned& nVersion) {m_id = id;m_nVersion = nVersion;}
    ~IDVersion(){}

    inline IDVersion& operator=(const IDVersion &param)
    {
        if(this == &param)  return *this;
        m_id = param.m_id;
        m_nVersion = param.m_nVersion;
        return *this;
    }
    inline bool operator==(const IDVersion &param) const
    {
        if(m_id != param.m_id)  return false;
        if(m_nVersion != param.m_nVersion)   return false;
        return true;
    }

    inline bool operator!=(const IDVersion &param) const
    {
        return !operator==(param);
    }

    inline bool operator< (const IDVersion &param) const
    {
        if(m_id < param.m_id)       return true;
        else if(m_id > param.m_id)  return false;

        if(m_nVersion < param.m_nVersion)         return true;
        else if(m_nVersion > param.m_nVersion)    return false;

        return false;
    }

    inline bool operator<=(const IDVersion &param) const
    {
        return (operator==(param) || operator<(param));
    }

    inline bool operator> (const IDVersion &param) const
    {
        return !operator<=(param);
    }

    inline bool operator>=(const IDVersion &param) const
    {
        return !operator<(param);
    }

private:
    ID m_id;
    unsigned m_nVersion;
};

}

#endif
