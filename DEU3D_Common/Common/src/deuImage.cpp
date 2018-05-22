#include <deuImage.h>
#include <memory.h>
#include <stdio.h>
#include <OpenSP/sp.h>
#include <xutility>
#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

#pragma warning( disable : 4996 )


#define BSON_IMAGE_WIDTH            "Width"
#define BSON_IMAGE_HEIGHT           "Height"
#define BSON_IMAGE_DATALEN          "DataLen"
#define BSON_IMAGE_DATA             "Data"

namespace cmm{ namespace image
{
IDEUImage *createDEUImage(void)
{
    return new Image();
}
ImageType getImageStreamType(const void *pStream)
{
    if(!pStream)
    {
        return TYPE_UNKNOWN;
    }

    const unsigned char headerJPG[3] ={ 0xff, 0xD8, 0xFF };
    if(memcmp(pStream, headerJPG, sizeof(headerJPG)) == 0)
    {
        return TYPE_JPG;
    }

    const unsigned char headerPNG[4] ={ 0x89, 0x50, 0x4E, 0x47 };
    if(memcmp(pStream, headerPNG, sizeof(headerPNG)) == 0)
    {
        return TYPE_PNG;
    }

    const unsigned char headerDDS[4] ={ 0x44, 0x44, 0x53, 0x20 };
    if(memcmp(pStream, headerDDS, sizeof(headerDDS)) == 0)
    {
        return TYPE_DDS;
    }

    const unsigned char headerTIF1[4] = { 0x49, 0x49, 0x2A, 0x00 };
    const unsigned char headerTIF2[4] = { 0x4D, 0x4D, 0x00, 0x2A };
    if(memcmp(pStream, headerTIF1, sizeof(headerTIF1)) == 0 ||
        memcmp(pStream, headerTIF2, sizeof(headerTIF2)) == 0)
    {
        return TYPE_TIF;
    }

    const unsigned char headerBMP[2] = { 0x42, 0x4D };
    if(memcmp(pStream, headerBMP, sizeof(headerBMP)) == 0)
    {
        return TYPE_BMP;
    }

    const unsigned char headerDEUHeightField[4] = {0x00, 0x00, 0x00, 0x00};
    if(memcmp(pStream, headerDEUHeightField, sizeof(headerDEUHeightField)) == 0)
    {
        return TYPE_DEU_HEIGHT_FIELD;
    }

    return TYPE_UNKNOWN;
}

Image::Image(void)
{
    m_pData = NULL;
    m_uHeight = 0u;
    m_uWidth  = 0u;
    m_fNullLuminance = -1e5;
    m_bAttached = false;
    m_eFormat = PF_RGBA;
    m_bIgnoreInvalidValue = true;
}


Image::~Image(void)
{
    releaseImage();
}


bool Image::isValid(void) const 
{
    if(!m_pData)        return false;
    if(m_uWidth < 1u)   return false;
    if(m_uHeight < 1u)  return false;
    return true;
}


void Image::releaseImage(void)
{
    if(!isValid())  return;

    if(m_bAttached)
    {
        m_pData = NULL;
    }
    else
    {
        if(m_pData)
        {
            delete[] m_pData;
            m_pData = NULL;
        }
    }
    m_uWidth = 0u;
    m_uHeight = 0u;
}


bool Image::attach(void *pData, unsigned nWidth, unsigned nHeight, PixelFormat eFormat)
{
    if(!pData)  return false;
    if(nWidth < 1u || nHeight < 1u) return false;

    releaseImage();

    m_pData   = pData;
    m_uHeight = nHeight;
    m_uWidth  = nWidth;
    m_eFormat = eFormat;
    m_bAttached = true;
    m_bIgnoreInvalidValue = true;
    buildImageMatrix();

    return true;
}


void Image::deatch(void)
{
    releaseImage();
}


void Image::buildImageMatrix(void)
{
    unsigned char *pData = (unsigned char *)m_pData;

    const unsigned nLineSize = getLineSizeInByte();
    m_mtxImagePixel.resize(m_uHeight);
    for(unsigned y = 0u; y < m_uHeight; y++)
    {
        m_mtxImagePixel[y] = pData;
        pData += nLineSize;
    }
}


unsigned Image::getPixelSizeInByte(void) const
{
    switch(m_eFormat)
    {
    case PF_RGB:
        return 3;
    case PF_RGBA:
        return 4;
    case PF_LUMINANCE:
        return 4;
    }

    return 0;
}


unsigned Image::getLineSizeInByte(void) const
{
    const unsigned uPixelSize = getPixelSizeInByte();
    if(uPixelSize < 1u) return 0;

    unsigned nLineSize = uPixelSize * m_uWidth;
    const unsigned nBias = nLineSize % 4;
    if(nBias == 0u) return nLineSize;

    nLineSize += 4 - nBias;
    return nLineSize;
}


unsigned Image::getImageSizeInByte(void) const
{
    const unsigned uLineSize = getLineSizeInByte();
    if(uLineSize < 1u)  return 0u;
    return uLineSize * m_uHeight;
}


bool Image::hasAlpha(void) const
{
    if(!isValid())  return false;

    const unsigned int nPixelSize = getPixelSizeInByte();
    if(m_eFormat == PF_RGBA)
    {
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            const unsigned char *pPixelData = (const unsigned char *)m_mtxImagePixel[y];
            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                if (pPixelData[3] < 128)
                {
                    return true;
                }
                pPixelData += nPixelSize;
            }
        }
    }
    else if(m_eFormat == PF_LUMINANCE)
    {
        const float *pPixelData = (const float *)m_pData;
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                if(*pPixelData <= m_fNullLuminance)
                {
                    return true;
                }
                pPixelData++;
            }
        }
    }

    return false;
}


void Image::clearAlphaAsColor(unsigned char r, unsigned char g, unsigned char b)
{
    if(!isValid())  return;
    if(m_eFormat != PF_RGBA)    return;

    for(unsigned y = 0u; y < m_uHeight; y++)
    {
        unsigned char *pPixelData = (unsigned char *)m_mtxImagePixel[y];
        for(unsigned x = 0u; x < m_uWidth; x++)
        {
            if(pPixelData[3] != 255)
            {
                pPixelData[0] = r;
                pPixelData[1] = g;
                pPixelData[2] = b;
            }
            pPixelData[3] = 255;
            pPixelData += 4u;
        }
    }
}


void Image::clearAlphaAsColor(float flt)
{
    if(!isValid())  return;
    if(m_eFormat != PF_LUMINANCE)   return;

    float *pPixelData = (float *)m_pData;
    for(unsigned y = 0u; y < m_uHeight; y++)
    {
        for(unsigned x = 0u; x < m_uWidth; x++)
        {
            if(*pPixelData <= m_fNullLuminance)
            {
                *pPixelData = flt;
            }
            pPixelData++;
        }
    }
}


