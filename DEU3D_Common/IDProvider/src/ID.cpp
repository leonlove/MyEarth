#include <sstream>
#include <iomanip>
#ifdef WIN32
#include <objbase.h>
#include <ID.h>
#include <Definer.h>
#else
#include <string.h>
#include "ID.h"
#endif


#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif

struct BitFour
{
    unsigned  LowBit : 4;
    unsigned  HighBit : 4;
};

#include "time.h"

#include "DEUCheck.h"
DEUCheck checker(18, 1);


static unsigned gs_nLogicalLayerCode = 6u;
static unsigned gs_nVirtualTileCode  = 0u;

const ID &ID::getCultureLayerRootID(void)
{
    static const ID idRoot(gs_nLogicalLayerCode, CULTURE_LAYER_ID, NULL);
    return idRoot;
}


const ID &ID::getTerrainDEMLayerRootID(void)
{
    static const ID idRoot(gs_nLogicalLayerCode, TERRAIN_DEM_LAYER_ID, NULL);
    return idRoot;
}


const ID &ID::getTerrainDOMLayerRootID(void)
{
    static const ID idRoot(gs_nLogicalLayerCode, TERRAIN_DOM_LAYER_ID, NULL);
    return idRoot;
}


const ID &ID::getVirtualTileRootID(void)
{
    static const ID idRoot(gs_nVirtualTileCode, VIRTUAL_CUBE, NULL);
    return idRoot;
}

const ID &ID::getSymbolCategoryRootID(void)
{
    static const ID idRoot(gs_nLogicalLayerCode, SYMBOL_CATEGORY_ID, NULL);
    return idRoot;
}

ID::ID(const void *pBlock, unsigned nLength)
{
    set(pBlock, nLength);
}


ID::ID(UINT_64 nHighBit, UINT_64 nMidBit, UINT_64 nLowBit)
{
    set(nHighBit, nMidBit, nLowBit);
}

ID::ID(UINT_32 nDataSetCode, DeuObjectIDType eType, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_64 nUniqueID)
{
    set(nDataSetCode, eType, nLevel, nRow, nCol, nUniqueID);
}

ID::ID(UINT_32 nDataSetCode, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_32 nHeight)
{
    set(nDataSetCode, nLevel, nRow, nCol, nHeight);
}

ID::ID(UINT_32 nDataSetCode, DeuObjectIDType eType, const unsigned char szModelID[])
{
    set(nDataSetCode, eType, szModelID);
}

ID::ID(UINT_32 nDataSetCode, DeuObjectIDType eType)
{
#ifdef WIN32
    ::CoCreateGuid((GUID *)ObjectID.m_UniqueID);
    set(nDataSetCode, eType, ObjectID.m_UniqueID);
#endif
}

ID::ID(const ID &id)
{
    set(id);
}


ID ID::genNewID(void)
{
    ID id;
    id.generate();
    return id;
}


ID ID::genIDfromString(const std::string &strID)
{
    ID id;
    id.fromString(strID);
    return id;
}

ID ID::genIDfromBinary(const void *pBinData, unsigned int nBinDataLen)
{
    ID id(pBinData, nBinDataLen);
    return id;
}

void ID::generate(void)
{
#ifdef WIN32
    m_nHighBit = 0;
    m_nMidBit  = 0;
    m_nLowBit  = 0;
    ::CoCreateGuid((GUID *)ObjectID.m_UniqueID);
#endif
}


ID ID::tran2FindingID(void) const
{
    ID idForFind = *this;
    switch(ObjectID.m_nType)
    {
        case TERRAIN_DOM_ID:
        case TERRAIN_DEM_ID:
        case PARAM_POINT_ID:
        case PARAM_LINE_ID:
        case PARAM_FACE_ID:
            idForFind.ObjectID.m_nType = INST_ID_2_PARENT_ID;
            break;
        case CULTURE_LAYER_ID:
            idForFind.ObjectID.m_nType = CULTURE_LAYER_ID_2_PARENT_ID;
            break;
        case TERRAIN_DOM_LAYER_ID:
            idForFind.ObjectID.m_nType = DOM_LAYER_ID_2_PARENT_ID;
            break;
        case TERRAIN_DEM_LAYER_ID:
            idForFind.ObjectID.m_nType = DEM_LAYER_ID_2_PARENT_ID;
            break;
        default:
            idForFind.setInvalid();
            break;
    }
    return idForFind;
}


