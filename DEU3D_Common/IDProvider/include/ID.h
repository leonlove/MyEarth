#ifndef ID_H_DE02727E_D067_411B_A41F_6A0BDF5785F2_INCLUDE
#define ID_H_DE02727E_D067_411B_A41F_6A0BDF5785F2_INCLUDE

#include "Export.h"
#include <string>
#include <vector>

#ifdef WIN32
typedef unsigned __int64    UINT_64;
enum DeuObjectIDType;

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4201 )
#endif

#else
#include "Definer.h"
typedef unsigned long long    UINT_64;
#endif
typedef unsigned int     UINT_32;


class IPV_EXPORT ID
{
public:
#ifdef WIN32
    ID(void)                        {   set(~0ui64, ~0ui64, ~0ui64);                        }
    ID(const std::string &strID)    {   set(~0ui64, ~0ui64, ~0ui64);    fromString(strID);  }
#else
    ID(void)                        {   set(~0uLL, ~0uLL, ~0uLL);                           }
    ID(const std::string &strID)    {   set(~0uLL, ~0uLL, ~0uLL);       fromString(strID);  }
#endif

    explicit ID(const void *pBlock, unsigned nLength);
    explicit ID(UINT_64 nHighBit, UINT_64 nMidBit, UINT_64 nLowBit);
    explicit ID(UINT_32 nDataSetCode, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_32 nHeight);
    explicit ID(UINT_32 nDataSetCode, DeuObjectIDType eType, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_64 nUniqueID);
    explicit ID(UINT_32 nDataSetCode, DeuObjectIDType eType, const unsigned char szModelID[]);
    explicit ID(UINT_32 nDataSetCode, DeuObjectIDType eType);

    ID(const ID &id);
    ~ID(void){}

public:
    static ID   genNewID(void);
    static ID   genIDfromString(const std::string &strID);
    static ID   genIDfromBinary(const void *pBinData, unsigned int nBinDataLen);

    static const ID   &getCultureLayerRootID(void);
    static const ID   &getTerrainDEMLayerRootID(void);
    static const ID   &getTerrainDOMLayerRootID(void);
    static const ID   &getVirtualTileRootID(void);
    static const ID   &getSymbolCategoryRootID(void);

public:
    void        generate(void);

    inline bool isValid(void) const;
    inline void setInvalid(void);
    void        set(const ID &id);
    void        set(const void *pBlock, unsigned nLength);
    inline void set(UINT_64 nHighBit, UINT_64 nMidBit, UINT_64 nLowBit);
    void        set(UINT_32 nDataSetCode, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, int nHeight);
    void        set(UINT_32 nDataSetCode, DeuObjectIDType eType, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_64 nUniqueID);
    void        set(UINT_32 nDataSetCode, DeuObjectIDType eType, const unsigned char szModelID[]);
    void        fromString(const std::string &strID);
    std::string toString(void) const;

    ID          tran2FindingID(void) const;

    inline const ID    &operator =(const ID &param);
    inline bool         operator==(const ID &param) const;
    inline bool         operator!=(const ID &param) const;
    inline bool         operator< (const ID &param) const;
    inline bool         operator<=(const ID &param) const;
    inline bool         operator> (const ID &param) const;
    inline bool         operator>=(const ID &param) const;

public:
#pragma pack(push, 1)
    union
    {
        struct
        {
            UINT_64        m_nLowBit;
            UINT_64        m_nMidBit;
            UINT_64        m_nHighBit;
        };

        struct
        {
            unsigned short m_nDataSetCode;
            unsigned char  m_nType;
            unsigned char  m_UniqueID[16];      // 16×Ö½ÚµÄGUID
            unsigned char  m_reserve[5];
        }ObjectID;

        struct
        {
            unsigned short   m_nDataSetCode;
            unsigned char    m_nType;
            unsigned char    m_nLevel;
            unsigned int     m_nRow;
            unsigned int     m_nCol;
            UINT_64          m_nUniqueID;
            unsigned char    m_reserve[4];
        }TileID;

        struct
        {
            unsigned short  m_nDataSetCode;
            unsigned char   m_nType;
            unsigned char   m_nLevel;
            unsigned int    m_nRow;
            unsigned int    m_nCol;
            unsigned int    m_nHeight;
            unsigned char   m_reserve[8];
        }CubeID;
    };
#pragma pack(pop)
};


inline const ID &ID::operator=(const ID &param)
{
    if(this == &param)  return *this;
    m_nHighBit = param.m_nHighBit;
    m_nMidBit  = param.m_nMidBit;
    m_nLowBit  = param.m_nLowBit;
    return *this;
}


inline bool ID::operator==(const ID &param) const
{
    if(m_nHighBit != param.m_nHighBit)  return false;
    if(m_nMidBit  != param.m_nMidBit)   return false;
    if(m_nLowBit  != param.m_nLowBit)   return false;
    return true;
}

inline bool ID::operator!=(const ID &param) const
{
    return !operator==(param);
}

inline bool ID::operator< (const ID &param) const
{
    if(m_nHighBit < param.m_nHighBit)       return true;
    else if(m_nHighBit > param.m_nHighBit)  return false;

    if(m_nMidBit < param.m_nMidBit)         return true;
    else if(m_nMidBit > param.m_nMidBit)    return false;

    if(m_nLowBit < param.m_nLowBit)         return true;
    else if(m_nLowBit > param.m_nLowBit)    return false;

    return false;
}


inline bool ID::operator<=(const ID &param) const
{
    return (operator==(param) || operator<(param));
}


inline bool ID::operator> (const ID &param) const
{
    return !operator<=(param);
}


inline bool ID::operator>=(const ID &param) const
{
    return !operator<(param);
}


#ifdef WIN32
inline bool ID::isValid(void) const
{
    return (m_nHighBit != ~0ui64 || m_nMidBit != ~0ui64 || m_nLowBit != ~0ui64);
}

inline void ID::setInvalid(void)
{
    set(~0ui64, ~0ui64, ~0ui64);
}
#else
inline bool ID::isValid(void) const
{
    return (m_nHighBit != ~0uLL || m_nMidBit != ~0uLL || m_nLowBit != ~0uLL);
}

inline void ID::setInvalid(void)
{
    set(~0uLL, ~0uLL, ~0uLL);
}
#endif

inline void ID::set(UINT_64 nHighBit, UINT_64 nMidBit, UINT_64 nLowBit)
{
    m_nLowBit  = nLowBit;
    m_nMidBit  = nMidBit;
    m_nHighBit = nHighBit;
}

typedef std::vector<ID> IDList;


#endif
