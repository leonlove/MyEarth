#include "GeospatialExtents.h"

namespace vtm
{

GeospatialExtents::GeospatialExtents() : 
    _minX(DBL_MAX),
    _minY(DBL_MAX),
    _maxX(-DBL_MAX),
    _maxY(-DBL_MAX),
    _isGeographic(false)
{
}

GeospatialExtents::GeospatialExtents(double xmin, double ymin, double xmax, double ymax, bool isGeographic) :
    _minX(xmin),
    _minY(ymin),
    _maxX(xmax),
    _maxY(ymax),
    _isGeographic(isGeographic)
{
}

}