void Image::deleteAlpha()
{
    if(!isValid() || m_bAttached)   return;
    if(m_eFormat != PF_RGBA)        return;

    m_eFormat = PF_RGB;
    const unsigned nLineSize = getLineSizeInByte();
    unsigned char* pData = new unsigned char[m_uHeight * nLineSize];

    unsigned char *pDesLineData = pData;
    for(unsigned y = 0u; y < m_uHeight; y++)
    {
        unsigned char *pSrcLineData = static_cast<unsigned char*>(m_mtxImagePixel[y]);

        unsigned char *pTmp = pDesLineData;
        for(unsigned x =  0u; x < m_uWidth; x++)
        {
            pTmp[0] = pSrcLineData[0];
            pTmp[1] = pSrcLineData[1];
            pTmp[2] = pSrcLineData[2];

            pTmp += 3;
            pSrcLineData += 4;
        }
        pDesLineData += nLineSize;
    }

    delete m_pData;
    m_pData = pData;

    buildImageMatrix();

    return;
}


bool Image::jointImage(const IDEUImage* pImage,unsigned nSrcX,unsigned nSrcY,unsigned nDesX,unsigned nDesY,unsigned nWidth,unsigned nHeight)
{
    if(!isValid())      return false;
    if(!pImage->isValid()) return false;

    PixelFormat pFormat = pImage->getPixelFormat();
    if(pFormat != m_eFormat) return false;
    if(m_eFormat == PF_RGB)
    {
        const unsigned nLineSize      = pImage->getLineSizeInByte();
        const unsigned nLinePixelSize = pImage->getPixelSizeInByte() * pImage->getWidth();
        const unsigned nLineReserved  = nLineSize - nLinePixelSize;

        unsigned char *pDesPixel = (unsigned char *)m_pData + (nDesY*nLineSize + nDesX*3);
        unsigned char *pSrcPixel = (unsigned char *)pImage->data() + (nSrcY*nLineSize + nSrcX*3);
        for(unsigned y = nSrcY; y < nSrcY + nHeight; y++)
        {
            for(unsigned x = nSrcX; x < nSrcX + nWidth; x++)
            {
                pDesPixel[0] = pSrcPixel[0];
                pDesPixel[1] = pSrcPixel[1];
                pDesPixel[2] = pSrcPixel[2];
                pDesPixel += 3;
                pSrcPixel += 3;
            }
            pDesPixel += nLineSize - (nDesX + nWidth)*3 + nDesX*3;
            pSrcPixel += nLineSize - (nSrcX + nWidth)*3 + nSrcX*3;
        }
    }
    else if(m_eFormat == PF_RGBA)
    {
        const unsigned nLineSize      = pImage->getLineSizeInByte();
        const unsigned nLinePixelSize = pImage->getPixelSizeInByte() * pImage->getWidth();
        const unsigned nLineReserved  = nLineSize - nLinePixelSize;

        unsigned char *pDesPixel = (unsigned char *)m_pData + (nDesY*nLineSize + nDesX*4);
        const unsigned char *pSrcPixel = (const unsigned char *)pImage->data() + (nSrcY*nLineSize + nSrcX*4);
        for(unsigned y = nSrcY; y < nSrcY + nHeight; y++)
        {
            for(unsigned x = nSrcX; x < nSrcX + nWidth; x++)
            {
                pDesPixel[0] = pSrcPixel[0];
                pDesPixel[1] = pSrcPixel[1];
                pDesPixel[2] = pSrcPixel[2];
                pDesPixel[3] = pSrcPixel[3];
                pDesPixel += 4;
                pSrcPixel += 4;
            }
            pDesPixel += nLineSize - (nDesX + nWidth)*4;
            pSrcPixel += nLineSize - (nSrcX + nWidth)*4;
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool Image::blendImage(const Image &image)
{
    if(!isValid())      return false;
    if(!image.isValid()) return false;

    if(m_uWidth != image.m_uWidth || m_uHeight != image.m_uHeight)
    {
        return false;
    }

    if(image.m_eFormat == PF_RGB)
    {
        if(m_eFormat == PF_RGB)
        {
            const unsigned uImageSize = getImageSizeInByte();
            memcpy(m_pData, image.m_pData, uImageSize);
            return true;
        }
        else if(m_eFormat == PF_RGBA)
        {
            const unsigned nLineSize      = image.getLineSizeInByte();
            const unsigned nLinePixelSize = image.getPixelSizeInByte() * image.getWidth();
            const unsigned nLineReserved  = nLineSize - nLinePixelSize;

            unsigned char *pDesPixel = (unsigned char *)m_pData;
            const unsigned char *pSrcPixel = (const unsigned char *)image.m_pData;
            for(unsigned y = 0u; y < m_uHeight; y++)
            {
                for(unsigned x = 0u; x < m_uWidth; x++)
                {
                    pDesPixel[0] = pSrcPixel[0];
                    pDesPixel[1] = pSrcPixel[1];
                    pDesPixel[2] = pSrcPixel[2];
                    pDesPixel[3] = 255;
                    pDesPixel += 4;
                    pSrcPixel += 3;
                }
                pSrcPixel += nLineReserved;
            }
            return true;
        }
        else return false;
    }
    else if(image.m_eFormat == PF_RGBA)
    {
        if(m_eFormat == PF_RGBA)
        {
            unsigned char *pDesPixel = (unsigned char *)m_pData;
            const unsigned char *pSrcPixel = (const unsigned char *)image.m_pData;
            for(unsigned y = 0u; y < m_uHeight; y++)
            {
                for(unsigned x = 0u; x < m_uWidth; x++)
                {
                    const float fltAlpha = pSrcPixel[3] / 255.0f;
                    pDesPixel[0] = pDesPixel[0] * (1.0 - fltAlpha) + pSrcPixel[0] * fltAlpha;
                    pDesPixel[1] = pDesPixel[1] * (1.0 - fltAlpha) + pSrcPixel[1] * fltAlpha;
                    pDesPixel[2] = pDesPixel[2] * (1.0 - fltAlpha) + pSrcPixel[2] * fltAlpha;
                    pDesPixel[3] = pDesPixel[3] * (1.0 - fltAlpha) + pSrcPixel[3] * fltAlpha;   // YJS ERROR
                    pDesPixel += 4;
                    pSrcPixel += 4;
                }
            }
        }
        else if(m_eFormat == PF_RGB)
        {
            const unsigned nLineSize      = image.getLineSizeInByte();
            const unsigned nLinePixelSize = image.getPixelSizeInByte() * image.getWidth();
            const unsigned nLineReserved  = nLineSize - nLinePixelSize;

            unsigned char *pDesPixel = (unsigned char *)m_pData;
            const unsigned char *pSrcPixel = (const unsigned char *)image.m_pData;
            for(unsigned y = 0u; y < m_uHeight; y++)
            {
                for(unsigned x = 0u; x < m_uWidth; x++)
                {
                    const float fltAlpha = pSrcPixel[3] / 255.0f;
                    pDesPixel[0] = pDesPixel[0] * (1.0 - fltAlpha) + pSrcPixel[0] * fltAlpha;
                    pDesPixel[1] = pDesPixel[1] * (1.0 - fltAlpha) + pSrcPixel[1] * fltAlpha;
                    pDesPixel[2] = pDesPixel[2] * (1.0 - fltAlpha) + pSrcPixel[2] * fltAlpha;
                    pDesPixel += 3;
                    pSrcPixel += 4;
                }
                pDesPixel += nLineReserved;
            }
        }
        else return false;
    }
    else if(image.m_eFormat == PF_LUMINANCE)
    {
        if(m_eFormat != PF_LUMINANCE)
        {
            return false;
        }

        float *pDesPixel = (float *)m_pData;
        const float *pSrcPixel = (const float *)image.m_pData;
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                //const bool bBlend = (*pSrcPixel < image.m_fNullLuminance);
                //if(!bBlend)
				const bool bBlend = (*pSrcPixel > image.m_fNullLuminance);
				if(bBlend)
                {
                    *pDesPixel = *pSrcPixel;
                }

                ++pDesPixel;
                ++pSrcPixel;
            }
        }
        return true;
    }
    else return false;

    return true;
}


bool Image::allocImage(unsigned nWidth, unsigned nHeight, PixelFormat eFormat, float fltNullLuminance /*= -1e5*/)
{
    if(nWidth < 1u || nHeight < 1u)
    {
        return false;
    }

    releaseImage();

    m_eFormat = eFormat;
    m_uWidth  = nWidth;
    m_uHeight = nHeight;
    m_fNullLuminance = fltNullLuminance;
    m_bAttached = false;
    const unsigned nImageSize = getImageSizeInByte();
    m_pData = new unsigned char[nImageSize];

    buildImageMatrix();
    return true;
}


IDEUImage *Image::getSubImage(unsigned nOffsetX, unsigned nOffsetY, unsigned nWidth, unsigned nHeight) const
{
    if(!isValid())  return NULL;
    if(nOffsetX >= m_uWidth || nOffsetY >= m_uHeight)
    {
        return NULL;
    }
    if(nWidth < 1u || nHeight < 1u)
    {
        return NULL;
    }

    nWidth  = std::min(nWidth, m_uWidth - nOffsetX);
    nHeight = std::min(nHeight, m_uHeight - nOffsetY);
    if(nWidth < 1u || nHeight < 1u)
    {
        return NULL;
    }

    OpenSP::sp<Image> pImage = new Image;
    pImage->allocImage(nWidth, nHeight, m_eFormat, m_fNullLuminance);

    const unsigned nDesLineSize  = pImage->getLineSizeInByte();

    const unsigned nSrcPixelSize = getPixelSizeInByte();
    const unsigned nSrcLineSize  = getLineSizeInByte();
    for(unsigned y = 0u; y < nHeight; y++)
    {
        const unsigned nOffset = nOffsetX * nSrcPixelSize;
        const void *pPixelSrc = (unsigned char *)(m_mtxImagePixel[y + nOffsetY]) + nOffset;
        void *pPixelDes = pImage->m_mtxImagePixel[y];
        memcpy(pPixelDes, pPixelSrc, nDesLineSize);
    }

    return pImage.release();
}


IDEUImage *Image::scaleImage(unsigned nNewWidth, unsigned nNewHeight)
{
    if(!isValid())  return NULL;
    if(nNewWidth < 1u || nNewHeight < 1u)
    {
        return NULL;
    }

    const double dblScaleWidth  = double(nNewWidth)  / double(m_uWidth);
    const double dblScaleHeight = double(nNewHeight) / double(m_uHeight);

    Image  *pNewImage = new Image;
    pNewImage->allocImage(nNewWidth, nNewHeight, m_eFormat, m_fNullLuminance);

    const unsigned nPixelSize    = getPixelSizeInByte();
    const unsigned nNewPixelSize = pNewImage->getPixelSizeInByte();

    if(m_eFormat == PF_RGBA)
    {
        for(unsigned y = 0u; y < nNewHeight; y++)
        {
            const double   dblScaleY = y / dblScaleHeight;
            const unsigned nTop      = cmm::math::clampBelow((unsigned)ceil(dblScaleY),  m_uHeight - 1u);
            const unsigned nBottom   = cmm::math::clampBelow((unsigned)floor(dblScaleY), m_uHeight - 1u);
            const double   dblV      = dblScaleY - nBottom;

            for(unsigned x = 0u; x < nNewWidth; x++)
            {
                const double   dblScaleX = x / dblScaleWidth;
                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblScaleX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblScaleX), m_uWidth - 1u);
                const double   dblU      = dblScaleX - nLeft;

                const unsigned char *pLB = (const unsigned char *)m_mtxImagePixel[nBottom] + nLeft  * nPixelSize;
                const unsigned char *pRB = (const unsigned char *)m_mtxImagePixel[nBottom] + nRight * nPixelSize;
                const unsigned char *pLT = (const unsigned char *)m_mtxImagePixel[nTop]    + nLeft  * nPixelSize;
                const unsigned char *pRT = (const unsigned char *)m_mtxImagePixel[nTop]    + nRight * nPixelSize;

                unsigned char r = linearInterpolation(pLB[0], pRB[0], pLT[0], pRT[0], dblU, dblV);
                unsigned char g = linearInterpolation(pLB[1], pRB[1], pLT[1], pRT[1], dblU, dblV);
                unsigned char b = linearInterpolation(pLB[2], pRB[2], pLT[2], pRT[2], dblU, dblV);

                unsigned char *pColor = (unsigned char *)pNewImage->m_mtxImagePixel[y] + x * nNewPixelSize;
                pColor[0] = r;
                pColor[1] = g;
                pColor[2] = b;

                if(dblV < 0.5)
                {
                    if(dblU < 0.5)
                    {
                        pColor[3] = pLB[3];
                    }
                    else
                    {
                        pColor[3] = pRB[3];
                    }
                }
                else
                {
                    if(dblU < 0.5)
                    {
                        pColor[3] = pLT[3];
                    }
                    else
                    {
                        pColor[3] = pRT[3];
                    }
                }
            }
        }
    }
    else if(m_eFormat == PF_RGB)
    {
        for(unsigned y = 0u; y < nNewHeight; y++)
        {
            const double   dblScaleY = y / dblScaleHeight;
            const unsigned nTop      = cmm::math::clampBelow((unsigned)ceil(dblScaleY),  m_uHeight - 1u);
            const unsigned nBottom   = cmm::math::clampBelow((unsigned)floor(dblScaleY), m_uHeight - 1u);
            const double   dblV      = dblScaleY - nBottom;

            for(unsigned x = 0u; x < nNewWidth; x++)
            {
                const double   dblScaleX = x / dblScaleWidth;
                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblScaleX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblScaleX), m_uWidth - 1u);
                const double   dblU      = dblScaleX - nLeft;

                const unsigned char *pLB = (const unsigned char *)m_mtxImagePixel[nBottom] + nLeft  * nPixelSize;
                const unsigned char *pRB = (const unsigned char *)m_mtxImagePixel[nBottom] + nRight * nPixelSize;
                const unsigned char *pLT = (const unsigned char *)m_mtxImagePixel[nTop]    + nLeft  * nPixelSize;
                const unsigned char *pRT = (const unsigned char *)m_mtxImagePixel[nTop]    + nRight * nPixelSize;

                unsigned char r = linearInterpolation(pLB[0], pRB[0], pLT[0], pRT[0], dblU, dblV);
                unsigned char g = linearInterpolation(pLB[1], pRB[1], pLT[1], pRT[1], dblU, dblV);
                unsigned char b = linearInterpolation(pLB[2], pRB[2], pLT[2], pRT[2], dblU, dblV);

                unsigned char *pColor = (unsigned char *)pNewImage->m_mtxImagePixel[y] + x * nNewPixelSize;
                pColor[0] = r;
                pColor[1] = g;
                pColor[2] = b;
            }
        }
    }
    else if(m_eFormat == PF_LUMINANCE)
    {
        for(unsigned y = 0u; y < nNewHeight; y++)
        {
            const double   dblScaleY = y / dblScaleHeight;
            const unsigned nTop      = cmm::math::clampBelow((unsigned)ceil(dblScaleY),  m_uHeight - 1u);
            const unsigned nBottom   = cmm::math::clampBelow((unsigned)floor(dblScaleY), m_uHeight - 1u);
            const double   dblV      = dblScaleY - nBottom;

            for(unsigned x = 0u; x < nNewWidth; x++)
            {
                const double   dblScaleX = x / dblScaleWidth;
                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblScaleX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblScaleX), m_uWidth - 1u);
                const double   dblU      = dblScaleX - nLeft;

                float fltLB = *((const float *)m_mtxImagePixel[nBottom] + nLeft);
                float fltRB = *((const float *)m_mtxImagePixel[nBottom] + nRight);
                float fltLT = *((const float *)m_mtxImagePixel[nTop]    + nLeft);
                float fltRT = *((const float *)m_mtxImagePixel[nTop]    + nRight);

                if(fltLB < pNewImage->m_fNullLuminance)     fltLB = 0.0f;
                if(fltRB < pNewImage->m_fNullLuminance)     fltRB = 0.0f;
                if(fltLT < pNewImage->m_fNullLuminance)     fltLT = 0.0f;
                if(fltRT < pNewImage->m_fNullLuminance)     fltRT = 0.0f;

                const float color = linearInterpolation(fltLB, fltRB, fltLT, fltRT, dblU, dblV);

                float *pColor = (float *)pNewImage->m_mtxImagePixel[y] + x;
                *pColor = color;
            }
        }
    }

    return pNewImage;
}


