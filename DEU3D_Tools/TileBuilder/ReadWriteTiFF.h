#pragma once

#include <string.h>
#include <iostream>

typedef unsigned short uint16;

enum GDTType
{
    GDTType_Unknown   = 0,
    GDTType_Byte      = 1,
    GDTType_UInt16    = 2,
    GDTType_Int16     = 3,
    GDTType_UInt32    = 4,
    GDTType_Int32     = 5,
    GDTType_Float32   = 6,
    GDTType_Float64   = 7,
    GDTType_CInt16    = 8,
    GDTType_CInt32    = 9,
    GDTType_CFloat32  = 10,
    GDTType_CFloat64  = 11,
    GDTType_TypeCount = 12
};

enum PixelType
{
    PixelType_DEPTH_COMPONENT = 0,
    PixelType_LUMINANCE       = 1,
    PixelType_ALPHA           = 2,
    PixelType_LUMINANCE_ALPHA = 3,
    PixelType_RGB             = 4,
    PixelType_RGBA            = 5
};

class ReadWriteTiFF
{
public:
    ReadWriteTiFF(void);
    ~ReadWriteTiFF(void);

public:
    unsigned char* readTIFStream(std::istream& fin, PixelType &nPixelType) const;
    bool           writeTIFStream(std::ostream& fout, const char* pBmpBuf, const PixelType nPixelType, const GDTType nType, const int nXSize, const int nYSize, const bool bImage) const;
};

