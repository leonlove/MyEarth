#include "ReadWriteTiFF.h"
#include "tiffio.h"

#define ERR_NO_ERROR    0
#define ERR_OPEN        1
#define ERR_READ        2
#define ERR_MEM         3
#define ERR_UNSUPPORTED 4
#define ERR_TIFFLIB     5

///////////////////////////////////////////////////////////////////////////////////
/*********************************************************************************/
/*********************************************************************************/
///////////////////////////////////////////////////////////////////////////////////
tsize_t libtiffStreamReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::istream *fin = (std::istream*)fd;
    if (fin == NULL)
    {
        return -1;
    }

    fin->read((char*)buf,size);

    if(fin->bad())
        return -1;

    if(fin->gcount() < size)
        return 0;

    return size;
}


tsize_t libtiffStreamWriteProc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}


toff_t libtiffStreamSeekProc(thandle_t fd, toff_t off, int i)
{
    std::istream *fin = (std::istream*)fd;
    if (fin == NULL)
    {
        return -1;
    }

    toff_t ret;
    switch(i)
    {
    case SEEK_SET:
        fin->seekg(off,std::ios::beg);
        ret = (toff_t)fin->tellg();
        if(fin->bad())
            ret = 0;
        break;

    case SEEK_CUR:
        fin->seekg(off,std::ios::cur);
        ret = (toff_t)fin->tellg();
        if(fin->bad())
            ret = 0;
        break;

    case SEEK_END:
        fin->seekg(off,std::ios::end);
        ret = (toff_t)fin->tellg();
        if(fin->bad())
            ret = 0;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}


toff_t libtiffStreamSizeProc(thandle_t fd)
{
    std::istream *fin = (std::istream*)fd;
    if (fin == NULL)
    {
        return -1;
    }

    std::streampos curPos = fin->tellg();

    fin->seekg(0, std::ios::end);
    toff_t size = (toff_t)fin->tellg();
    fin->seekg(curPos, std::ios::beg);

    return size;
}


tsize_t libtiffOStreamReadProc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}


tsize_t libtiffOStreamWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::ostream *fout = (std::ostream*)fd;
    if (fout == NULL)
    {
        return -1;
    }

    fout->write((const char*)buf,size);

    if(fout->bad())
    {
        return -1;
    }

    return size;
}


toff_t libtiffOStreamSeekProc(thandle_t fd, toff_t off, int i)
{
    std::ostream *fout = (std::ostream*)fd;
    if (fout == NULL)
    {
        return -1;
    }

    toff_t pos_required = 0;
    toff_t stream_end = 0;
    switch(i)
    {
    case SEEK_SET:
        {
            if (off==0)
            {
                std::streampos checkEmpty = fout->tellp();
                if(checkEmpty < 0)
                {
                    return 0;
                }
            }
            pos_required = off;

            fout->seekp(0, std::ios::end);
            stream_end = (toff_t)fout->tellp();
            break;
        }
    case SEEK_CUR:
        {
            toff_t stream_curr = (toff_t)fout->tellp();
            pos_required = stream_curr + off;

            fout->seekp(0, std::ios::end);
            stream_end = (toff_t)fout->tellp();
            break;
        }
    case SEEK_END:
        {
            fout->seekp(0, std::ios::end);
            stream_end = (toff_t)fout->tellp();
            pos_required = stream_end + off;
            break;
        }
    default:
        break;
    }

    if (pos_required>stream_end)
    {
        fout->seekp(0, std::ios::end);
        for(toff_t i=stream_end; i<pos_required; ++i)
        {
            fout->put(char(0));
        }
    }

    fout->seekp(pos_required,std::ios::beg);
    toff_t ret = (toff_t)fout->tellp();
    if (fout->bad())
    {
        ret = 0;
    }
    return ret;
}


int libtiffStreamCloseProc(thandle_t)
{
    return 0;
}


