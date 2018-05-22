/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/GLExtensions>
#include <osg/Image>
#include <osg/Texture>
#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>
#include <osg/Timer>
#include <osg/FrameBufferObject>
#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/GLObjects>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R                 0x8072
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL              0x813D
#endif


#ifndef GL_UNPACK_CLIENT_STORAGE_APPLE
#define GL_UNPACK_CLIENT_STORAGE_APPLE    0x85B2
#endif

#ifndef GL_APPLE_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_APPLE       0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE 0x851E
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE 0x851F
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE 0x8521
#define GL_STORAGE_CACHED_APPLE           0x85BE
#define GL_STORAGE_SHARED_APPLE           0x85BF
#endif

#if 0
    #define CHECK_CONSISTENCY checkConsistency();
#else
    #define CHECK_CONSISTENCY
#endif

namespace osg {

typedef buffered_value< ref_ptr<Texture::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

void Texture::TextureObject::generate(GLenum eTarget)
{
    m_nTexture = GLObjectPool::instance()->genTexture();
    m_eTarget  = eTarget;
}


void Texture::TextureObject::destruct(unsigned nContextID)
{
    if(isValid())
    {
        GLObjectPool::instance()->deleteTexture(nContextID, m_nTexture);
    }
    m_eTarget = 0;
    m_nTexture = 0u;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Texture class implementation
//
Texture::Texture():
            _wrap_s(CLAMP),
            _wrap_t(CLAMP),
            _wrap_r(CLAMP),
            _min_filter(LINEAR_MIPMAP_LINEAR), // trilinear
            _mag_filter(LINEAR),
            _maxAnisotropy(1.0f),
            _useHardwareMipMapGeneration(true),
            _unrefImageDataAfterApply(false),
            _clientStorageHint(false),
            _resizeNonPowerOfTwoHint(true),
            _borderColor(0.0, 0.0, 0.0, 0.0),
            _borderWidth(0),
            _internalFormatMode(USE_IMAGE_DATA_FORMAT),
            _internalFormatType(NORMALIZED),
            _internalFormat(0),
            _sourceFormat(0),
            _sourceType(0),
            _use_shadow_comparison(false),
            _shadow_compare_func(LEQUAL),
            _shadow_texture_mode(LUMINANCE),
            _shadow_ambient(0)
{
}

Texture::Texture(const Texture& text,const CopyOp& copyop):
            StateAttribute(text,copyop),
            _wrap_s(text._wrap_s),
            _wrap_t(text._wrap_t),
            _wrap_r(text._wrap_r),
            _min_filter(text._min_filter),
            _mag_filter(text._mag_filter),
            _maxAnisotropy(text._maxAnisotropy),
            _useHardwareMipMapGeneration(text._useHardwareMipMapGeneration),
            _unrefImageDataAfterApply(text._unrefImageDataAfterApply),
            _clientStorageHint(text._clientStorageHint),
            _resizeNonPowerOfTwoHint(text._resizeNonPowerOfTwoHint),
            _borderColor(text._borderColor),
            _borderWidth(text._borderWidth),
            _internalFormatMode(text._internalFormatMode),
            _internalFormatType(text._internalFormatType),
            _internalFormat(text._internalFormat),
            _sourceFormat(text._sourceFormat),
            _sourceType(text._sourceType),
            _use_shadow_comparison(text._use_shadow_comparison),
            _shadow_compare_func(text._shadow_compare_func),
            _shadow_texture_mode(text._shadow_texture_mode),
            _shadow_ambient(text._shadow_ambient)
{
}

Texture::~Texture()
{
    // delete old texture objects.
    dirtyTextureObject();
}

int Texture::compareTexture(const Texture& rhs) const
{
    COMPARE_StateAttribute_Parameter(_wrap_s)
    COMPARE_StateAttribute_Parameter(_wrap_t)
    COMPARE_StateAttribute_Parameter(_wrap_r)
    COMPARE_StateAttribute_Parameter(_min_filter)
    COMPARE_StateAttribute_Parameter(_mag_filter)
    COMPARE_StateAttribute_Parameter(_maxAnisotropy)
    COMPARE_StateAttribute_Parameter(_useHardwareMipMapGeneration)
    COMPARE_StateAttribute_Parameter(_internalFormatMode)

    // only compare _internalFomat is it has alrady been set in both lhs, and rhs
    if (_internalFormat!=0 && rhs._internalFormat!=0)
    {
        COMPARE_StateAttribute_Parameter(_internalFormat)
    }

    COMPARE_StateAttribute_Parameter(_sourceFormat)
    COMPARE_StateAttribute_Parameter(_sourceType)

    COMPARE_StateAttribute_Parameter(_use_shadow_comparison)
    COMPARE_StateAttribute_Parameter(_shadow_compare_func)
    COMPARE_StateAttribute_Parameter(_shadow_texture_mode)
    COMPARE_StateAttribute_Parameter(_shadow_ambient)

    COMPARE_StateAttribute_Parameter(_unrefImageDataAfterApply)
    COMPARE_StateAttribute_Parameter(_clientStorageHint)
    COMPARE_StateAttribute_Parameter(_resizeNonPowerOfTwoHint)

    COMPARE_StateAttribute_Parameter(_internalFormatType);
    
    return 0;
}

int Texture::compareTextureObjects(const Texture& rhs) const
{
    if (_glObjectList.size() < rhs._glObjectList.size()) return -1; 
    if (rhs._glObjectList.size() < _glObjectList.size()) return 1;
    for(unsigned int i = 0u; i < _glObjectList.size(); ++i)
    {
        if (_glObjectList[i].m_eTarget < rhs._glObjectList[i].m_eTarget) return -1;
        else if (rhs._glObjectList[i].m_eTarget < _glObjectList[i].m_eTarget) return 1;

        if (_glObjectList[i].m_nTexture < rhs._glObjectList[i].m_nTexture) return -1;
        else if (rhs._glObjectList[i].m_nTexture < _glObjectList[i].m_nTexture) return 1;
    }
    return 0;
}

void Texture::setWrap(WrapParameter which, WrapMode wrap)
{
    switch( which )
    {
        case WRAP_S : _wrap_s = wrap; dirtyTextureParameters(); break;
        case WRAP_T : _wrap_t = wrap; dirtyTextureParameters(); break;
        case WRAP_R : _wrap_r = wrap; dirtyTextureParameters(); break;
        default : OSG_WARN<<"Error: invalid 'which' passed Texture::setWrap("<<(unsigned int)which<<","<<(unsigned int)wrap<<")"<<std::endl; break;
    }
    
}


Texture::WrapMode Texture::getWrap(WrapParameter which) const
{
    switch( which )
    {
        case WRAP_S : return _wrap_s;
        case WRAP_T : return _wrap_t;
        case WRAP_R : return _wrap_r;
        default : OSG_WARN<<"Error: invalid 'which' passed Texture::getWrap(which)"<<std::endl; return _wrap_s;
    }
}


void Texture::setFilter(FilterParameter which, FilterMode filter)
{
    switch( which )
    {
        case MIN_FILTER : _min_filter = filter; dirtyTextureParameters(); break;
        case MAG_FILTER : _mag_filter = filter; dirtyTextureParameters(); break;
        default : OSG_WARN<<"Error: invalid 'which' passed Texture::setFilter("<<(unsigned int)which<<","<<(unsigned int)filter<<")"<<std::endl; break;
    }
}


Texture::FilterMode Texture::getFilter(FilterParameter which) const
{
    switch( which )
    {
        case MIN_FILTER : return _min_filter;
        case MAG_FILTER : return _mag_filter;
        default : OSG_WARN<<"Error: invalid 'which' passed Texture::getFilter(which)"<< std::endl; return _min_filter;
    }
}

void Texture::setMaxAnisotropy(float anis)
{
    if (_maxAnisotropy!=anis)
    {
        _maxAnisotropy = anis; 
        dirtyTextureParameters();
    }
}


/** Force a recompile on next apply() of associated OpenGL texture objects.*/
void Texture::dirtyTextureObject()
{
    for(unsigned int i = 0; i < _glObjectList.size();++i)
    {
        TextureObject &textureObject = _glObjectList[i];
        textureObject.destruct(i);
    }
}

void Texture::dirtyTextureParameters()
{
    _texParametersDirtyList.setAllElementsTo(1);
}

void Texture::allocateMipmapLevels()
{
    _texMipmapGenerationDirtyList.setAllElementsTo(1);
}

void Texture::computeInternalFormatWithImage(const osg::Image& image) const
{
    GLint internalFormat = image.getInternalTextureFormat();

    if (_internalFormatMode==USE_IMAGE_DATA_FORMAT)
    {
        internalFormat = image.getInternalTextureFormat();
    }
    else if (_internalFormatMode==USE_USER_DEFINED_FORMAT)
    {
        internalFormat = _internalFormat;
    }
    else
    {

        const unsigned int contextID = 0; // state.getContextID();  // set to 0 right now, assume same parameters for each graphics context...
        const Extensions* extensions = getExtensions(contextID,true);

        switch(_internalFormatMode)
        {
        case(USE_ARB_COMPRESSION):
            if (extensions->isTextureCompressionARBSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(1): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(2): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(3): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(4): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_RGB): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(GL_RGBA): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_ALPHA): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(GL_LUMINANCE): internalFormat = GL_COMPRESSED_LUMINANCE_ARB; break;
                    case(GL_LUMINANCE_ALPHA): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(GL_INTENSITY): internalFormat = GL_COMPRESSED_INTENSITY_ARB; break;
                }
            }
            break;

        case(USE_S3TC_DXT1_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):        internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_S3TC_DXT1c_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):        
                    case(4):        
                    case(GL_RGB):   
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_S3TC_DXT1a_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):        
                    case(4):        
                    case(GL_RGB):   
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_S3TC_DXT3_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_S3TC_DXT5_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_PVRTC_2BPP_COMPRESSION):
            if (extensions->isTextureCompressionPVRTC2BPPSupported())
            {
                switch(image.getPixelFormat())
                {
                case(3):
                case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG; break;
                case(4):
                case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG; break;
                default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_PVRTC_4BPP_COMPRESSION):
            if (extensions->isTextureCompressionPVRTC4BPPSupported())
            {
                switch(image.getPixelFormat())
                {
                case(3):
                case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;
                case(4):
                case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;
                default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_ETC_COMPRESSION):
            if (extensions->isTextureCompressionETCSupported())
            {
                switch(image.getPixelFormat())
                {
                case(3):
                case(GL_RGB):   internalFormat = GL_ETC1_RGB8_OES; break;
                default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        case(USE_RGTC1_COMPRESSION):
            if (extensions->isTextureCompressionRGTCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RED_RGTC1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RED_RGTC1_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;
            
        case(USE_RGTC2_COMPRESSION):
            if (extensions->isTextureCompressionRGTCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            break;

        default:
            break;
        }
    }
    
    _internalFormat = internalFormat;

    // GLES doesn't cope with internal formats of 1,2,3 and 4 so map them to the appropriate equivilants.
    if (_internalFormat==1) _internalFormat = GL_LUMINANCE;
    if (_internalFormat==2) _internalFormat = GL_LUMINANCE_ALPHA;
    if (_internalFormat==3) _internalFormat = GL_RGB;
    if (_internalFormat==4) _internalFormat = GL_RGBA;

    computeInternalFormatType();
    
    //OSG_NOTICE<<"Internal format="<<std::hex<<internalFormat<<std::dec<<std::endl;
}

void Texture::computeInternalFormatType() const
{
    // Here we could also precompute the _sourceFormat if it is not set,
    // since it is different for different internal formats
    // (i.e. rgba integer texture --> _sourceFormat = GL_RGBA_INTEGER_EXT)
    // Should we do this?  ( Art, 09. Sept. 2007)
    
    // compute internal format type based on the internal format
    switch(_internalFormat)
    {
        case GL_RGBA32UI_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA8UI_EXT:

        case GL_RGB32UI_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGB8UI_EXT:

        case GL_LUMINANCE32UI_EXT:  
        case GL_LUMINANCE16UI_EXT:   
        case GL_LUMINANCE8UI_EXT:    

        case GL_INTENSITY32UI_EXT:    
        case GL_INTENSITY16UI_EXT:    
        case GL_INTENSITY8UI_EXT:   

        case GL_LUMINANCE_ALPHA32UI_EXT:    
        case GL_LUMINANCE_ALPHA16UI_EXT:    
        case GL_LUMINANCE_ALPHA8UI_EXT :   
            _internalFormatType = UNSIGNED_INTEGER;
            break;

        case GL_RGBA32I_EXT:
        case GL_RGBA16I_EXT:
        case GL_RGBA8I_EXT:

        case GL_RGB32I_EXT:
        case GL_RGB16I_EXT:
        case GL_RGB8I_EXT:

        case GL_LUMINANCE32I_EXT:    
        case GL_LUMINANCE16I_EXT:    
        case GL_LUMINANCE8I_EXT:    

        case GL_INTENSITY32I_EXT:    
        case GL_INTENSITY16I_EXT:    
        case GL_INTENSITY8I_EXT:    

        case GL_LUMINANCE_ALPHA32I_EXT:    
        case GL_LUMINANCE_ALPHA16I_EXT:    
        case GL_LUMINANCE_ALPHA8I_EXT:    
            _internalFormatType = SIGNED_INTEGER;
            break;
        
        case GL_RGBA32F_ARB:
        case GL_RGBA16F_ARB:

        case GL_RGB32F_ARB:
        case GL_RGB16F_ARB:

        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE16F_ARB:    

        case GL_INTENSITY32F_ARB:
        case GL_INTENSITY16F_ARB:    

        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:    
            _internalFormatType = FLOAT;
            break;
            
        default:
            _internalFormatType = NORMALIZED;
            break;
    };
}

bool Texture::isCompressedInternalFormat() const
{
    return isCompressedInternalFormat(getInternalFormat());
}

bool Texture::isCompressedInternalFormat(GLint internalFormat)
{
    switch(internalFormat)
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
        case(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT):
        case(GL_COMPRESSED_RED_RGTC1_EXT):
        case(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT):
        case(GL_COMPRESSED_RED_GREEN_RGTC2_EXT):
        case(GL_ETC1_RGB8_OES):  
        case(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG):
        case(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG):
        case(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG):
            return true;
        default:
            return false;
    }
}

void Texture::getCompressedSize(GLenum internalFormat, GLint width, GLint height, GLint depth, GLint& blockSize, GLint& size)
{
    if (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
        blockSize = 8;
    else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
        blockSize = 16;
    else if (internalFormat == GL_ETC1_RGB8_OES)
        blockSize = 16;
    else if (internalFormat == GL_COMPRESSED_RED_RGTC1_EXT || internalFormat == GL_COMPRESSED_SIGNED_RED_RGTC1_EXT)
        blockSize = 8;
    else if (internalFormat == GL_COMPRESSED_RED_GREEN_RGTC2_EXT || internalFormat == GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT)
        blockSize = 16;    
    else if (internalFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG || internalFormat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG)
    {
         blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
         GLint widthBlocks = width / 8;
         GLint heightBlocks = height / 4;
         GLint bpp = 2;
         
         // Clamp to minimum number of blocks
         if(widthBlocks < 2)
             widthBlocks = 2;
         if(heightBlocks < 2)
             heightBlocks = 2;
         
         size = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);    
         return;
     }
    else if (internalFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG || internalFormat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG)
    {
         blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
         GLint widthBlocks = width / 4;
         GLint heightBlocks = height / 4;
         GLint bpp = 4;
         
         // Clamp to minimum number of blocks
         if(widthBlocks < 2)
             widthBlocks = 2;
         if(heightBlocks < 2)
             heightBlocks = 2;
         
         size = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);    
         return;
    }
    else
    {
        blockSize = 0;
    }
         
    size = ((width+3)/4)*((height+3)/4)*depth*blockSize;        
}

void Texture::applyTexParameters(GLenum target, State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    WrapMode ws = _wrap_s, wt = _wrap_t, wr = _wrap_r;

    // GL_IBM_texture_mirrored_repeat, fall-back REPEAT
    if (!extensions->isTextureMirroredRepeatSupported())
    {
        if (ws == MIRROR)
            ws = REPEAT;
        if (wt == MIRROR)
            wt = REPEAT;
        if (wr == MIRROR)
            wr = REPEAT;
    }

    // GL_EXT_texture_edge_clamp, fall-back CLAMP
    if (!extensions->isTextureEdgeClampSupported())
    {
        if (ws == CLAMP_TO_EDGE)
            ws = CLAMP;
        if (wt == CLAMP_TO_EDGE)
            wt = CLAMP;
        if (wr == CLAMP_TO_EDGE)
            wr = CLAMP;
    }

    if(!extensions->isTextureBorderClampSupported())
    {
        if(ws == CLAMP_TO_BORDER)
            ws = CLAMP;
        if(wt == CLAMP_TO_BORDER)
            wt = CLAMP;
        if(wr == CLAMP_TO_BORDER)
            wr = CLAMP;
    }

    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE) 
        if (ws == CLAMP) ws = CLAMP_TO_EDGE;
        if (wt == CLAMP) wt = CLAMP_TO_EDGE;
        if (wr == CLAMP) wr = CLAMP_TO_EDGE;
    #endif
    
    const Image * image = getImage(0);
    if( image &&
        image->isMipmap() &&
        extensions->isTextureMaxLevelSupported() &&
        int( image->getNumMipmapLevels() ) <
            Image::computeNumberOfMipmapLevels( image->s(), image->t(), image->r() ) )
            glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, image->getNumMipmapLevels() - 1 );    


    glTexParameteri( target, GL_TEXTURE_WRAP_S, ws );
    
    if (target!=GL_TEXTURE_1D) glTexParameteri( target, GL_TEXTURE_WRAP_T, wt );
    
    if (target==GL_TEXTURE_3D) glTexParameteri( target, GL_TEXTURE_WRAP_R, wr );

    
    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _min_filter);
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _mag_filter);

    // Art: I think anisotropic filtering is not supported by the integer textures
    if (extensions->isTextureFilterAnisotropicSupported() &&
        _internalFormatType != SIGNED_INTEGER && _internalFormatType != UNSIGNED_INTEGER)
    {
        // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
        // by gl.h (or via glext.h) or by include/osg/Texture.
        glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
    }

    if (extensions->isTextureBorderClampSupported())
    {

        #ifndef GL_TEXTURE_BORDER_COLOR
            #define GL_TEXTURE_BORDER_COLOR     0x1004
        #endif


        if (_internalFormatType == SIGNED_INTEGER)
        {
            GLint color[4] = {(GLint)_borderColor.r(), (GLint)_borderColor.g(), (GLint)_borderColor.b(), (GLint)_borderColor.a()};
            extensions->glTexParameterIiv(target, GL_TEXTURE_BORDER_COLOR, color);
        }else if (_internalFormatType == UNSIGNED_INTEGER)
        {
            GLuint color[4] = {(GLuint)_borderColor.r(), (GLuint)_borderColor.g(), (GLuint)_borderColor.b(), (GLuint)_borderColor.a()};
            extensions->glTexParameterIuiv(target, GL_TEXTURE_BORDER_COLOR, color);
        }else{
            GLfloat color[4] = {(GLfloat)_borderColor.r(), (GLfloat)_borderColor.g(), (GLfloat)_borderColor.b(), (GLfloat)_borderColor.a()};
            glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
        }
    }

    // integer textures are not supported by the shadow
    // GL_TEXTURE_1D_ARRAY_EXT could be included in the check below but its not yet implemented in OSG
    if (extensions->isShadowSupported() && 
        (target == GL_TEXTURE_2D || target == GL_TEXTURE_1D || target == GL_TEXTURE_RECTANGLE || target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_2D_ARRAY_EXT ) &&
        _internalFormatType != SIGNED_INTEGER && _internalFormatType != UNSIGNED_INTEGER)
    {
        if (_use_shadow_comparison)
        {
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
            glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC_ARB, _shadow_compare_func);
            glTexParameteri(target, GL_DEPTH_TEXTURE_MODE_ARB, _shadow_texture_mode);

            // if ambient value is 0 - it is default behaviour of GL_ARB_shadow
            // no need for GL_ARB_shadow_ambient in this case
            if (extensions->isShadowAmbientSupported() && _shadow_ambient > 0)
            {
                glTexParameterf(target, TEXTURE_COMPARE_FAIL_VALUE_ARB, _shadow_ambient);
            }
        }
        else 
        {
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
        }
    }

    getTextureParameterDirty(state.getContextID()) = false;

}

