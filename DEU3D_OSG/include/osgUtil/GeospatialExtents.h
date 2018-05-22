#ifndef GEOSPATIAL_EXTENTS_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#define GEOSPATIAL_EXTENTS_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE

#include "Export"

#include <osg/Vec2d>
#include <osg/vec3>
#include <osg/BoundingSphere>

namespace osgUtil
{

class OSGUTIL_EXPORT GeospatialExtents
{
public:

    osg::Vec2d  _min;
    osg::Vec2d  _max;
    bool        _isGeographic;
    
    inline GeospatialExtents() : 
        _min(DBL_MAX,DBL_MAX),
        _max(-DBL_MAX,-DBL_MAX),
        _isGeographic(false) {}

    inline GeospatialExtents(const GeospatialExtents& rhs):
        _min(rhs._min),
        _max(rhs._max),
        _isGeographic(rhs._isGeographic) {}

  /** Initialize GeospatialExtents from doubles and Geographic indicator.
    * Order of xmin vs xmax and ymin vs ymax are significant.
  */
    inline GeospatialExtents(double xmin, double ymin, double xmax, double ymax, bool isGeographic) :
        _min(xmin, ymin),
        _max(xmax, ymax),
        _isGeographic(isGeographic) {}
        
    GeospatialExtents& operator = (const GeospatialExtents& rhs)
    {
        _min = rhs._min;
        _max = rhs._max;
        _isGeographic = rhs._isGeographic;
        return *this;
    }

    bool operator != (const GeospatialExtents& rhs) const
    {
        return (_min != rhs._min) || (_max != rhs._max) || (_isGeographic != rhs._isGeographic);
    }

    bool operator == (const GeospatialExtents& rhs) const
    {
        return (_min == rhs._min) && (_max == rhs._max) && (_isGeographic == rhs._isGeographic);
    }

    bool equivalent(const GeospatialExtents& rhs, double epsilon=1e-6) const
    {
        return osg::equivalent(_min.x(),rhs._min.x(),epsilon) &&
               osg::equivalent(_min.y(),rhs._min.y(),epsilon) && 
               osg::equivalent(_max.x(),rhs._max.x(),epsilon) && 
               osg::equivalent(_max.y(),rhs._max.y(),epsilon) && 
               _isGeographic == rhs._isGeographic;
    }

    inline double& xMin() { return _min.x(); }
    inline double xMin() const { return _min.x(); }

    inline double& yMin() { return _min.y(); }
    inline double yMin() const { return _min.y(); }

    inline double& xMax() { return _max.x(); }
    inline double xMax() const { return _max.x(); }

    inline double& yMax() { return _max.y(); }
    inline double yMax() const { return _max.y(); }

    inline void init()
    {
        _min.set(DBL_MAX,DBL_MAX);
        _max.set(-DBL_MAX,-DBL_MAX);
    }

    inline void initExtents(double dblMinX = -180.0, double dblMinY = -90.0, double dblMaxX = 180.0, double dblMaxY = 90.0, bool bIsGeographic = true)
    {
        _min.set(dblMinX, dblMinY);
        _max.set(dblMaxX, dblMaxY);
        _isGeographic = bIsGeographic;
    }

    inline bool valid() const
    {
        return _max.x()>=_min.x() &&  _max.y()>=_min.y();
    }

    inline bool nonZeroExtents() const
    {
        return valid() && _max.x()!=_min.x() &&  _max.y()!=_min.y();
    }

    inline double radius() const
    {
        return sqrt((radius2()));
    }

    inline double radius2() const
    {
        return 0.25f*((_max-_min).length2());
    }

    GeospatialExtents intersection(const GeospatialExtents& e, double xoffset) const
    {
        return GeospatialExtents(osg::maximum(xMin(),e.xMin()+xoffset),osg::maximum(yMin(),e.yMin()),
                                 osg::minimum(xMax(),e.xMax()+xoffset),osg::minimum(yMax(),e.yMax()),_isGeographic);
    }

    /** Return true if this bounding box intersects the specified bounding box. */
    bool intersects(const GeospatialExtents& bb) const
    {
        if (_isGeographic)
        {
            // first check vertical axis overlap
            if (osg::maximum(yMin(),bb.yMin()) > osg::minimum(yMax(),bb.yMax())) return false;
            
            // next check if overlaps directly without any 360 degree horizontal shifts.
            if (osg::maximum(xMin(),bb.xMin()) <= osg::minimum(xMax(),bb.xMax())) return true;
            
            // next check if a 360 rotation will produce an overlap
            float rotationAngle = (xMin() > bb.xMin()) ? 360.0 : -360;
            return (osg::maximum(xMin(),bb.xMin()+rotationAngle) <= osg::minimum(xMax(),bb.xMax()+rotationAngle));
        }
        else
        {
            return (osg::maximum(xMin(),bb.xMin()) <= osg::minimum(xMax(),bb.xMax()) &&
                    osg::maximum(yMin(),bb.yMin()) <= osg::minimum(yMax(),bb.yMax()));
        }
    }

 
    void expandBy(const osg::BoundingSphere& sh)
    {
        if (!sh.valid()) return;

        if(sh._center.x()-sh._radius<_min.x()) _min.x() = sh._center.x()-sh._radius;
        if(sh._center.x()+sh._radius>_max.x()) _max.x() = sh._center.x()+sh._radius;

        if(sh._center.y()-sh._radius<_min.y()) _min.y() = sh._center.y()-sh._radius;
        if(sh._center.y()+sh._radius>_max.y()) _max.y() = sh._center.y()+sh._radius;
    }

    inline void expandBy(const osg::Vec3& v)
    {
        if(v.x()<_min.x()) _min.x() = v.x();
        if(v.x()>_max.x()) _max.x() = v.x();

        if(v.y()<_min.y()) _min.y() = v.y();
        if(v.y()>_max.y()) _max.y() = v.y();
    }

    void expandBy(const GeospatialExtents& e)
    {
        if (!e.valid()) return;

        if(e._min.x()<_min.x()) _min.x() = e._min.x();
        if(e._max.x()>_max.x()) _max.x() = e._max.x();

        if(e._min.y()<_min.y()) _min.y() = e._min.y();
        if(e._max.y()>_max.y()) _max.y() = e._max.y();
    }
};

}
#endif