bool Image::scaleImageByArea(const cmm::math::Box2d &bbTotal, const cmm::math::Box2d &bbArea)
{
    if(!isValid())  return false;

    const cmm::math::Point2d ptLBTotal = bbTotal.corner(cmm::math::Box2d::LeftBottom);
    const cmm::math::Point2d ptLBArea  = bbArea.corner(cmm::math::Box2d::LeftBottom);

    const double dblTotalWidth = bbTotal.width();
    if(cmm::math::floatEqual(dblTotalWidth, 0.0))
    {
        return false;
    }

    const double dblTotalHeight = bbTotal.height();
    if(cmm::math::floatEqual(dblTotalHeight, 0.0))
    {
        return false;
    }

    const double dblAreaWidth = bbArea.width();
    if(cmm::math::floatEqual(dblAreaWidth, 0.0))
    {
        return false;
    }

    const double dblAreaHeight = bbArea.height();
    if(cmm::math::floatEqual(dblAreaHeight, 0.0))
    {
        return false;
    }

    const unsigned nPixelSize = getPixelSizeInByte();
    const unsigned nLineSize  = getLineSizeInByte();
    const unsigned nImageSize = getImageSizeInByte();
    if(m_eFormat == PF_RGBA)
    {
        std::vector<unsigned char> vecNewData(nImageSize, 0);
        unsigned char *pData = vecNewData.data();
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            double dblPosY = double(y) / double(m_uHeight);
            dblPosY *= dblAreaHeight;
            dblPosY += ptLBArea.y();
            dblPosY -= ptLBTotal.y();
            dblPosY /= dblTotalHeight;
            dblPosY *= m_uHeight;
            if(dblPosY < 0.0 || dblPosY >= m_uHeight)
            {
                continue;
            }

            const unsigned nTop      = cmm::math::clampBelow((unsigned)ceil(dblPosY),  m_uHeight - 1u);
            const unsigned nBottom   = cmm::math::clampBelow((unsigned)floor(dblPosY), m_uHeight - 1u);
            const double   dblV      = dblPosY - nBottom;

            unsigned char *pLineData = pData + y * nLineSize;
            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                double dblPosX = double(x) / double(m_uWidth);
                dblPosX *= dblAreaWidth;
                dblPosX += ptLBArea.x();
                dblPosX -= ptLBTotal.x();
                dblPosX /= dblTotalWidth;
                dblPosX *= m_uWidth;
                if(dblPosX < 0.0 || dblPosX >= m_uWidth)
                {
                    continue;
                }

                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblPosX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblPosX), m_uWidth - 1u);
                const double   dblU      = dblPosX - nLeft;

                const unsigned char *pLB = (const unsigned char *)m_mtxImagePixel[nBottom] + nLeft  * nPixelSize;
                const unsigned char *pRB = (const unsigned char *)m_mtxImagePixel[nBottom] + nRight * nPixelSize;
                const unsigned char *pLT = (const unsigned char *)m_mtxImagePixel[nTop]    + nLeft  * nPixelSize;
                const unsigned char *pRT = (const unsigned char *)m_mtxImagePixel[nTop]    + nRight * nPixelSize;

                const unsigned char r = linearInterpolation(pLB[0], pRB[0], pLT[0], pRT[0], dblU, dblV);
                const unsigned char g = linearInterpolation(pLB[1], pRB[1], pLT[1], pRT[1], dblU, dblV);
                const unsigned char b = linearInterpolation(pLB[2], pRB[2], pLT[2], pRT[2], dblU, dblV);
                const unsigned char a = linearInterpolation(pLB[3], pRB[3], pLT[3], pRT[3], dblU, dblV);

                unsigned char *pColor = pLineData + x * nPixelSize;
                pColor[0] = r;
                pColor[1] = g;
                pColor[2] = b;
                pColor[3] = a;
            }
        }

        memcpy(m_pData, pData, nImageSize);
    }
    else if(m_eFormat == PF_RGB)
    {
        std::vector<unsigned char> vecNewData(nImageSize, 0);
        unsigned char *pData = vecNewData.data();
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            double dblPosY = double(y) / double(m_uHeight);
            dblPosY *= dblAreaHeight;
            dblPosY += ptLBArea.y();
            dblPosY -= ptLBTotal.y();
            dblPosY /= dblTotalHeight;
            dblPosY *= m_uHeight;
            if(dblPosY < 0.0 || dblPosY >= m_uHeight)
            {
                continue;
            }

            const unsigned nTop      = cmm::math::clampBelow((unsigned)ceil(dblPosY),  m_uHeight - 1u);
            const unsigned nBottom   = cmm::math::clampBelow((unsigned)floor(dblPosY), m_uHeight - 1u);
            const double   dblV      = dblPosY - nBottom;
            unsigned char *pLineData = pData + y * nLineSize;

            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                double dblPosX = double(x) / double(m_uWidth);
                dblPosX *= dblAreaWidth;
                dblPosX += ptLBArea.x();
                dblPosX -= ptLBTotal.x();
                dblPosX /= dblTotalWidth;
                dblPosX *= m_uWidth;
                if(dblPosX < 0.0 || dblPosX >= m_uWidth)
                {
                    continue;
                }

                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblPosX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblPosX), m_uWidth - 1u);
                const double   dblU      = dblPosX - nLeft;

                const unsigned char *pLB = (const unsigned char *)m_mtxImagePixel[nBottom] + nLeft  * nPixelSize;
                const unsigned char *pRB = (const unsigned char *)m_mtxImagePixel[nBottom] + nRight * nPixelSize;
                const unsigned char *pLT = (const unsigned char *)m_mtxImagePixel[nTop]    + nLeft  * nPixelSize;
                const unsigned char *pRT = (const unsigned char *)m_mtxImagePixel[nTop]    + nRight * nPixelSize;

                const unsigned char r = linearInterpolation(pLB[0], pRB[0], pLT[0], pRT[0], dblU, dblV);
                const unsigned char g = linearInterpolation(pLB[1], pRB[1], pLT[1], pRT[1], dblU, dblV);
                const unsigned char b = linearInterpolation(pLB[2], pRB[2], pLT[2], pRT[2], dblU, dblV);

                unsigned char *pColor = pLineData + x * nPixelSize;
                pColor[0] = r;
                pColor[1] = g;
                pColor[2] = b;
            }
        }
        memcpy(m_pData, pData, nImageSize);
    }
    else if(m_eFormat == PF_LUMINANCE)
    {
        std::vector<float> vecNewData(m_uHeight * m_uWidth, m_fNullLuminance);
        float *pData = vecNewData.data();
        for(unsigned y = 0u; y < m_uHeight; y++)
        {
            double dblPosY = double(y) / double(m_uHeight);
            dblPosY *= dblAreaHeight;
            dblPosY += ptLBArea.y();
            dblPosY -= ptLBTotal.y();
            dblPosY /= dblTotalHeight;
            dblPosY *= m_uHeight - 1u;                  // 万分注意：这里需要减去1，因为16个像素只有15个空挡，不同于图像处理
            if(dblPosY < 0.0 || dblPosY >= m_uHeight)
            {
                continue;
            }

            const unsigned nTop    = cmm::math::clampBelow((unsigned)ceil(dblPosY),  m_uHeight - 1u);
            const unsigned nBottom = cmm::math::clampBelow((unsigned)floor(dblPosY), m_uHeight - 1u);
            const double   dblV    = dblPosY - nBottom;
            float *pLineData = pData + y * m_uWidth;

            for(unsigned x = 0u; x < m_uWidth; x++)
            {
                double dblPosX = double(x) / double(m_uWidth);
                dblPosX *= dblAreaWidth;
                dblPosX += ptLBArea.x();
                dblPosX -= ptLBTotal.x();
                dblPosX /= dblTotalWidth;
                dblPosX *= m_uWidth - 1u;               // 万分注意：这里需要减去1，因为16个像素只有15个空挡，不同于图像处理
                if(dblPosX < 0.0 || dblPosX >= m_uWidth)
                {
                    continue;
                }

                const unsigned nRight    = cmm::math::clampBelow((unsigned)ceil(dblPosX),  m_uWidth - 1u);
                const unsigned nLeft     = cmm::math::clampBelow((unsigned)floor(dblPosX), m_uWidth - 1u);
                const double   dblU      = dblPosX - nLeft;

                float fltLB = *((const float *)m_mtxImagePixel[nBottom] + nLeft);
                float fltRB = *((const float *)m_mtxImagePixel[nBottom] + nRight);
                float fltLT = *((const float *)m_mtxImagePixel[nTop]    + nLeft);
                float fltRT = *((const float *)m_mtxImagePixel[nTop]    + nRight);

				if(fltLB < m_fNullLuminance)    continue;//fltLB = 0.0f;
				if(fltRB < m_fNullLuminance)    continue;//fltRB = 0.0f;
				if(fltLT < m_fNullLuminance)    continue;//fltLT = 0.0f;
				if(fltRT < m_fNullLuminance)    continue;//fltRT = 0.0f;

                const float color = linearInterpolation(fltLB, fltRB, fltLT, fltRT, dblU, dblV);

                float *pColor = pLineData + x;
                *pColor = color;
            }
        }

        memcpy(m_pData, pData, nImageSize);
    }
    else return false;

    return true;
}