void Texture::computeRequiredTextureDimensions(State& state, const osg::Image& image,GLsizei& inwidth, GLsizei& inheight,GLsizei& numMipmapLevels) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    int width,height;

    if( !_resizeNonPowerOfTwoHint && extensions->isNonPowerOfTwoTextureSupported(_min_filter) )
    {
        width = image.s();
        height = image.t();
    }
    else
    {
        width = Image::computeNearestPowerOfTwo(image.s()-2*_borderWidth)+2*_borderWidth;
        height = Image::computeNearestPowerOfTwo(image.t()-2*_borderWidth)+2*_borderWidth;
    }

    // cap the size to what the graphics hardware can handle.
    if (width>extensions->maxTextureSize()) width = extensions->maxTextureSize();
    if (height>extensions->maxTextureSize()) height = extensions->maxTextureSize();

    inwidth = width;
    inheight = height;
    
    if( _min_filter == LINEAR || _min_filter == NEAREST)
    {
        numMipmapLevels = 1;
    }
    else if( image.isMipmap() )
    {
        numMipmapLevels = image.getNumMipmapLevels();
    }
    else
    {
        numMipmapLevels = 1;
        for(int s=1; s<width || s<height; s <<= 1, ++numMipmapLevels) {}
    }
}

bool Texture::areAllTextureObjectsLoaded() const
{
    for(unsigned int i=0;i<DisplaySettings::instance()->getMaxNumberOfGraphicsContexts();++i)
    {
        if (_glObjectList[i].m_nTexture == 0u)
        {
            return false;
        }
    }
    return true;
}


