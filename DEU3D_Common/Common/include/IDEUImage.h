#ifndef I_DEUIMAGE_H_A9931EB5_67EE_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_DEUIMAGE_H_A9931EB5_67EE_41AD_A22C_BA4A2931ACBA_INCLUDE

#include "Export.h"
#include <vector>
#include <OpenSP/Ref.h>
#include "deuMath.h"
#include "DEUBson.h"
namespace cmm
{
namespace image
{
 enum ImageType
{
    TYPE_UNKNOWN,
    TYPE_PNG,
    TYPE_JPG,
    TYPE_DDS,
    TYPE_TIF,
    TYPE_BMP,
    TYPE_DEU_HEIGHT_FIELD
};

enum PixelFormat
{
    PF_RGB,
    PF_RGBA,
    PF_LUMINANCE,
};


///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-29
//  功能简介：Image的抽象基类       
//
///////////////////////////////////////////////////////////
class IDEUImage : virtual public OpenSP::Ref
{
public:
    virtual bool        allocImage(unsigned nWidth, unsigned nHeight, PixelFormat eFormat, float fltNullLuminance = -1e5) = 0;

    virtual bool        isValid(void) const = 0;
    virtual unsigned    getWidth(void) const = 0;
    virtual unsigned    getHeight(void) const = 0;
    virtual unsigned    getPixelSizeInByte(void) const = 0;
    virtual unsigned    getLineSizeInByte(void) const = 0;
    virtual unsigned    getImageSizeInByte(void) const = 0;

    virtual const void *data(void) const = 0;
    virtual void       *data(void) = 0;

    virtual bool        attach(void *pData, unsigned nWidth, unsigned nHeight, PixelFormat eFormat) = 0;
    virtual void        deatch(void) = 0;

    virtual IDEUImage  *scaleImage(unsigned nNewWidth, unsigned nNewHeight) = 0;
    virtual bool        scaleImageByArea(const cmm::math::Box2d &bbTotal, const cmm::math::Box2d &bbArea) = 0;
    virtual IDEUImage  *getSubImage(unsigned nOffsetX, unsigned nOffsetY, unsigned nWidth, unsigned nHeight) const = 0;

    virtual bool        convoluteImage(const double mtxConKernel[3][3]) = 0;

    virtual bool        jointImage(const IDEUImage* pImage,unsigned nSrcX,unsigned nSrcY,unsigned nDesX,unsigned nDesY,unsigned nWidth,unsigned nHeight) = 0;
    virtual PixelFormat getPixelFormat() const = 0;

    virtual bool        hasAlpha(void) const = 0;
    virtual void        clearAlphaAsColor(unsigned char r, unsigned char g, unsigned char b) = 0;
    virtual void        clearAlphaAsColor(float flt) = 0;
    virtual void        deleteAlpha() = 0;

    virtual bool        saveToFile(const std::string &strFilePath) const = 0;

    virtual bool        swapRedAndBlueChanel()      = 0;
    
    virtual bool        flipVertical()               = 0;

    virtual bool fromBson(bson::bsonElement &val) = 0;

    virtual void toBson(bson::bsonElement &val) = 0;
};

CM_EXPORT IDEUImage *createDEUImage(void);
}
}


#endif