bool Image::convoluteImage(const double mtxConKernel[3][3])
{
    if(!isValid())  return false;

    if(!mtxConKernel)
    {
        return false;
    }

    const double dblConGain = 
        mtxConKernel[0][0] + mtxConKernel[0][1] + mtxConKernel[0][2] +
        mtxConKernel[1][0] + mtxConKernel[1][1] + mtxConKernel[1][2] +
        mtxConKernel[2][0] + mtxConKernel[2][1] + mtxConKernel[2][2];

    const unsigned nPixelSize = getPixelSizeInByte();
    const unsigned nLineSize  = getLineSizeInByte();
    const unsigned nImageSize = getImageSizeInByte();

    if(m_eFormat == PF_RGBA)
    {
        std::vector<unsigned char> vecNewData(nImageSize, 0);
        memcpy(vecNewData.data(), m_pData, nImageSize);

        for(unsigned y = 1u; y < m_uHeight - 1u; y++)
        {
            const unsigned char *pLastLineData = (const unsigned char *)m_mtxImagePixel[y - 1u];
            const unsigned char *pCurLineData  = (const unsigned char *)m_mtxImagePixel[y];
            const unsigned char *pNextLineData = (const unsigned char *)m_mtxImagePixel[y + 1u];

            unsigned char *pTargetLine = vecNewData.data() + nLineSize;

            for(unsigned x = 1u; x < m_uWidth - 1u; x++)
            {
                const unsigned nLeftOffset  = (x - 1u) * nPixelSize;
                const unsigned char *pLeftT = pLastLineData + nLeftOffset;
                const unsigned char *pLeft  = pCurLineData  + nLeftOffset;
                const unsigned char *pLeftB = pNextLineData + nLeftOffset;

                const unsigned nMidOffset  = x * nPixelSize;
                const unsigned char *pMidT = pLastLineData + nMidOffset;
                const unsigned char *pMid  = pCurLineData  + nMidOffset;
                const unsigned char *pMidB = pNextLineData + nMidOffset;

                const unsigned nRightOffset  = (x + 1u) * nPixelSize;
                const unsigned char *pRightT = pLastLineData + nRightOffset;
                const unsigned char *pRight  = pCurLineData  + nRightOffset;
                const unsigned char *pRightB = pNextLineData + nRightOffset;

                unsigned char *pTargetPos = pTargetLine + nMidOffset;
                for(unsigned n = 0; n < nPixelSize; n++)
                {
                    const double dblLeftT  = pLeftT[n]  * mtxConKernel[0][0];
                    const double dblLeft   = pLeft [n]  * mtxConKernel[1][0];
                    const double dblLeftB  = pLeftB[n]  * mtxConKernel[2][0];

                    const double dblMidT   = pMidT[n]   * mtxConKernel[0][1];
                    const double dblMid    = pMid [n]   * mtxConKernel[1][1];
                    const double dblMidB   = pMidB[n]   * mtxConKernel[2][1];

                    const double dblRightT = pRightT[n] * mtxConKernel[0][2];
                    const double dblRight  = pRight [n] * mtxConKernel[1][2];
                    const double dblRightB = pRightB[n] * mtxConKernel[2][2];

                    const double dblValue  = dblLeftT  + dblLeft  + dblLeftB
                                            + dblMidT   + dblMid   + dblMidB
                                            + dblRightT + dblRight + dblRightB;
                    pTargetPos[n] = (unsigned char)(dblValue + 0.5);
                }
        }
        }

        memcpy(m_pData, vecNewData.data(), nImageSize);
    }
    else if(m_eFormat == PF_RGB)
    {
        std::vector<unsigned char> vecNewData(nImageSize, 0);
        memcpy(vecNewData.data(), m_pData, nImageSize);

        for(unsigned y = 1u; y < m_uHeight - 1u; y++)
        {
            const unsigned char *pLastLineData = (const unsigned char *)m_mtxImagePixel[y - 1u];
            const unsigned char *pCurLineData  = (const unsigned char *)m_mtxImagePixel[y];
            const unsigned char *pNextLineData = (const unsigned char *)m_mtxImagePixel[y + 1u];

            unsigned char *pTargetLine = vecNewData.data() + nLineSize;

            for(unsigned x = 1u; x < m_uWidth - 1u; x++)
            {
                const unsigned nLeftOffset  = (x - 1u) * nPixelSize;
                const unsigned char *pLeftT = pLastLineData + nLeftOffset;
                const unsigned char *pLeft  = pCurLineData  + nLeftOffset;
                const unsigned char *pLeftB = pNextLineData + nLeftOffset;

                const unsigned nMidOffset  = x * nPixelSize;
                const unsigned char *pMidT = pLastLineData + nMidOffset;
                const unsigned char *pMid  = pCurLineData  + nMidOffset;
                const unsigned char *pMidB = pNextLineData + nMidOffset;

                const unsigned nRightOffset  = (x + 1u) * nPixelSize;
                const unsigned char *pRightT = pLastLineData + nRightOffset;
                const unsigned char *pRight  = pCurLineData  + nRightOffset;
                const unsigned char *pRightB = pNextLineData + nRightOffset;

                unsigned char *pTargetPos = pTargetLine + nMidOffset;
                for(unsigned n = 0; n < nPixelSize; n++)
                {
                    const double dblLeftT  = pLeftT[n]  * mtxConKernel[0][0];
                    const double dblLeft   = pLeft [n]  * mtxConKernel[1][0];
                    const double dblLeftB  = pLeftB[n]  * mtxConKernel[2][0];

                    const double dblMidT   = pMidT[n]   * mtxConKernel[0][1];
                    const double dblMid    = pMid [n]   * mtxConKernel[1][1];
                    const double dblMidB   = pMidB[n]   * mtxConKernel[2][1];

                    const double dblRightT = pRightT[n] * mtxConKernel[0][2];
                    const double dblRight  = pRight [n] * mtxConKernel[1][2];
                    const double dblRightB = pRightB[n] * mtxConKernel[2][2];

                    const double dblValue  = dblLeftT  + dblLeft  + dblLeftB
                                           + dblMidT   + dblMid   + dblMidB
                                           + dblRightT + dblRight + dblRightB;
                    pTargetPos[n] = (unsigned char)(dblValue + 0.5);
                }
            }
        }

        memcpy(m_pData, vecNewData.data(), nImageSize);
    }
    else if(m_eFormat == PF_LUMINANCE)
    {
        std::vector<float> vecNewData(m_uHeight * m_uWidth, 0);
        memcpy(vecNewData.data(), m_pData, nImageSize);

        for(unsigned y = 1u; y < m_uHeight - 1u; y++)
        {
            const float *pLastLineData = (const float *)m_mtxImagePixel[y - 1u];
            const float *pCurLineData  = (const float *)m_mtxImagePixel[y];
            const float *pNextLineData = (const float *)m_mtxImagePixel[y + 1u];

            float *pTargetLine = vecNewData.data() + y * m_uWidth;

            for(unsigned x = 1u; x < m_uWidth - 1u; x++)
            {
                const unsigned nLeftOffset  = x - 1u;
                const float *pLeftT = pLastLineData + nLeftOffset;
                const float *pLeft  = pCurLineData  + nLeftOffset;
                const float *pLeftB = pNextLineData + nLeftOffset;

                const unsigned nMidOffset  = x;
                const float *pMidT = pLastLineData + nMidOffset;
                const float *pMid  = pCurLineData  + nMidOffset;
                const float *pMidB = pNextLineData + nMidOffset;

                const unsigned nRightOffset  = x + 1u;
                const float *pRightT = pLastLineData + nRightOffset;
                const float *pRight  = pCurLineData  + nRightOffset;
                const float *pRightB = pNextLineData + nRightOffset;

                float *pTargetPos = pTargetLine + nMidOffset;
                if(m_bIgnoreInvalidValue)
                {
                    double dblGain = dblConGain;

                    double dblLeftT = 0.0;
                    if(*pLeftT > m_fNullLuminance)  dblLeftT  = *pLeftT * mtxConKernel[0][0];
                    else    dblGain -= mtxConKernel[0][0];

                    double dblLeft = 0.0;
                    if(*pLeft > m_fNullLuminance)  dblLeft  = *pLeft * mtxConKernel[1][0];
                    else    dblGain -= mtxConKernel[1][0];

                    double dblLeftB = 0.0;
                    if(*pLeftB > m_fNullLuminance)  dblLeftB  = *pLeftB * mtxConKernel[2][0];
                    else    dblGain -= mtxConKernel[2][0];

                    double dblMidT = 0.0;
                    if(*pMidT > m_fNullLuminance)  dblMidT  = *pMidT * mtxConKernel[0][1];
                    else    dblGain -= mtxConKernel[0][1];

                    double dblMid = 0.0;
                    if(*pMid > m_fNullLuminance)  dblMid  = *pMid * mtxConKernel[1][1];
                    else    dblGain -= mtxConKernel[1][1];

                    double dblMidB = 0.0;
                    if(*pMidB > m_fNullLuminance)  dblMidB  = *pMidB * mtxConKernel[2][1];
                    else    dblGain -= mtxConKernel[2][1];

                    double dblRightT = 0.0;
                    if(*pRightT > m_fNullLuminance)  dblRightT  = *pRightT * mtxConKernel[0][2];
                    else    dblGain -= mtxConKernel[0][2];

                    double dblRight = 0.0;
                    if(*pRight > m_fNullLuminance)  dblRight  = *pRight * mtxConKernel[1][2];
                    else    dblGain -= mtxConKernel[1][2];

                    double dblRightB = 0.0;
                    if(*pRightB > m_fNullLuminance)  dblRightB  = *pRightB * mtxConKernel[2][2];
                    else    dblGain -= mtxConKernel[2][2];

                    const double dblValue  = dblLeftT  + dblLeft  + dblLeftB
                                           + dblMidT   + dblMid   + dblMidB
                                           + dblRightT + dblRight + dblRightB;
                    *pTargetPos = (float)(dblValue) * dblConGain / dblGain;
                }
                else
                {
                    const double dblLeftT  = *pLeftT  * mtxConKernel[0][0];
                    const double dblLeft   = *pLeft   * mtxConKernel[1][0];
                    const double dblLeftB  = *pLeftB  * mtxConKernel[2][0];

                    const double dblMidT   = *pMidT   * mtxConKernel[0][1];
                    const double dblMid    = *pMid    * mtxConKernel[1][1];
                    const double dblMidB   = *pMidB   * mtxConKernel[2][1];

                    const double dblRightT = *pRightT * mtxConKernel[0][2];
                    const double dblRight  = *pRight  * mtxConKernel[1][2];
                    const double dblRightB = *pRightB * mtxConKernel[2][2];

                    const double dblValue  = dblLeftT  + dblLeft  + dblLeftB
                                           + dblMidT   + dblMid   + dblMidB
                                           + dblRightT + dblRight + dblRightB;
                    *pTargetPos = (float)(dblValue);
                }
            }
        }

        memcpy(m_pData, vecNewData.data(), nImageSize);
    }
    else return false;

    return true;
}