void Texture::applyTexImage2D_load(State& state, GLenum target, const Image* image, GLsizei inwidth, GLsizei inheight,GLsizei numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    // select the internalFormat required for the texture.
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());

    // If the texture's internal format is a compressed type, then the
    // user is requesting that the graphics card compress the image if it's
    // not already compressed. However, if the image is not a multiple of 
    // four in each dimension the subsequent calls to glTexSubImage* will
    // fail. Revert to uncompressed format in this case.
    if (isCompressedInternalFormat(_internalFormat) &&
        (inwidth > 2u && inheight > 2u) && 
        (((inwidth >> 2) << 2) != inwidth || ((inheight >> 2) << 2) != inheight))
    {
        //OSG_NOTICE<<"Received a request to compress an image, but image size is not a multiple of four ("<<inwidth<<"x"<<inheight<<"). Reverting to uncompressed.\n";
        switch(_internalFormat)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            case GL_ETC1_RGB8_OES:
            case GL_COMPRESSED_RGB: _internalFormat = GL_RGB; break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
            case GL_COMPRESSED_RGBA: _internalFormat = GL_RGBA; break;
            case GL_COMPRESSED_ALPHA: _internalFormat = GL_ALPHA; break;
            case GL_COMPRESSED_LUMINANCE: _internalFormat = GL_LUMINANCE; break;
            case GL_COMPRESSED_LUMINANCE_ALPHA: _internalFormat = GL_LUMINANCE_ALPHA; break;
            case GL_COMPRESSED_INTENSITY: _internalFormat = GL_INTENSITY; break;
            case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            case GL_COMPRESSED_RED_RGTC1_EXT: _internalFormat = GL_RED; break;
            case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            case GL_COMPRESSED_RED_GREEN_RGTC2_EXT: _internalFormat = GL_LUMINANCE_ALPHA; break;
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    bool useClientStorage = extensions->isClientStorageSupported() && getClientStorageHint();
    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_TRUE);

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_PRIORITY,0.0f);
        #endif

        #ifdef GL_TEXTURE_STORAGE_HINT_APPLE    
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
        #endif
    }

    unsigned char* dataPtr = (unsigned char*)image->data();

    // OSG_NOTICE<<"inwidth="<<inwidth<<" inheight="<<inheight<<" image->getFileName()"<<image->getFileName()<<std::endl;

    bool needImageRescale = inwidth!=image->s() || inheight!=image->t();
    if (needImageRescale)
    {
        // resize the image to power of two.
        
        if (image->isMipmap())
        {
            return;
        }
        else if (compressed_image)
        {
            return;
        }
        
        unsigned int newTotalSize = osg::Image::computeRowWidthInBytes(inwidth,image->getPixelFormat(),image->getDataType(),image->getPacking())*inheight;
        dataPtr = (unsigned char *)malloc(newTotalSize);
        if (!dataPtr)
        {
            return;
        }

        PixelStorageModes psm;
        psm.pack_alignment = image->getPacking();
        psm.unpack_alignment = image->getPacking();

        // rescale the image to the correct size.
        gluScaleImage(&psm, image->getPixelFormat(),
                        image->s(),image->t(),image->getDataType(),image->data(),
                        inwidth,inheight,image->getDataType(),
                        dataPtr);

    }

    bool mipmappingRequired = _min_filter != LINEAR && _min_filter != NEAREST;
    bool useHardwareMipMapGeneration = mipmappingRequired && (!image->isMipmap() && isHardwareMipmapGenerationEnabled(state));
    bool useGluBuildMipMaps = mipmappingRequired && (!useHardwareMipMapGeneration && !image->isMipmap());

    GLBufferObject* pbo = image->getOrCreateGLBufferObject(contextID);
    if (pbo && !needImageRescale && !useGluBuildMipMaps)
    {
        state.bindPixelBufferObject(pbo);
        dataPtr = reinterpret_cast<unsigned char*>(pbo->getOffset(image->getBufferIndex()));
    }
    else
    {
        pbo = 0;
    }

    if( !mipmappingRequired || useHardwareMipMapGeneration)
    {

        GenerateMipmapMode mipmapResult = mipmapBeforeTexImage(state, useHardwareMipMapGeneration);

        if ( !compressed_image)
        {
            numMipmapLevels = 1;

            glTexImage2D( target, 0, _internalFormat,
                inwidth, inheight, _borderWidth,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                dataPtr);

        }
        else if (extensions->isCompressedTexImage2DSupported())
        {
            numMipmapLevels = 1;

            GLint blockSize, size;
            getCompressedSize(_internalFormat, inwidth, inheight, 1, blockSize,size);

            extensions->glCompressedTexImage2D(target, 0, _internalFormat, 
                inwidth, inheight,0, 
                size, 
                dataPtr);
        }

        mipmapAfterTexImage(state, mipmapResult);
    }
    else
    {
        // we require mip mapping.
        if(image->isMipmap())
        {

            // image is mip mapped so we take the mip map levels from the image.
        
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = inwidth;
            int height = inheight;

            if( !compressed_image )
            {
                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {

                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    glTexImage2D( target, k, _internalFormat,
                         width, height, _borderWidth,
                        (GLenum)image->getPixelFormat(),
                        (GLenum)image->getDataType(),
                        dataPtr + image->getMipmapOffset(k));

                    width >>= 1;
                    height >>= 1;
                }
            }
            else if (extensions->isCompressedTexImage2DSupported())
            {
                GLint blockSize, size;

                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {
                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    getCompressedSize(_internalFormat, width, height, 1, blockSize,size);
                    
                    extensions->glCompressedTexImage2D(target, k, _internalFormat, 
                                                       width, height, _borderWidth, 
                                                       size, dataPtr + image->getMipmapOffset(k));

                    width >>= 1;
                    height >>= 1;
                }
            }
        }
        else
        {
            if ( !compressed_image)
            {
                numMipmapLevels = 0;

                gluBuild2DMipmaps( target, _internalFormat,
                    inwidth,inheight,
                    (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                    dataPtr);

                int width  = image->s();
                int height = image->t();
                for( numMipmapLevels = 0 ; (width || height) ; ++numMipmapLevels)
                {
                    width >>= 1;
                    height >>= 1;
                }
            }
        }
    }

    if (pbo)
    {
        state.unbindPixelBufferObject();

        const BufferObject* bo = image->getBufferObject();
        if (bo->getCopyDataAndReleaseGLBufferObject())
        {
            bo->releaseGLObjects(&state);
        }
    }

    if (needImageRescale)
    {
        // clean up the resized image.
        free(dataPtr);
    }
    
    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_FALSE);
    }
}


