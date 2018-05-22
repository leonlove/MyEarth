#ifndef RADIAL_H_BD522E5C_F2A3_4D8F_BB75_248BB70A3E20_INCLUDE
#define RADIAL_H_BD522E5C_F2A3_4D8F_BB75_248BB70A3E20_INCLUDE

#include "Export"
#include <osg/Plane>
#include <osg/CoordinateSystemNode>
#include <osg/Camera>

namespace osgUtil
{

class OSGUTIL_EXPORT Radial3
{
public:
    explicit Radial3(void);
    explicit Radial3(const osg::Vec3d &ptOrigin, const osg::Vec3d &vecDirection);
    explicit Radial3(const osg::Camera *pCamera, const osg::Vec2d &ptPosNormalize);
    virtual ~Radial3(void);

    void setOrigin(const osg::Vec3d &ptOrigin);
    const osg::Vec3d &getOrigin(void) const;

    void setDirection(const osg::Vec3d &vecDir);
    const osg::Vec3d &getDirection(void) const;

    osg::Vec3d getPoint(osg::Vec3d::value_type t) const;
    osg::Vec3d operator*(osg::Vec3d::value_type t) const;

    osg::Vec3d::value_type intersectPlane(const osg::Plane &plane) const;
    bool hitPlane(const osg::Plane &plane, osg::Vec3d &point) const;

    unsigned hitSphere(const osg::BoundingSphere &sphere, osg::Vec3d &point1, osg::Vec3d &point2) const;
    unsigned hitEllipsoid(const osg::EllipsoidModel &ellipsoid, osg::Vec3d &point1, osg::Vec3d &point2) const;

protected:
    osg::Vec3d    m_ptOrigin;
    osg::Vec3d    m_vecDirection;
};

}

#endif