bool Image::saveToFile(const std::string &strFilePath) const
{
    if(!isValid())  return false;

    const unsigned nImageSize = getImageSizeInByte();

#ifdef _WIN32
    BITMAPFILEHEADER    fileHeader;
    memset(&fileHeader, 0, sizeof(BITMAPFILEHEADER));
    fileHeader.bfType    = 0x4D42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize    = fileHeader.bfOffBits + nImageSize;

    BITMAPINFOHEADER infoHeader;
    memset(&infoHeader, 0, sizeof(BITMAPINFOHEADER));
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = m_uWidth;
    infoHeader.biHeight = m_uHeight;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = getPixelSizeInByte() * 8;
    infoHeader.biCompression = BI_RGB;

    FILE *pFile = fopen(strFilePath.c_str(), "wb");
    if(!pFile)  return false;
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
    fwrite(m_pData, nImageSize, 1, pFile);
    fclose(pFile);
    return true;
#endif
    return false;
}

bool Image::swapRedAndBlueChanel()
{

    if(NULL == m_pData)   
    {
        return false;
    }

    unsigned ChanelCnt = 0;

    if(cmm::image::PF_RGBA == m_eFormat) 
    {
        ChanelCnt = 4;
    }
    else if(cmm::image::PF_RGB == m_eFormat) 
    {
        ChanelCnt = 3;
    }
    else return false;

    unsigned PixelCnt           = getImageSizeInByte() / getPixelSizeInByte();
    unsigned PixelChanelInBytes = getPixelSizeInByte() / ChanelCnt;
    unsigned OffsetFromB2R      = PixelChanelInBytes * 2;

        //将每个像素的0字节和第2个字节对换
    for (unsigned i = 0; i < PixelCnt; i++)
    {
        char    chRed = ((char *)m_pData)[i*getPixelSizeInByte()];
        char    chBlue = ((char *)m_pData)[i*getPixelSizeInByte()+2];
        
        ((char *)m_pData)[i*getPixelSizeInByte()] = chBlue;
        ((char *)m_pData)[i*getPixelSizeInByte()+2] = chRed;
    }

    return true;
}