bool Texture::isHardwareMipmapGenerationEnabled(const State& state) const
{
    if (_useHardwareMipMapGeneration)
    {
        unsigned int contextID = state.getContextID();
        const Extensions* extensions = getExtensions(contextID,true);

        if (extensions->isGenerateMipMapSupported())
        {
            return true;
        }

        const FBOExtensions* fbo_ext = FBOExtensions::instance(contextID,true);

        if (fbo_ext->glGenerateMipmap)
        {
            return true;
        }
    }

    return false;
}

Texture::GenerateMipmapMode Texture::mipmapBeforeTexImage(const State& state, bool hardwareMipmapOn) const
{
    if (hardwareMipmapOn)
    {
#if defined( OSG_GLES2_AVAILABLE ) || defined( OSG_GL3_AVAILABLE ) 
        return GENERATE_MIPMAP;
#else
        int width = getTextureWidth();
        int height = getTextureHeight();

        //quick bithack to determine whether width or height are non-power-of-two
        if ((width & (width - 1)) || (height & (height - 1)))
        {
            //GL_GENERATE_MIPMAP_SGIS with non-power-of-two textures on NVIDIA hardware
            //is extremely slow. Use glGenerateMipmapEXT() instead if supported.
            if (_internalFormatType != SIGNED_INTEGER &&
                _internalFormatType != UNSIGNED_INTEGER)
            {
                if (FBOExtensions::instance(state.getContextID(), true)->glGenerateMipmap)
                {
                    return GENERATE_MIPMAP;
                }
            }
        }

        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        return GENERATE_MIPMAP_TEX_PARAMETER;
#endif
    }
    return GENERATE_MIPMAP_NONE;
}

