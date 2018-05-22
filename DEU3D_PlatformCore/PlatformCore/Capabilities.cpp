#include "Capabilities.h"

#include <osg/FragmentProgram>
#include <osg/GraphicsContext>
#include <osg/GL>
#include <osg/GLExtensions>
#include <osg/GL2Extensions>
#include <osg/Texture>
#include <osgViewer/Version>
#include <osg/FrameBufferObject>

#define LC "[Capabilities] "

struct MyGraphicsContext
{
    MyGraphicsContext()
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 0;
        traits->y = 0;
        traits->width = 1;
        traits->height = 1;
        traits->windowDecoration = false;
        traits->doubleBuffer = false;
        traits->sharedContext = 0;
        traits->pbuffer = false;

        if (!_gc.valid())
        {
            // fall back on a mapped window
            traits->pbuffer = false;
            _gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        }

        if (_gc.valid()) 
        {
            _gc->realize();
            _gc->makeCurrent();
        }
    }

    bool valid() const { return _gc.valid() && _gc->isRealized(); }

    osg::ref_ptr<osg::GraphicsContext> _gc;
};

// ---------------------------------------------------------------------------

Capabilities::Capabilities() :
            _maxFFPTextureUnits(1),
            _maxGPUTextureUnits(1),
            _maxGPUTextureCoordSets(1),
            _maxTextureSize(256),
            _maxFastTextureSize(256),
            _maxLights(1),
            _depthBits(0),
            _supportsGLSL(false),
            _GLSLversion(1.0f),
            _supportsTextureArrays(false),
            _supportsMultiTexture(false),
            _supportsStencilWrap(true),
            _supportsTwoSidedStencil(false),
            _supportsTexture2DLod(false),
            _supportsMipmappedTextureUpdates(false),
            _supportsDepthPackedStencilBuffer(false),
            _supportsOcclusionQuery(false),
            _supportsDrawInstanced(false),
            _supportsUniformBufferObjects(false),
            _supportsMultiSamples(false),
            _maxUniformBlockSize(0)
{
    // little hack to force the osgViewer library to link so we can create a graphics context
    osgViewerGetVersion();

    // check the environment in order to disable ATI workarounds
    bool enableATIworkarounds = true;

    // create a graphics context so we can query OpenGL support:
    MyGraphicsContext mgc;

    if (mgc.valid())
    {
        osg::GraphicsContext* gc = mgc._gc.get();
        unsigned int id = gc->getState()->getContextID();
        const osg::GL2Extensions* GL2 = osg::GL2Extensions::Get(id, true);

        _vendor = std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));

        _renderer = std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        _version = std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &_maxFFPTextureUnits);

        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &_maxGPUTextureUnits);

        glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &_maxGPUTextureCoordSets);
#if defined(OSG_GLES2_AVAILABLE)
        int maxVertAttributes = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertAttributes);
        _maxGPUTextureCoordSets = maxVertAttributes - 5; //-5 for vertex, normal, color, tangent and binormal
#endif


        glGetIntegerv(GL_DEPTH_BITS, &_depthBits);


        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);

#if !(defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE))
        // Use the texture-proxy method to determine the maximum texture size 
        for(int s = _maxTextureSize; s > 2; s >>= 1)
        {
            glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, s, s, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0L);
            GLint width = 0;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            if (width == s)
            {
                _maxTextureSize = s;
                break;
            }
        }
#endif

        //PORT@tom, what effect will this have?
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
        glGetIntegerv(GL_MAX_LIGHTS, &_maxLights);
#else
        _maxLights = 1;
#endif
        _supportsGLSL = GL2->isGlslSupported();

        if(_supportsGLSL)
        {
            _GLSLversion = GL2->getLanguageVersion();
        }

        _supportsMultiSamples = osg::isGLExtensionSupported(id, "WGL_ARB_multisample");

        _supportsTextureArrays = _supportsGLSL &&
                                osg::getGLVersionNumber() >= 2.0 &&
                                osg::isGLExtensionSupported(id, "GL_EXT_texture_array");

        _supportsTexture3D = osg::isGLExtensionSupported(id, "GL_EXT_texture3D");

        const bool bSupported = osg::isGLExtensionSupported(id, "WGL_ARB_multisample");

        _supportsMultiTexture = 
            osg::getGLVersionNumber() >= 1.3 ||
            osg::isGLExtensionSupported(id, "GL_ARB_multitexture") ||
            osg::isGLExtensionSupported(id, "GL_EXT_multitexture");

        _supportsStencilWrap = osg::isGLExtensionSupported(id, "GL_EXT_stencil_wrap");

        _supportsTwoSidedStencil = osg::isGLExtensionSupported(id, "GL_EXT_stencil_two_side");

        _supportsDepthPackedStencilBuffer = osg::isGLExtensionSupported(id, "GL_EXT_packed_depth_stencil") || 
                                            osg::isGLExtensionSupported(id, "GL_OES_packed_depth_stencil");

        _supportsOcclusionQuery = osg::isGLExtensionSupported(id, "GL_ARB_occlusion_query");

        _supportsDrawInstanced = osg::isGLExtensionOrVersionSupported(id, "GL_EXT_draw_instanced", 3.1f);

        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_maxUniformBlockSize);

        _supportsUniformBufferObjects = osg::isGLExtensionOrVersionSupported(id, "GL_ARB_uniform_buffer_object", 2.0f);

        if (_supportsUniformBufferObjects && _maxUniformBlockSize == 0)
        {
            _supportsUniformBufferObjects = false;
        }


        //_supportsTexture2DLod = osg::isGLExtensionSupported(id, "GL_ARB_shader_texture_lod");
        //OE_INFO << LC << "  texture2DLod = " << SAYBOOL(_supportsTexture2DLod) << std::endl;

        // ATI workarounds:
        bool isATI = _vendor.find("ATI ") == 0;

        _supportsMipmappedTextureUpdates = isATI && enableATIworkarounds ? false : true;

        _maxFastTextureSize = _maxTextureSize;
    }
}