toff_t libtiffOStreamSizeProc(thandle_t fd)
{
    std::ostream *fout = (std::ostream*)fd;
    if (fout == NULL)
    {
        return -1;
    }

    std::streampos curPos = fout->tellp();

    fout->seekp(0, std::ios::end);
    toff_t size = (toff_t)fout->tellp();
    fout->seekp(curPos, std::ios::beg);

    return size;
}


int libtiffStreamMapProc(thandle_t, tdata_t*, toff_t*)
{
    return 0;
}


void libtiffStreamUnmapProc(thandle_t, tdata_t, toff_t)
{
}


static void invert_row(unsigned char *ptr, unsigned char *data, int n, int invert, uint16 bitspersample)
{
    if (ptr == NULL || data == NULL)
    {
        return;
    }

    if (bitspersample == 8)
    {
        while (n--)
        {
            if (invert) *ptr++ = 255 - *data++;
            else *ptr++ = *data++;
        }
    }
    else if (bitspersample == 16)
    {
        unsigned short *ptr1 = (unsigned short *)ptr;
        unsigned short *data1 = (unsigned short *)data;

        while (n--)
        {
            if (invert) *ptr1++ = 65535 - *data1++;
            else *ptr1++ = *data1++;
        }
    }
    else if (bitspersample == 32)
    {
        float *ptr1 = (float *)ptr;
        float *data1 = (float *)data;

        while (n--)
        {
            if (invert) *ptr1++ = 1.0f - *data1++;
            else *ptr1++ = *data1++;
        }
    }
}


static int checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
    if (r == NULL || g == NULL)
    {
        return -1;
    }

    while (n-- > 0)
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
            return (16);

    return (8);
}


static void
remap_row(unsigned char *ptr, unsigned char *data, int n,
          unsigned short *rmap, unsigned short *gmap, unsigned short *bmap)
{
    if (ptr == NULL || data == NULL || rmap == NULL || gmap == NULL || bmap == NULL)
    {
        return;
    }

    unsigned int ix;
    while (n--)
    {
        ix = *data++;
        *ptr++ = (unsigned char) rmap[ix];
        *ptr++ = (unsigned char) gmap[ix];
        *ptr++ = (unsigned char) bmap[ix];
    }
}


static void interleave_row(unsigned char *ptr,
                           unsigned char *red, unsigned char *green, unsigned char *blue,
                           int n, int numSamples, uint16 bitspersample)
{
    if (ptr == NULL || red == NULL || green == NULL || blue == NULL)
    {
        return;
    }

    // OSG_NOTICE<<"Interleave row RGB"<<std::endl;
    if (bitspersample == 8)
    {
        while (n--)
        {
            *ptr++ = *red++;
            *ptr++ = *green++;
            *ptr++ = *blue++;
            if (numSamples==4) *ptr++ = 255;
        }
    }
    else if (bitspersample == 16)
    {
        unsigned short *ptr1 = (unsigned short *)ptr;
        unsigned short *red1 = (unsigned short *)red;
        unsigned short *green1 = (unsigned short *)green;
        unsigned short *blue1 = (unsigned short *)blue;

        while (n--)
        {
            *ptr1++ = *red1++;
            *ptr1++ = *green1++;
            *ptr1++ = *blue1++;
            if (numSamples==4) *ptr1++ = 65535;
        }
    }
    else if (bitspersample == 32)
    {
        float *ptr1 = (float *)ptr;
        float *red1 = (float *)red;
        float *green1 = (float *)green;
        float *blue1 = (float *)blue;

        while (n--)
        {
            *ptr1++ = *red1++;
            *ptr1++ = *green1++;
            *ptr1++ = *blue1++;
            if (numSamples==4) *ptr1++ = 1.0f;
        }
    }
}