void Texture::mipmapAfterTexImage(State& state, GenerateMipmapMode beforeResult) const
{
    switch (beforeResult)
    {
        case GENERATE_MIPMAP:
        {
            unsigned int contextID = state.getContextID();
            const TextureObject &texObj = _glObjectList[contextID];
            if (texObj.m_nTexture != 0u)
            {
                osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(contextID, true);
                fbo_ext->glGenerateMipmap(texObj.m_eTarget);
            }
            break;
        }
        case GENERATE_MIPMAP_TEX_PARAMETER:
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
            break;
        case GENERATE_MIPMAP_NONE:
            break;
    }
}

void Texture::generateMipmap(State& state) const
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    const TextureObject &texObj = _glObjectList[contextID];

    // if not initialized before, then do nothing
    if (texObj.m_nTexture == 0) return;

    _texMipmapGenerationDirtyList[contextID] = 0;
    
    // if internal format does not provide automatic mipmap generation, then do manual allocation
    if (_internalFormatType == SIGNED_INTEGER || _internalFormatType == UNSIGNED_INTEGER)
    {
        allocateMipmap(state);
        return;
    }
    
    // get fbo extension which provides us with the glGenerateMipmapEXT function
    osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(state.getContextID(), true);

    // check if the function is supported
    if (fbo_ext->glGenerateMipmap)
    {
        texObj.bind();
        fbo_ext->glGenerateMipmap(texObj.m_eTarget);
        
        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);
    
    // if the function is not supported, then do manual allocation
    }else
    {
        allocateMipmap(state);
    }
    
}

