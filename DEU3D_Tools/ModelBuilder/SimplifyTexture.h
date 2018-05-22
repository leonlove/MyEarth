#ifndef SIMPLIFY_TEXTURE_H_2383A98A_19F9_4EB4_8A5F_5F97A2AB546C_INCLUDE
#define SIMPLIFY_TEXTURE_H_2383A98A_19F9_4EB4_8A5F_5F97A2AB546C_INCLUDE

#include <osg/Texture2D>
#include <osg/BoundingSphere>

int Do_2_Log(int iValue);
GLboolean legalFormat(GLenum format);
double computeScaleRatio(const osg::Viewport& W, const osg::Matrixd& P, const osg::Matrixd& M);
double computeDistanceByPixelSize(double dblSphereRadius, double fPixelSize, unsigned int nResolutionH, unsigned int nResolutionV);
bool SimplifyTexture2Color(osg::Image *pImage, osg::Vec4 &color);
osg::ref_ptr<osg::Image> CompressImage2DDS(osg::Image *pImage, const unsigned int iscale, const std::string &strAppPath);
osg::ref_ptr<osg::Image> CompressImage2DDS(osg::Image *pImage, const unsigned int iscale);


#endif