static void interleave_row(unsigned char *ptr,
                           unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha,
                           int n, int numSamples, uint16 bitspersample)
{
    if (ptr == NULL || red == NULL || green == NULL || blue == NULL || alpha == NULL)
    {
        return;
    }

    // OSG_NOTICE<<"Interleave row RGBA"<<std::endl;
    if (bitspersample == 8)
    {
        while (n--)
        {
            *ptr++ = *red++;
            *ptr++ = *green++;
            *ptr++ = *blue++;
            if (numSamples==4) *ptr++ = *alpha++;
        }
    }
    else if (bitspersample == 16)
    {
        unsigned short *ptr1 = (unsigned short *)ptr;
        unsigned short *red1 = (unsigned short *)red;
        unsigned short *green1 = (unsigned short *)green;
        unsigned short *blue1 = (unsigned short *)blue;
        unsigned short *alpha1 = (unsigned short *)alpha;

        while (n--)
        {
            *ptr1++ = *red1++;
            *ptr1++ = *green1++;
            *ptr1++ = *blue1++;
            if (numSamples==4) *ptr1++ = *alpha1++;
        }
    }
    else if (bitspersample == 32)
    {
        float *ptr1 = (float *)ptr;
        float *red1 = (float *)red;
        float *green1 = (float *)green;
        float *blue1 = (float *)blue;
        float *alpha1 = (float *)alpha;

        while (n--)
        {
            *ptr1++ = *red1++;
            *ptr1++ = *green1++;
            *ptr1++ = *blue1++;
            if (numSamples==4) *ptr1++ = *alpha1++;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////
/*********************************************************************************/
/*********************************************************************************/
///////////////////////////////////////////////////////////////////////////////////
ReadWriteTiFF::ReadWriteTiFF(void)
{
}


ReadWriteTiFF::~ReadWriteTiFF(void)
{
}


#define CVT(x)      (((x) * 255L) / ((1L<<16)-1))
#define pack(a,b)   ((a)<<8 | (b))


static int tifferror = ERR_NO_ERROR;

unsigned char* ReadWriteTiFF::readTIFStream(std::istream& fin, PixelType &nPixelType) const
{
    TIFF*   pIn;
    uint16  dataType;
    uint16  samplesperpixel;
    uint16  photometric;
    uint16  bitspersample;
    uint32  w, h;
    uint16  config;
    uint16* red;
    uint16* green;
    uint16* blue;
    tsize_t rowsize;
    uint32  row;
    int     format;
    int     width;
    int     height;
    unsigned char *inbuf   = NULL;
    unsigned char *buffer  = NULL;
    unsigned char *currPtr = NULL;

    pIn = TIFFClientOpen("inputstream", "r", (thandle_t)&fin,
                         libtiffStreamReadProc,   //Custom read function
                         libtiffStreamWriteProc,  //Custom write function
                         libtiffStreamSeekProc,   //Custom seek function
                         libtiffStreamCloseProc,  //Custom close function
                         libtiffStreamSizeProc,   //Custom size function
                         libtiffStreamMapProc,    //Custom map function
                         libtiffStreamUnmapProc); //Custom unmap function

    if (pIn == NULL)
    {
        tifferror = ERR_OPEN;
        return NULL;
    }
    if (TIFFGetField(pIn, TIFFTAG_PHOTOMETRIC, &photometric) == 1)
    {
        if (photometric != PHOTOMETRIC_RGB && photometric != PHOTOMETRIC_PALETTE &&
            photometric != PHOTOMETRIC_MINISWHITE &&
            photometric != PHOTOMETRIC_MINISBLACK)
        {
            tifferror = ERR_UNSUPPORTED;
            TIFFClose(pIn);
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(pIn);
        return NULL;
    }

    if (TIFFGetField(pIn, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel) == 1)
    {
        if (samplesperpixel != 1 &&
            samplesperpixel != 2 &&
            samplesperpixel != 3 &&
            samplesperpixel != 4)
        {
            tifferror = ERR_UNSUPPORTED;
            TIFFClose(pIn);
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(pIn);
        return NULL;
    }

    if (TIFFGetField(pIn, TIFFTAG_BITSPERSAMPLE, &bitspersample) == 1)
    {
         if (bitspersample != 8 && bitspersample != 16 && bitspersample != 32)
        {
            tifferror = ERR_UNSUPPORTED;
            TIFFClose(pIn);
            return NULL;
        }
    }
    else
    {
        tifferror = ERR_READ;
        TIFFClose(pIn);
        return NULL;
    }
    
    if (TIFFGetField(pIn, TIFFTAG_IMAGEWIDTH, &w) != 1 ||
        TIFFGetField(pIn, TIFFTAG_IMAGELENGTH, &h) != 1 ||
        TIFFGetField(pIn, TIFFTAG_PLANARCONFIG, &config) != 1)
    {
        tifferror = ERR_READ;
        TIFFClose(pIn);
        return NULL;
    }

        
    TIFFGetField(pIn, TIFFTAG_DATATYPE, &dataType);

    if (photometric == PHOTOMETRIC_PALETTE)
        format = 3; 
    else
        format = samplesperpixel * bitspersample / 8;
    
    
    int bytespersample = bitspersample / 8;
    int bytesperpixel = bytespersample * samplesperpixel;

    buffer = new unsigned char [w*h*format];

    if (!buffer)
    {
        tifferror = ERR_MEM;
        TIFFClose(pIn);
        return NULL;
    }

    // initialize memory
    for(unsigned char* ptr=buffer;ptr<buffer+w*h*format;++ptr) *ptr = 0;

    width = w;
    height = h;

    currPtr = buffer + (h-1)*w*format;

    switch (pack(photometric, config))
    {
        case pack(PHOTOMETRIC_MINISWHITE, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_MINISBLACK, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_MINISWHITE, PLANARCONFIG_SEPARATE):
        case pack(PHOTOMETRIC_MINISBLACK, PLANARCONFIG_SEPARATE):
            inbuf = new unsigned char [TIFFScanlineSize(pIn)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(pIn, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                invert_row(currPtr, inbuf, w, photometric == PHOTOMETRIC_MINISWHITE, bitspersample);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_PALETTE, PLANARCONFIG_CONTIG):
        case pack(PHOTOMETRIC_PALETTE, PLANARCONFIG_SEPARATE):
            if (TIFFGetField(pIn, TIFFTAG_COLORMAP, &red, &green, &blue) != 1)
                tifferror = ERR_READ;
            if (!tifferror && checkcmap(1<<bitspersample, red, green, blue) == 16)
            {
                int i;
                for (i = (1<<bitspersample)-1; i >= 0; i--)
                {
                    red[i] = CVT(red[i]);
                    green[i] = CVT(green[i]);
                    blue[i] = CVT(blue[i]);
                }
            }

            inbuf = new unsigned char [TIFFScanlineSize(pIn)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(pIn, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                remap_row(currPtr, inbuf, w, red, green, blue);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_RGB, PLANARCONFIG_CONTIG):
            inbuf = new unsigned char [TIFFScanlineSize(pIn)];
            for (row = 0; row < h; row++)
            {
                if (TIFFReadScanline(pIn, inbuf, row, 0) < 0)
                {
                    tifferror = ERR_READ;
                    break;
                }
                memcpy(currPtr, inbuf, format*w);
                currPtr -= format*w;
            }
            break;

        case pack(PHOTOMETRIC_RGB, PLANARCONFIG_SEPARATE):
            rowsize = TIFFScanlineSize(pIn);
            inbuf = new unsigned char [format*rowsize];
            for (row = 0; !tifferror && row < h; row++)
            {
                int s;
                for (s = 0; s < format; s++)
                {
                    if (TIFFReadScanline(pIn, (tdata_t)(inbuf+s*rowsize), (uint32)row, (tsample_t)s) < 0)
                    {
                        tifferror = ERR_READ;
                        break;
                    }
                }
                if (!tifferror)
                {
                    if (format==3) interleave_row(currPtr, inbuf, inbuf+rowsize, inbuf+2*rowsize, w, format, bitspersample);
                    else if (format==4) interleave_row(currPtr, inbuf, inbuf+rowsize, inbuf+2*rowsize, inbuf+3*rowsize, w, format, bitspersample);
                    currPtr -= format*w;
                }
            }
            break;
        default:
            tifferror = ERR_UNSUPPORTED;
            break;
    }

    if (inbuf) delete [] inbuf;
    TIFFClose(pIn);

    if (tifferror)
    {
        if (buffer) delete [] buffer;
        return NULL;
    }

    int numComponents_ret = 0;
    if (photometric == PHOTOMETRIC_PALETTE)
        numComponents_ret = format;
    else
        numComponents_ret = samplesperpixel;

    switch (numComponents_ret)
    {
    case 1:
        nPixelType = PixelType_LUMINANCE;
        break;
    case 2:
        nPixelType = PixelType_LUMINANCE_ALPHA;
        break;
    case 3:
        nPixelType = PixelType_RGB;
        break;
    case 4:
        nPixelType = PixelType_RGBA;
        break;
    }
    

    return buffer;
}


bool ReadWriteTiFF::writeTIFStream(std::ostream& fout, const char* pBmpBuf, const PixelType nPixelType, const GDTType nType, const int nXSize, const int nYSize, const bool bImage) const
{
    if (pBmpBuf == NULL)
    {
        return false;
    }

    int    samplesPerPixel = 0;
    int    bitsPerSample   = 0;
    TIFF*  pImage          = NULL;
    uint16 photometric     = 0;

    pImage = TIFFClientOpen("outputstream", "w", (thandle_t)&fout,
                            libtiffOStreamReadProc,  //Custom read function
                            libtiffOStreamWriteProc, //Custom write function
                            libtiffOStreamSeekProc,  //Custom seek function
                            libtiffStreamCloseProc,  //Custom close function
                            libtiffOStreamSizeProc,  //Custom size function
                            libtiffStreamMapProc,    //Custom map function
                            libtiffStreamUnmapProc); //Custom unmap function

    if(pImage == NULL)
    {
        return false;
    }

    switch(nPixelType)
    {
    case PixelType_DEPTH_COMPONENT:
    case PixelType_LUMINANCE:
    case PixelType_ALPHA:
        photometric = PHOTOMETRIC_MINISBLACK;
        samplesPerPixel = 1;
        break;
    case PixelType_LUMINANCE_ALPHA:
        photometric = PHOTOMETRIC_MINISBLACK;
        samplesPerPixel = 2;
        break;
    case PixelType_RGB:
        photometric = PHOTOMETRIC_RGB;
        samplesPerPixel = 3;
        break;
    case PixelType_RGBA:
        photometric = PHOTOMETRIC_RGB;
        samplesPerPixel = 4;
        break;
    default:
        return false;
        break;
    }

    switch(nType){
    case GDTType_Float32:
        TIFFSetField(pImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(pImage, TIFFTAG_ROWSPERSTRIP, 1);
        bitsPerSample = 32;
        break;
    case GDTType_UInt32:
        TIFFSetField(pImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
        bitsPerSample = 16;
        break;
    default:
        bitsPerSample = 8;
        break;
    }

    TIFFSetField(pImage, TIFFTAG_IMAGEWIDTH, nXSize);
    TIFFSetField(pImage, TIFFTAG_IMAGELENGTH, nYSize);
    TIFFSetField(pImage, TIFFTAG_BITSPERSAMPLE,bitsPerSample);
    TIFFSetField(pImage, TIFFTAG_SAMPLESPERPIXEL,samplesPerPixel);
    TIFFSetField(pImage, TIFFTAG_PHOTOMETRIC, photometric);
    if (bImage)
    {
        TIFFSetField(pImage, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
    }
    else
    {
        TIFFSetField(pImage, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    }
    TIFFSetField(pImage, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(pImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Write the information to the file
    for(int i = 0; i < nYSize; ++i) 
    {
        const char* pScanLine = pBmpBuf + (nXSize * samplesPerPixel * bitsPerSample / 8) * (nYSize - i - 1);
        TIFFWriteScanline(pImage,(tdata_t)pScanLine, i, 0);
    }

    // Close the file
    TIFFClose(pImage);

    return true;
}
