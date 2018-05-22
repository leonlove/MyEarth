#ifndef TYPE_DEFS_H_4562AA48_8CD3_4295_85E7_8D8BC0C9F21C_INCLUDE
#define TYPE_DEFS_H_4562AA48_8CD3_4295_85E7_8D8BC0C9F21C_INCLUDE

#include <map>

#include <IDProvider\ID.h>
#include <OpenSP\sp.h>
#include <Common\deuMath.h>
#include <osgDB\FileNameUtils>
#include <osg\image>
#include <osg\Vec3d>
#include <osg\MatrixTransform>

#include <Common\DEUBson.h>

#include "IModelBuilder.h"

typedef std::pair<ID, OpenSP::sp<osg::Image>>           SharedImage;
typedef std::map<unsigned int, SharedImage>             SharedTexsValue;
typedef std::map<const std::string, SharedTexsValue>    SharedTexs;



#endif
