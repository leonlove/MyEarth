
#include <osgUtil/ClusterCullingCreator>
#include <osg/ClusterCullingCallback>
#include <osg/CoordinateSystemNode>

namespace osgUtil
{

osg::ClusterCullingCallback* createClusterCullingCallbackByHeightField(osg::HeightField *pHeightField)
{
    //// make sure we are dealing with a geocentric database
    //if (!_dataSet->mapLatLongsToXYZ()) return 0;

    if (!pHeightField) return NULL;
    osg::EllipsoidModel *pEllipsoid = osg::EllipsoidModel::instance();
    const double dblPlanetRadius = (pEllipsoid->getRadiusPolar() + pEllipsoid->getRadiusEquator()) * 0.5;
    const unsigned int nNumColumns = pHeightField->getNumColumns();
    const unsigned int nNumRows = pHeightField->getNumRows();

    const osg::Vec3d ptOriginal(pHeightField->getOrigin());
    const osg::Vec2d vecInterval(pHeightField->getXInterval(),pHeightField->getYInterval());
    const osg::Vec3d ptCenterCoord
    (
        ptOriginal.x() + vecInterval.x() * ((double)(nNumColumns - 1)) * 0.5,
        ptOriginal.y() + vecInterval.y() * ((double)(nNumRows - 1)) * 0.5,
        ptOriginal.z()
    );

    osg::Vec3d ptCenter;
    pEllipsoid->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), ptCenterCoord.z(), ptCenter.x(), ptCenter.y(), ptCenter.z());

    const osg::Vec3d vecCenterNormal = -pEllipsoid->computeLocalPlumbLine(ptCenterCoord.y(), ptCenterCoord.x());
    osg::Vec3d vecGridNormal = vecCenterNormal;

    double min_dot_product = 1.0;
    double max_cluster_culling_height = 0.0;
    double max_cluster_culling_radius = 0.0;

    osg::Vec3d ptCoord = ptOriginal;
    for(unsigned r = 0; r < nNumRows; ++r)
    {
        for(unsigned c = 0; c < nNumColumns; ++c)
        {
            ptCoord.z() = pHeightField->getHeight(c, r);
            osg::Vec3d point;
            const double dblHeight = fabs(ptCoord.z());

            pEllipsoid->convertLatLongHeightToXYZ(ptCoord.y(), ptCoord.x(), ptCoord.z(), point.x(), point.y(), point.z());

            const osg::Vec3d dv = point - ptCenter;
            const double d = dv.length();
            const double theta = acos(dblPlanetRadius / (dblPlanetRadius + dblHeight));
            const double phi = 2.0 * asin (d * 0.5 / dblPlanetRadius);      // 得到瓦片的半张角
            const double beta = theta + phi;
            const double cutoff = osg::PI_2 - 0.1;

            if(phi >= cutoff || beta >= cutoff)
            {
                return NULL;
            }

            const double local_dot_product = -sin(theta + phi);
            const double local_m = dblPlanetRadius * (1.0 / cos(theta + phi) - 1.0);
            const double local_radius = dblPlanetRadius * tan(beta);
            min_dot_product = osg::minimum(min_dot_product, local_dot_product);
            max_cluster_culling_height = osg::maximum(max_cluster_culling_height, local_m);
            max_cluster_culling_radius = osg::maximum(max_cluster_culling_radius, local_radius);

            ptCoord.x() += vecInterval.x();
        }
        ptCoord.y() += vecInterval.y();
    }

    // set up cluster cullling
    osg::ref_ptr<osg::ClusterCullingCallback> pCCC = new osg::ClusterCullingCallback;
    pCCC->setControlPoint(ptCenter + vecCenterNormal * max_cluster_culling_height);
    pCCC->setNormal(vecGridNormal);
    pCCC->setDeviation(min_dot_product);
    pCCC->setRadius(max_cluster_culling_radius);

    return pCCC.release();
}

}