bool Image::flipVertical()
{
    if (!isValid())
    {
        return false;
    }
    std::vector<char> LineBuf;
    LineBuf.resize(getLineSizeInByte());
    int iCount = m_uHeight / 2;
    //图片像素的总行数为n, 进行上下交换，0和n-1  1和n-2  2和n-3 依次类推进行交换
    for (int i = 0; i < iCount ; i++)
    {
        memcpy(LineBuf.data(), &((char*)m_pData)[i*getLineSizeInByte()], getLineSizeInByte());
        memcpy(&((char*)m_pData)[i*getLineSizeInByte()], &((char*)m_pData)[(m_uHeight-1-i)*getLineSizeInByte()], getLineSizeInByte());
        memcpy(&((char*)m_pData)[(m_uHeight-1-i)*getLineSizeInByte()], LineBuf.data(), getLineSizeInByte());
    }
    return true;
}

bool Image::fromBson(bson::bsonElement &val)
{
	if (val.GetType() != bson::bsonDocType)
	{
		return false;
	}

	bson::bsonDocumentEle *elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
	bson::bsonDocument &doc = elem->GetDoc();

	if (doc.GetElement(BSON_IMAGE_WIDTH) == NULL   || doc.GetElement(BSON_IMAGE_WIDTH)->GetType() != bson::bsonDoubleType ||
		doc.GetElement(BSON_IMAGE_HEIGHT) == NULL  || doc.GetElement(BSON_IMAGE_HEIGHT)->GetType() != bson::bsonDoubleType ||
		doc.GetElement(BSON_IMAGE_DATALEN) == NULL || doc.GetElement(BSON_IMAGE_DATALEN)->GetType() != bson::bsonDoubleType ||
		doc.GetElement(BSON_IMAGE_DATA) == NULL    || doc.GetElement(BSON_IMAGE_DATA)->GetType() != bson::bsonStringType)
	{
		return false;
	}

	bson::bsonDoubleEle *w    = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement(BSON_IMAGE_WIDTH));
	bson::bsonDoubleEle *h    = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement(BSON_IMAGE_HEIGHT));
	bson::bsonDoubleEle *l  = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement(BSON_IMAGE_DATALEN));
	bson::bsonStringEle *data = dynamic_cast<bson::bsonStringEle*>(doc.GetElement(BSON_IMAGE_DATA));

	const char* src_data = data->StrValue();
	unsigned src_len = (unsigned)l->DblValue();
	unsigned len = src_len / 2;

	if (src_len < 5 || src_len % 2 != 0 || src_data[src_len] != 0)
	{
		//编码后的图片大小必须大于等于2*2,并且最后一个字符是0结尾
		return false;
	}
    
	char *img_data = new char[len];
	char *tmp = img_data;

	for (unsigned i = 0; i < len ; i++)
	{
		PixelData *pTemp = (PixelData *)tmp;

		pTemp->LowBit = (0x0f & *src_data++);
		pTemp->HighBit = (0x0f & *src_data++);

		tmp++;
	}

    attach(img_data, (unsigned long)w->DblValue(), (unsigned long)h->DblValue(), PF_RGBA);
	return true;
}