void ID::set(const ID &id)
{
    set(id.m_nHighBit, id.m_nMidBit, id.m_nLowBit);
}

#ifdef WIN32
void ID::set(const void *pBlock, unsigned nLength)
{
    const unsigned nLen = std::min(nLength, 24u);
    unsigned __int64    nTemp[3] = {~0ui64, ~0ui64, ~0ui64};
    memcpy(nTemp, pBlock, nLen);
    set(nTemp[2], nTemp[1], nTemp[0]);
}
#else
void ID::set(const void *pBlock, unsigned nLength)
{
    const unsigned nLen = std::min(nLength, 24u);
    UINT_64    nTemp[3] = {~0uLL, ~0uLL, ~0uLL};
    memcpy(nTemp, pBlock, nLen);
    set(nTemp[2], nTemp[1], nTemp[0]);
}
#endif

void ID::set(UINT_32 nDataSetCode, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, int nHeight)
{
    CubeID.m_nDataSetCode = nDataSetCode;
    CubeID.m_nType = VIRTUAL_CUBE;
    CubeID.m_nLevel = nLevel;
    CubeID.m_nRow = nRow;
    CubeID.m_nCol = nCol;
    CubeID.m_nHeight = nHeight;
    memset(CubeID.m_reserve, 0, 8);
}


void ID::set(UINT_32 nDataSetCode, DeuObjectIDType eType, UINT_32 nLevel, UINT_32 nRow, UINT_32 nCol, UINT_64 nUniqueID)
{
    TileID.m_nDataSetCode = nDataSetCode;
    TileID.m_nType = eType;
    TileID.m_nLevel = nLevel;
    TileID.m_nRow = nRow;
    TileID.m_nCol = nCol;
    TileID.m_nUniqueID = nUniqueID;
    memset(TileID.m_reserve, 0, 4);
}


void ID::set(UINT_32 nDataSetCode, DeuObjectIDType eType, const unsigned char szModelID[])
{
    ObjectID.m_nDataSetCode = nDataSetCode;
    ObjectID.m_nType = eType;
    memset(ObjectID.m_reserve, 0, 5);
    if(szModelID == NULL)
    {
        memset(ObjectID.m_UniqueID, 0, 16);
    }
    else
    {
        memcpy(ObjectID.m_UniqueID, szModelID, 16);
    }
}


void ID::fromString(const std::string &strID)
{
    if(strID.length() < 48u)   return;

    static unsigned int Table[256] = {0};
    static bool bTableInvalid = true;
    if(bTableInvalid)
    {
        Table['0'] = 0;     Table['1'] = 1;
        Table['2'] = 2;     Table['3'] = 3;
        Table['4'] = 4;     Table['5'] = 5;
        Table['6'] = 6;     Table['7'] = 7;
        Table['8'] = 8;     Table['9'] = 9;
        Table['A'] = 10;    Table['B'] = 11;
        Table['C'] = 12;    Table['D'] = 13;
        Table['E'] = 14;    Table['F'] = 15;
        bTableInvalid = false;
    }

    char *buffer = (char *)&ObjectID;
    const char *pch = strID.c_str();
    for(unsigned i = 0u; i < 24u; i++, buffer++)
    {
        BitFour *pBits = (BitFour *)buffer;
        pBits->HighBit = Table[*pch++];
        pBits->LowBit  = Table[*pch++];
    }
}


std::string ID::toString(void) const
{
    const static char g_ch[16] = {'0','1','2','3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    char ch[50] = {0};
    char *pch = ch;
    const char *buffer = (const char *)&ObjectID;

    for(unsigned i = 0; i < 24u; i++, buffer++)
    {
        const BitFour *pBits = (const BitFour *)buffer;
        *pch++ = g_ch[pBits->HighBit];
        *pch++ = g_ch[pBits->LowBit];
    }

    return ch;
}


