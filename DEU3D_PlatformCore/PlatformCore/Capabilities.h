#ifndef CAPABILITIES_H
#define CAPABILITIES_H 1

#include <osg/Referenced>
#include <string>

class Capabilities : public osg::Referenced
{
public:
    /** maximum # of texture units exposed in the fixed-function pipeline */
    int getMaxFFPTextureUnits() const { return _maxFFPTextureUnits; }

    /** maximum # of texture image units exposed in a GPU fragment shader */
    int getMaxGPUTextureUnits() const { return _maxGPUTextureUnits; }

    /** maximum # of texture coordinate sets available in a GPU fragment shader */
    int getMaxGPUTextureCoordSets() const { return _maxGPUTextureCoordSets; }

    /** maximum supported size (in pixels) of a texture */
    int getMaxTextureSize() const { return _maxTextureSize; }

    /** maximum texture size that doesn't cause a slowdown (vendor-specific) */
    int getMaxFastTextureSize() const { return _maxFastTextureSize; }

    /** maximum number of openGL lights */
    int getMaxLights() const { return _maxLights; }

    /** bits in depth buffer */
    int getDepthBufferBits() const { return _depthBits; }

    /** whether the GPU supports shaders */
    bool supportsGLSL(float minimumVersion =1.0f) const { return _supportsGLSL && _GLSLversion >= minimumVersion; }

    bool supportsMultiSample() const {  return _supportsMultiSamples; }

    /** the GLSL version */
    float getGLSLVersion() const { return _GLSLversion; }

    /** the GPU vendor */
    const std::string& getVendor() const { return _vendor;}

    /** the GPU renderer */
    const std::string& getRenderer() const { return _renderer;}

    /** the GPU driver version */
    const std::string& getVersion() const { return _version;}

    /** whether the GPU supports texture arrays */
    bool supportsTextureArrays() const { return _supportsTextureArrays; }

    /** whether the GPU supports OpenGL 3D textures */
    bool supportsTexture3D() const { return _supportsTexture3D; }

    /** whether the GPU supports OpenGL multi-texturing */
    bool supportsMultiTexture() const { return _supportsMultiTexture; }

    /** whether the GPU supports OpenGL stencil wrapping extensions */
    bool supportsStencilWrap() const { return _supportsStencilWrap; }

    /** whether the GPU supports OpenGL the two-sided stenciling extension */
    bool supportsTwoSidedStencil() const { return _supportsTwoSidedStencil; }

    /** whether the GPU support the texture2dLod() function */
    bool supportsTexture2DLod() const { return _supportsTexture2DLod; }

    /** whether the GPU properly supports updating an existing texture with a new mipmapped image */
    bool supportsMipmappedTextureUpdates() const { return _supportsMipmappedTextureUpdates; }

    /** whether the GPU supports DEPTH_PACKED_STENCIL buffer */
    bool supportsDepthPackedStencilBuffer() const { return _supportsDepthPackedStencilBuffer; }

    /** whether the GPU supporst occlusion query */
    bool supportsOcclusionQuery() const { return _supportsOcclusionQuery; }

    /** whether the GPU supports DrawInstanced rendering */
    bool supportsDrawInstanced() const { return _supportsDrawInstanced; }

    /** whether the GPU supports Uniform Buffer Objects */
    bool supportsUniformBufferObjects() const { return _supportsUniformBufferObjects; }

    /** maximum size of a uniform buffer block, in bytes */
    int getMaxUniformBlockSize() const { return _maxUniformBlockSize; }


protected:
    Capabilities();

    /** dtor */
    virtual ~Capabilities() { }

private:
    int  _maxFFPTextureUnits;
    int  _maxGPUTextureUnits;
    int  _maxGPUTextureCoordSets;
    int  _maxTextureSize;
    int  _maxFastTextureSize;
    int  _maxLights;
    int  _depthBits;
    bool _supportsGLSL;
    float _GLSLversion;
    bool _supportsTextureArrays;
    bool _supportsTexture3D;
    bool _supportsMultiTexture;
    bool _supportsStencilWrap;
    bool _supportsTwoSidedStencil;
    bool _supportsTexture2DLod;
    bool _supportsMipmappedTextureUpdates;
    bool _supportsDepthPackedStencilBuffer;
    bool _supportsOcclusionQuery;
    bool _supportsDrawInstanced;
    bool _supportsUniformBufferObjects;
    bool _supportsMultiSamples;
    int  _maxUniformBlockSize;
    std::string _vendor;
    std::string _renderer;
    std::string _version;

    public:
        friend class Registry;
};

#endif
