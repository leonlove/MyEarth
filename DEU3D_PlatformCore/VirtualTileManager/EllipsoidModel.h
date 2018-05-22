#ifndef ELLIPSOID_MODEL_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#define ELLIPSOID_MODEL_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#include <common/deuMath.h>

const double WGS_84_RADIUS_EQUATOR = 6378137.0;
const double WGS_84_RADIUS_POLAR = 6356752.3142;

class EllipsoidModel
{
public:
    explicit EllipsoidModel(double radiusEquator = WGS_84_RADIUS_EQUATOR, double radiusPolar = WGS_84_RADIUS_POLAR);
    ~EllipsoidModel() {}

    void setRadiusEquator(double radius) { _radiusEquator = radius; computeCoefficients(); }
    double getRadiusEquator() const { return _radiusEquator; }

    void setRadiusPolar(double radius) { _radiusPolar = radius; computeCoefficients(); }

    double getRadiusPolar() const { return _radiusPolar; }

    void convertLatLongHeightToXYZ(double latitude, double longitude, double height, double& X, double& Y, double& Z) const;

    void convertXYZToLatLongHeight(double X, double Y, double Z, double& latitude, double& longitude, double& height) const;

    bool isWGS84() const {   return(_radiusEquator == WGS_84_RADIUS_EQUATOR && _radiusPolar == WGS_84_RADIUS_POLAR); }

    double computeLocalSeaLevel(double latitude, double longitude) const;

protected:

    void computeCoefficients()
    {
        double flattening = (_radiusEquator-_radiusPolar) / _radiusEquator;
        _eccentricitySquared = 2 * flattening - flattening * flattening;
    }

    double _radiusEquator;
    double _radiusPolar;
    double _eccentricitySquared;
};

#endif

