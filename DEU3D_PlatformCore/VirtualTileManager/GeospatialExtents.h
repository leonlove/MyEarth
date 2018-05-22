#ifndef GEO_SPATIAL_EXTENT_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#define GEO_SPATIAL_EXTENT_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE

#include <float.h>

namespace vtm
{

class GeospatialExtents
{
public:
    double _minX, _minY;
    double _maxX, _maxY;
    bool   _isGeographic;

    GeospatialExtents();

    GeospatialExtents(double xmin, double ymin, double xmax, double ymax, bool isGeographic);

    inline void initExtents(double dblMinX = -180.0, double dblMinY = -90.0, double dblMaxX = 180.0, double dblMaxY = 90.0, bool bIsGeographic = true)
    {
        _minX = dblMinX;
        _minY = dblMinY;
        _maxX = dblMaxX;
        _maxY = dblMaxY;
        _isGeographic = bIsGeographic;
    }

    inline void expandBy(double x, double y)
    {
        if(x < _minX) _minX = x;
        if(x > _maxX) _maxX = x;

        if(y < _minY) _minY = y;
        if(y > _maxY) _maxY = y;
    }
};
}

#endif