void Image::toBson(bson::bsonElement &val)
{
	bson::bsonDoubleEle   *elem_w     = NULL;
	bson::bsonDoubleEle   *elem_h     = NULL;
	bson::bsonDoubleEle   *elem_len   = NULL;
	bson::bsonStringEle   *elem_data  = NULL;
	bson::bsonDocumentEle *doc_elem   = dynamic_cast<bson::bsonDocumentEle*>(&val);

	for (unsigned int i = 0; i < doc_elem->GetDoc().ChildCount(); i++)
	{
		bson::bsonElement *e = doc_elem->GetDoc().GetElement(i);

		if (strcmp(e->EName(), BSON_IMAGE_WIDTH) == 0 && e->GetType() == bson::bsonDoubleType)
		{
			elem_w = dynamic_cast<bson::bsonDoubleEle*>(e);
			elem_w->SetDblValue(getWidth());
		}

		if (strcmp(e->EName(), BSON_IMAGE_HEIGHT) == 0 && e->GetType() == bson::bsonDoubleType)
		{
			elem_h = dynamic_cast<bson::bsonDoubleEle*>(e);
			elem_h->SetDblValue(getHeight());
		}

		if (strcmp(e->EName(), BSON_IMAGE_DATALEN) == 0 && e->GetType() == bson::bsonDoubleType)
		{
			elem_h = dynamic_cast<bson::bsonDoubleEle*>(e);
		}

		if (strcmp(e->EName(), BSON_IMAGE_DATA) == 0 && e->GetType() == bson::bsonStringType)
		{
			elem_data = dynamic_cast<bson::bsonStringEle*>(e);
		}
	}

	const char *src = (const char*)data();
    const unsigned src_len = getImageSizeInByte();

	unsigned int len = getImageSizeInByte() * 2;
	std::string dst;
    dst.resize(len);
    char *tmp = (char *)dst.data();

	for (unsigned i = 0; i <  src_len; i++)
	{
		PixelData *pTemp = (PixelData *)src;

		*tmp++ = pTemp->LowBit | 0x10;
		*tmp++ = pTemp->HighBit | 0x10;

		src++;
	}

	*tmp = 0;
    
	if (!elem_w)
	{
		doc_elem->GetDoc().AddDblElement(BSON_IMAGE_WIDTH, getWidth());
	}

	if (!elem_h)
	{
		doc_elem->GetDoc().AddDblElement(BSON_IMAGE_HEIGHT, getHeight());
	}

	if (elem_len)
	{
		elem_len->SetDblValue(len);
	}
	else
	{
		doc_elem->GetDoc().AddDblElement(BSON_IMAGE_DATALEN, len);
	}

	if (elem_data)
	{
        elem_data->SetStrValue(dst.data());
	}
	else
	{
		doc_elem->GetDoc().AddStringElement(BSON_IMAGE_DATA, dst.data());
	}

}

