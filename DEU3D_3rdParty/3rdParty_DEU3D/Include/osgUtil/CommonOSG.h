#ifndef COMMON_OSG_H_C06E2523_D5D6_472C_8A60_04A4C62386EE_INCLUDE
#define COMMON_OSG_H_C06E2523_D5D6_472C_8A60_04A4C62386EE_INCLUDE

#include "Export"

#include <string>
#include <osg/Matrix>

#define UNUSED_VAR(x)    (x)

namespace osgUtil
{

OSGUTIL_EXPORT bool isMatrixPerspective(const osg::Matrixf &mtxProj);
OSGUTIL_EXPORT bool isMatrixOrtho(const osg::Matrixf &mtxProj);
OSGUTIL_EXPORT bool isMatrixPerspective(const osg::Matrixd &mtxProj);
OSGUTIL_EXPORT bool isMatrixOrtho(const osg::Matrixd &mtxProj);
OSGUTIL_EXPORT osg::Quat calcRotationByCoordAndAngle(const osg::Vec2d &ptCoord, double dblAzimuth, double dblPitch);

OSGUTIL_EXPORT osg::Vec3d convertWorld2Globe(const osg::Vec3d &point);
OSGUTIL_EXPORT osg::Vec3d convertGlobe2World(const osg::Vec3d &point);

}
#endif
