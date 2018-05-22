#ifndef DEU_IMAGE_H_A38B3C1F_25EB_40F7_9599_F69D5B1C564E_INCLUDE
#define DEU_IMAGE_H_A38B3C1F_25EB_40F7_9599_F69D5B1C564E_INCLUDE

#include "Export.h"
#include <deuMath.h>
#include <vector>
#include <OpenSP/Ref.h>
#include <IDEUImage.h>
#include <DEUBson.h>
namespace cmm{ namespace image
{



template<typename T>
inline T linearInterpolation(T dblLB, T dblRB, T dblLT, T dblRT, double dblU, double dblV)
{
    const T dblValue =
          dblLB * (1.0 - dblU) * (1.0 - dblV)
        + dblRB * (      dblU) * (1.0 - dblV)
        + dblLT * (1.0 - dblU) * (      dblV)
        + dblRT * (      dblU) * (      dblV);
    return dblValue;
}

struct PixelData
{
    unsigned  LowBit : 4;
    unsigned  HighBit : 4;
};

class CM_EXPORT Image : public IDEUImage
{
public:
    Image(void);
    ~Image(void);

    bool        allocImage(unsigned nWidth, unsigned nHeight, PixelFormat eFormat, float fltNullLuminance = -1e5);

    bool        isValid(void) const;
    unsigned    getWidth(void) const        {   return m_uWidth;    }
    unsigned    getHeight(void) const       {   return m_uHeight;   }
    unsigned    getPixelSizeInByte(void) const;
    unsigned    getLineSizeInByte(void) const;
    unsigned    getImageSizeInByte(void) const;

    const void *data(void) const    {   return m_pData; }
    void       *data(void)          {   return m_pData; }

    bool    attach(void *pData, unsigned nWidth, unsigned nHeight, PixelFormat eFormat);
    void    deatch(void);

    IDEUImage  *scaleImage(unsigned nNewWidth, unsigned nNewHeight);
    bool    scaleImageByArea(const cmm::math::Box2d &bbTotal, const cmm::math::Box2d &bbArea);
    IDEUImage  *getSubImage(unsigned nOffsetX, unsigned nOffsetY, unsigned nWidth, unsigned nHeight) const;

    bool    convoluteImage(const double mtxConKernel[3][3]);

    bool    hasAlpha(void) const;
    void    clearAlphaAsColor(unsigned char r, unsigned char g, unsigned char b);
    void    clearAlphaAsColor(float flt);
    void    deleteAlpha();

    bool    blendImage(const Image &image);
    bool    jointImage(const IDEUImage* pImage,unsigned nSrcX,unsigned nSrcY,unsigned nDesX,unsigned nDesY,unsigned nWidth,unsigned nHeight);
    PixelFormat getPixelFormat() const {return m_eFormat;}

    bool    saveToFile(const std::string &strFilePath) const;

public:
    bool    swapRedAndBlueChanel();
    
    bool flipVertical();

    bool fromBson(bson::bsonElement &val);

    void toBson(bson::bsonElement &val);


protected:
    void    releaseImage(void);
    void    buildImageMatrix(void);

protected:
    void       *                m_pData;
    unsigned                    m_uWidth;
    unsigned                    m_uHeight;
    bool                        m_bIgnoreInvalidValue;
    std::vector<void *>         m_mtxImagePixel;
    PixelFormat                 m_eFormat;
    float                       m_fNullLuminance;
    bool                        m_bAttached;
};

CM_EXPORT ImageType getImageStreamType(const void *pStream);

CM_EXPORT Image *scaleImageByArea(const Image &imageSrc, const cmm::math::Box2d &bbFrom, const cmm::math::Box2d &bbTo);

}}

#endif