#if 0
Image *scaleImageByArea(const Image &imageSrc, const cmm::math::Box2d &bbFrom, const cmm::math::Box2d &bbTo)
{
    // 检测两张图片的覆盖区域，若没有重叠区域，则无法进行缩放
    const cmm::math::Point2d ptLBFrom = bbFrom.corner(cmm::math::Box2d::LeftBottom);
    const cmm::math::Point2d ptRTFrom = bbFrom.corner(cmm::math::Box2d::RightTop);
    const cmm::math::Point2d ptLBTo   = bbTo.corner(cmm::math::Box2d::LeftBottom);
    const cmm::math::Point2d ptRTTo   = bbTo.corner(cmm::math::Box2d::RightTop);

    if(ptLBFrom.x() >= ptRTTo.x())  return NULL;
    if(ptLBFrom.y() >= ptRTTo.y())  return NULL;
    if(ptRTFrom.x() <= ptLBTo.x())  return NULL;
    if(ptRTFrom.y() <= ptLBTo.y())  return NULL;

    const double dblLeft   = std::max(ptLBFrom.x(), ptLBTo.x()) - ptLBFrom.x();
    const double dblRight  = std::min(ptRTFrom.x(), ptRTTo.x()) - ptLBFrom.x();
    const double dblBottom = std::max(ptLBFrom.y(), ptLBTo.y()) - ptLBFrom.y();
    const double dblTop    = std::min(ptRTFrom.y(), ptRTTo.y()) - ptLBFrom.y();

    const double dblWidthFrom  = bbFrom.width();
    const double dblHeightFrom = bbFrom.height();
    const unsigned nLeft   = unsigned((dblLeft   / dblWidthFrom)  * imageSrc.getWidth()  + 0.5);
    const unsigned nRight  = unsigned((dblRight  / dblWidthFrom)  * imageSrc.getWidth()  + 0.5);
    const unsigned nBottom = unsigned((dblBottom / dblHeightFrom) * imageSrc.getHeight() + 0.5);
    const unsigned nTop    = unsigned((dblTop    / dblHeightFrom) * imageSrc.getHeight() + 0.5);

    const unsigned nWidth = cmm::math::clampAbove(nRight - nLeft, 1u);
    const unsigned nHeight = cmm::math::clampAbove(nTop - nBottom, 1u);
    OpenSP::sp<cmm::image::Image>  pSubImage = imageSrc.getSubImage(nLeft, nBottom, nWidth, nHeight);
    if(!pSubImage.valid())
    {
        return NULL;
    }

    pSubImage->scaleImage(imageSrc.getWidth(), imageSrc.getHeight());

    return pSubImage.release();
}


bool blendImage(Image &DstImage,   const Image &SrcImage,  
                int xDstOffset,     int yDstOffset,
                int xSrcOffset,     int ySrcOffset,
                unsigned uWidth,    unsigned uHeight)
{
    //检测图像操作合法性
    if (!SrcImage.IsValid() || !DstImage.IsValid()) return false;

    if (uWidth  == -1) uWidth  = SrcImage.m_uWidth;
    if (uHeight == -1) uHeight = SrcImage.m_uHeight;

    if (xSrcOffset + uWidth  > SrcImage.m_uWidth ||
        ySrcOffset + uHeight > SrcImage.m_uHeight||
        xDstOffset + uWidth  > DstImage.m_uWidth ||
        yDstOffset + uHeight > DstImage.m_uHeight) 
    {
        return false;
    }

    unsigned int uPixelSize = 0;
              
    for (unsigned i = 0; i < uHeight; i++)
    {
        const char *pSrc = (const char *)SrcImage.OffsetPtr(xSrcOffset, ySrcOffset + i);
              char *pDst = (char *)DstImage.OffsetPtr(xDstOffset, yDstOffset + i);

        if (!pSrc || !pDst) return false;

        for (unsigned j = 0; j < uWidth; j++)
        {
            if (SrcImage.m_Format != PF_LUMINANCE) 
            {
                unsigned char dstA = (DstImage.m_Format == PF_RGB ? 255u : pDst[DstImage.OffsetA()]);
                unsigned char srcA = (SrcImage.m_Format == PF_RGB ? 255u : pSrc[SrcImage.OffsetA()]);

                unsigned char dstR = pDst[DstImage.OffsetR()] * dstA / 255;
                unsigned char dstG = pDst[DstImage.OffsetG()] * dstA / 255;
                unsigned char dstB = pDst[DstImage.OffsetB()] * dstA / 255;

                pDst[DstImage.OffsetR()] = (dstR * (255 - srcA) + pSrc[SrcImage.OffsetR()] * srcA) / 255;
                pDst[DstImage.OffsetG()] = (dstG * (255 - srcA) + pSrc[SrcImage.OffsetG()] * srcA) / 255;
                pDst[DstImage.OffsetB()] = (dstB * (255 - srcA) + pSrc[SrcImage.OffsetB()] * srcA) / 255;

                if (DstImage.m_Format != PF_RGB)
                {
                    //DstA = 1 - (1 - srcA / 255) (1 - dstA / 255);
                    //     = 255 - (255 - srcA)(1 - dstA / 255);
                    //     = 255 - (255 - dstA - srcA + srcA * dstA / 255);
                    pDst[DstImage.OffsetA()] = srcA + dstA - srcA * dstA / 255;
                }
            }
            else
            {
                if (math::floatEqual(pSrc[DstImage.OffsetA()], SrcImage.m_fNullLuminance) == false)
                {
                    pDst[DstImage.OffsetA()] = pSrc[SrcImage.OffsetA()];
                }
            }

            pSrc += SrcImage.GetPixelSizeInByte();
            pDst += DstImage.GetPixelSizeInByte();
        }
    }
    return true;
}
#endif


}}