void Texture::compileGLObjects(State& state) const
{
    apply(state);
}

void Texture::releaseGLObjects(State* state) const
{
//    if (state) OSG_NOTICE<<"Texture::releaseGLObjects contextID="<<state->getContextID()<<std::endl;
//    else OSG_NOTICE<<"Texture::releaseGLObjects no State "<<std::endl;

    if (!state) const_cast<Texture*>(this)->dirtyTextureObject();
    else
    {
        unsigned int contextID = state->getContextID();
        const GLuint nTex = _glObjectList[contextID].m_nTexture;
        if (nTex != 0u)
        {
            GLObjectPool::instance()->deleteTexture(contextID, nTex);
            _glObjectList[contextID].m_nTexture = 0;
        }
    }
}

Texture::Extensions* Texture::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Texture::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Texture::Extensions::Extensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        return;
    }
    
    const char* renderer = (const char*) glGetString(GL_RENDERER);
    std::string rendererString(renderer ? renderer : "");

    bool builtInSupport = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;
    
    _isMultiTexturingSupported = builtInSupport || OSG_GLES1_FEATURES || 
                                 isGLExtensionOrVersionSupported( contextID,"GL_ARB_multitexture", 1.3f) ||
                                 isGLExtensionOrVersionSupported(contextID,"GL_EXT_multitexture", 1.3f);
                                 
    _isTextureFilterAnisotropicSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_filter_anisotropic");
    
    _isTextureCompressionARBSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_compression", 1.3f);
    
    _isTextureCompressionS3TCSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_compression_s3tc");

    _isTextureCompressionPVRTC2BPPSupported = isGLExtensionSupported(contextID,"GL_IMG_texture_compression_pvrtc");

    _isTextureCompressionPVRTC4BPPSupported = _isTextureCompressionPVRTC2BPPSupported;//covered by same extension

    _isTextureCompressionETCSupported = isGLExtensionSupported(contextID,"GL_OES_compressed_ETC1_RGB8_texture");
    

    _isTextureCompressionRGTCSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_compression_rgtc");

    _isTextureCompressionPVRTCSupported = isGLExtensionSupported(contextID,"GL_IMG_texture_compression_pvrtc");

    _isTextureMirroredRepeatSupported = builtInSupport || 
                                        isGLExtensionOrVersionSupported(contextID,"GL_IBM_texture_mirrored_repeat", 1.4f) ||
                                        isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_mirrored_repeat", 1.4f);
                                        
    _isTextureEdgeClampSupported = builtInSupport ||
                                   isGLExtensionOrVersionSupported(contextID,"GL_EXT_texture_edge_clamp", 1.2f) || 
                                   isGLExtensionOrVersionSupported(contextID,"GL_SGIS_texture_edge_clamp", 1.2f);
                                   

    _isTextureBorderClampSupported = OSG_GL3_FEATURES ||
                                     ((OSG_GL1_FEATURES || OSG_GL2_FEATURES) && isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_border_clamp", 1.3f));
    
    _isGenerateMipMapSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_SGIS_generate_mipmap", 1.4f);

    _isTextureMultisampledSupported = isGLExtensionSupported(contextID,"GL_ARB_texture_multisample");
                                  
    _isShadowSupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID,"GL_ARB_shadow");
    
    _isShadowAmbientSupported = isGLExtensionSupported(contextID,"GL_ARB_shadow_ambient");

    _isClientStorageSupported = isGLExtensionSupported(contextID,"GL_APPLE_client_storage");

    _isNonPowerOfTwoTextureNonMipMappedSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_non_power_of_two", 2.0) || isGLExtensionSupported(contextID,"GL_APPLE_texture_2D_limited_npot");

    _isNonPowerOfTwoTextureMipMappedSupported = builtInSupport || _isNonPowerOfTwoTextureNonMipMappedSupported;
    
    _isTextureIntegerEXTSupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID, "GL_EXT_texture_integer");

    if (rendererString.find("GeForce FX")!=std::string::npos)
    {
        _isNonPowerOfTwoTextureMipMappedSupported = false;
    }

    _maxTextureSize=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&_maxTextureSize);

    if( _isMultiTexturingSupported )
    {
       _numTextureUnits = 0;
       #if defined(OSG_GLES2_AVAILABLE) || defined(OSG_GL3_AVAILABLE)
           glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&_numTextureUnits);
       #else
           if (osg::asciiToFloat(version)>=2.0)
           {
               glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&_numTextureUnits);
           }
           else
           {
               glGetIntegerv(GL_MAX_TEXTURE_UNITS,&_numTextureUnits);
           }
       #endif
    }
    else
    {
       _numTextureUnits = 1;
    }

    setGLExtensionFuncPtr(_glCompressedTexImage2D,"glCompressedTexImage2D","glCompressedTexImage2DARB");
    setGLExtensionFuncPtr(_glCompressedTexSubImage2D,"glCompressedTexSubImage2D","glCompressedTexSubImage2DARB");
    setGLExtensionFuncPtr(_glGetCompressedTexImage,"glGetCompressedTexImage","glGetCompressedTexImageARB");;
    setGLExtensionFuncPtr(_glTexImage2DMultisample, "glTexImage2DMultisample", "glTexImage2DMultisampleARB");

    setGLExtensionFuncPtr(_glTexParameterIiv, "glTexParameterIiv", "glTexParameterIivARB");
    setGLExtensionFuncPtr(_glTexParameterIuiv, "glTexParameterIuiv", "glTexParameterIuivARB");


    if (_glTexParameterIiv == NULL) setGLExtensionFuncPtr(_glTexParameterIiv, "glTexParameterIivEXT");
    if (_glTexParameterIuiv == NULL) setGLExtensionFuncPtr(_glTexParameterIuiv, "glTexParameterIuivEXT");

    _isTextureMaxLevelSupported = ( getGLVersionNumber() >= 1.2f );
}


}
