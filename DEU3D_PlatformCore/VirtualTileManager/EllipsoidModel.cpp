#include "EllipsoidModel.h"
#include <algorithm>

EllipsoidModel::EllipsoidModel(double radiusEquator, double radiusPolar) :
    _radiusEquator(radiusEquator),
    _radiusPolar(radiusPolar)
{
    computeCoefficients(); 
}

void EllipsoidModel::convertLatLongHeightToXYZ(double latitude, double longitude, double height,double& X, double& Y, double& Z) const
{
    double sin_latitude = sin(latitude);
    double cos_latitude = cos(latitude);
    double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);
    X = (N + height) * cos_latitude * cos(longitude);
    Y = (N + height) * cos_latitude * sin(longitude);
    Z = (N * (1 - _eccentricitySquared) + height) * sin_latitude;
}


void EllipsoidModel::convertXYZToLatLongHeight(double X, double Y, double Z, double& latitude, double& longitude, double& height) const
{
    double p = sqrt(X * X + Y * Y);
    double theta = atan2(Z * _radiusEquator, (p * _radiusPolar));
    double eDashSquared = (_radiusEquator * _radiusEquator - _radiusPolar * _radiusPolar) / (_radiusPolar * _radiusPolar);

    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    latitude = atan((Z + eDashSquared * _radiusPolar * sin_theta * sin_theta * sin_theta) / (p - _eccentricitySquared * _radiusEquator * cos_theta * cos_theta * cos_theta));
    longitude = atan2(Y, X);

    double sin_latitude = sin(latitude);
    double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);

    height = p / cos(latitude) - N;
}


double EllipsoidModel::computeLocalSeaLevel(double latitude, double longitude) const
{
    cmm::math::Point3d point;
    convertLatLongHeightToXYZ(latitude, longitude, 0.0, point.x(), point.y(), point.z());
    return point.length();
}

