#include "SkyDome.h"

#include "SkyShader.h"

#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PointSprite>
#include <osg/BlendFunc>
#include <osg/FrontFace>
#include <osg/CullFace>
#include <osg/Program>
#include <osg/Camera>
#include <osg/Point>
#include <osg/Shape>
#include <osg/Depth>
#include <osg/Quat>

#include <osg/Geometry>
#include <osg/Billboard>
#include <osgUtil/CullVisitor>
#include <osgDB/ReadFile>
#include <osg/Texture2D>

#include <sstream>
#include <time.h>

#include "StarData.h"
#include "StarData.cpp"
#include "Registry.h"

//---------------------------------------------------------------------------

#define BIN_STARS       -100003
#define BIN_SUN         -100002
#define BIN_MOON        -100001
#define BIN_ATMOSPHERE  -100000

//---------------------------------------------------------------------------

// constucts an ellipsoidal mesh that we will use to draw the atmosphere
osg::Geometry *makeEllipsoidGeometry(const osg::EllipsoidModel *pEllipsoid, double outerRadius, bool genTexCoords = false)
{
    double hae = outerRadius - pEllipsoid->getRadiusEquator();

    osg::Geometry* geom = new osg::Geometry();
    geom->setUseVertexBufferObjects(true);

    int latSegments = 100;
    int lonSegments = 2 * latSegments;

    double segmentSize = 180.0 /(double)latSegments; // degrees

    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->reserve(latSegments * lonSegments);

    osg::Vec2Array* texCoords = 0;
    osg::Vec3Array* normals = 0;
    if(genTexCoords)
    {
        texCoords = new osg::Vec2Array();
        texCoords->reserve(latSegments * lonSegments);
        geom->setTexCoordArray(0, texCoords);

        normals = new osg::Vec3Array();
        normals->reserve(latSegments * lonSegments);
        geom->setNormalArray(normals);
        geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
    el->reserve(latSegments * lonSegments * 6);

    for(int y = 0; y <= latSegments; ++y)
    {
        double lat = -90.0 + segmentSize * (double)y;
        for(int x = 0; x < lonSegments; ++x)
        {
            double lon = -180.0 + segmentSize * (double)x;
            double gx, gy, gz;
            pEllipsoid->convertLatLongHeightToXYZ(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), hae, gx, gy, gz);
            verts->push_back(osg::Vec3(gx, gy, gz));

            if(genTexCoords)
            {
                double s = (lon + 180) / 360.0;
                double t = (lat + 90.0) / 180.0;
                texCoords->push_back(osg::Vec2(s, t));
            }

            if(normals)
            {
                osg::Vec3 normal(gx, gy, gz);
                normal.normalize();
                normals->push_back(normal);
            }

            if(y < latSegments)
            {
                int x_plus_1 = x < lonSegments-1 ? x + 1 : 0;
                int y_plus_1 = y + 1;
                el->push_back(y * lonSegments + x);
                el->push_back(y_plus_1 * lonSegments + x);
                el->push_back(y * lonSegments + x_plus_1);
                el->push_back(y * lonSegments + x_plus_1);
                el->push_back(y_plus_1 * lonSegments + x);
                el->push_back(y_plus_1 * lonSegments + x_plus_1);
            }
        }
    }

    geom->setVertexArray(verts);
    geom->addPrimitiveSet(el);

    return geom;
}

// makes a disc geometry that we'll use to render the sun/moon
osg::Geometry *makeDiscGeometry(double radius)
{
    int segments = 48;
    float deltaAngle = 360.0 / (float)segments;

    osg::Geometry* geom = new osg::Geometry();
    geom->setUseVertexBufferObjects(true);

    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->reserve(1 + segments);
    geom->setVertexArray(verts);

    osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
    el->reserve(1 + 2*segments);
    geom->addPrimitiveSet(el);

    verts->push_back(osg::Vec3(0,0,0)); // center point

    for(int i=0; i<segments; ++i)
    {
        double angle = osg::DegreesToRadians(deltaAngle * (float)i);
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        verts->push_back(osg::Vec3(x, y, 0.0));

        int i_plus_1 = i < segments-1? i+1 : 0;
        el->push_back(0);
        el->push_back(1 + i_plus_1);
        el->push_back(1 + i);
    }

    return geom;
}

/**
* Gets a position from the right ascension, declination and range
* @param ra
*        Right ascension in radians
* @param decl
*        Declination in radians
* @param range
*        Range in meters
*/
osg::Vec3d getPositionFromRADecl(double ra, double decl, double range)
{
    return osg::Vec3(0,range,0) * 
        osg::Matrix::rotate(decl, 1, 0, 0) * 
        osg::Matrix::rotate(ra - osg::PI_2, 0, 0, 1);
}


//---------------------------------------------------------------------------
// Astronomical Math
#define d2r(X) osg::DegreesToRadians(X)
#define r2d(X) osg::RadiansToDegrees(X)
#define nrad(X) { while(X > TWO_PI) X -= TWO_PI; while(X < 0.0) X += TWO_PI; }
#define nrad2(X) { while(X <= -osg::PI) X += TWO_PI; while(X > osg::PI) X -= TWO_PI; }

static const double TWO_PI = (2.0 * osg::PI);
static const double JD2000 = 2451545.0;


double getJulianDate(const cmm::DateTime &dateTime)
{
    int nMonth = dateTime.m_nMonth;
    int nYear = dateTime.m_nYear;
    if(nMonth <= 2)
    {
        nMonth += 12;
        nYear  -= 1;
    }

    const int A = int(nYear / 100);
    const int B = 2 - A + (A / 4);
    const int C = int(365.25 * (nYear + 4716));
    const int D = int(30.6001 *(nMonth + 1));
    return B + C + D + dateTime.m_nDate - 1524.5;
}


/**
* The default EphemerisProvider, provides positions based on freely available models
*/
class DefaultEphemerisProvider : public EphemerisProvider
{
public:
    virtual osg::Vec3d getSunPosition(const cmm::DateTime &dateTime)
    {
        // https://www.cfa.harvard.edu/~wsoon/JuanRamirez09-d/Chang09-OptimalTiltAngleforSolarCollector.pdf
        const double JD = getJulianDate(dateTime);
        const double JD1 = (JD - JD2000);                         // julian time since JD2000 epoch
        const double JC = JD1 / 36525.0;                          // julian century

        const double mu = 282.937348 + 0.00004707624 * JD1 + 0.0004569 * (JC * JC);
        const double epsilon = 280.466457 + 0.985647358 * JD1 + 0.000304 * (JC * JC);

        // orbit eccentricity:
        const double E = 0.01670862 - 0.00004204 * JC;

        // mean anomaly of the perihelion
        const double M = epsilon - mu;

        // perihelion anomaly:
        const double v = M + 
            360.0 * E * sin(d2r(M)) / osg::PI + 
            900.0 * (E * E) * sin(d2r(2 * M)) / 4 * osg::PI - 
            180.0 * (E * E * E) * sin(d2r(M)) / 4.0 * osg::PI;

        // longitude of the sun in ecliptic coordinates:
        double sun_lon = d2r(v - 360.0 + mu); // lambda
        nrad2(sun_lon);

        // angle between the ecliptic plane and the equatorial plane
        const double zeta = d2r(23.4392); // zeta

        // latitude of the sun on the ecliptic plane:
        const double omega = d2r(0.0);

        // latitude of the sun with respect to the equatorial plane (solar m_dblDeclination):
        double sun_lat = asin(sin(sun_lon)*sin(zeta));
        nrad2(sun_lat);

        // finally, adjust for the time of day (rotation of the earth)
        double time_r = dateTime.m_dblHours / 24.0; // 0..1
        nrad(sun_lon); // clamp to 0..TWO_PI
        const double sun_r = sun_lon / TWO_PI; // convert to 0..1

        // rotational difference between UTC and current time
        const double diff_r = sun_r - time_r;
        const double diff_lon = TWO_PI * diff_r;

        // apparent sun longitude.
        double app_sun_lon = sun_lon - diff_lon + osg::PI;
        nrad2(app_sun_lon);

        return osg::Vec3d(
            cos(sun_lat) * cos(-app_sun_lon),
            cos(sun_lat) * sin(-app_sun_lon),
            sin(sun_lat));
    }

    virtual osg::Vec3d getMoonPosition(const cmm::DateTime &dateTime)
    {
        // From http://www.stjarnhimlen.se/comp/ppcomp.html
        //double julianDate = getJulianDate(year, month, date);
        //julianDate += dblHoursUTC /24.0;
        double d = 367 * dateTime.m_nYear - 7 * (dateTime.m_nYear + (dateTime.m_nMonth + 9) / 12) / 4 + 275 * dateTime.m_nMonth / 9 + dateTime.m_nDate - 730530;
        d += (dateTime.m_dblHours / 24.0);

        const double ecl = osg::DegreesToRadians(23.4393 - 3.563E-7 * d);

        const double N = osg::DegreesToRadians(125.1228 - 0.0529538083 * d);
        const double i = osg::DegreesToRadians(5.1454);
        const double w = osg::DegreesToRadians(318.0634 + 0.1643573223 * d);
        const double a = 60.2666;//  (Earth radii)
        const double e = 0.054900;
        const double M = osg::DegreesToRadians(115.3654 + 13.0649929509 * d);
        const double E = M + e*(180.0/osg::PI) * sin(M) * (1.0 + e * cos(M));

        const double xv = a * (cos(E) - e);
        const double yv = a * (sqrt(1.0 - e*e) * sin(E));

        const double v = atan2(yv, xv);
        const double r = sqrt(xv*xv + yv*yv);

        //Compute the geocentric (Earth-centered) position of the moon in the ecliptic coordinate system
        const double xh = r * (cos(N) * cos(v+w) - sin(N) * sin(v+w) * cos(i));
        const double yh = r * (sin(N) * cos(v+w) + cos(N) * sin(v+w) * cos(i));
        const double zh = r * (sin(v+w) * sin(i));

        // calculate the ecliptic latitude and longitude here
        const double lonEcl = atan2 (yh, xh);
        const double latEcl = atan2(zh, sqrt(xh*xh + yh*yh));

        const double xg = r * cos(lonEcl) * cos(latEcl);
        const double yg = r * sin(lonEcl) * cos(latEcl);
        const double zg = r * sin(latEcl);

        const double xe = xg;
        const double ye = yg * cos(ecl) -zg * sin(ecl);
        const double ze = yg * sin(ecl) +zg * cos(ecl);

        double RA        = atan2(ye, xe);
        const double Dec = atan2(ze, sqrt(xe*xe + ye*ye));

        //Just use the average distance from the earth
        const double rg = 6378137.0 + 384400000.0 * 0.2;

        // finally, adjust for the time of day (rotation of the earth)
        const double time_r = dateTime.m_dblHours / 24.0; // 0..1
        const double moon_r = RA / TWO_PI; // convert to 0..1

        // rotational difference between UTC and current time
        const double diff_r = moon_r - time_r;
        const double diff_lon = TWO_PI * diff_r;

        RA -= diff_lon;
        nrad2(RA);

        return getPositionFromRADecl(RA, Dec, rg);
    }
};


//---------------------------------------------------------------------------
SkyDome::SkyDome(void)
    : m_bInitialized(false)
    , m_strLightMode("sky_light")
{
    m_dblStarDetailLevel   = 1.0;
    m_dblAmbientBrightness = 0.6;
    m_dblDiffuseBrightness = 1.0;

    m_bStarsVisible         = true;
    m_bMoonVisible          = true;
    m_bAmtosphereVisiible   = true;
    m_bSunVisible           = true;

    m_DateTime.m_nYear    = 2000;
    m_DateTime.m_nMonth   = 1;
    m_DateTime.m_nDate    = 1;
    m_DateTime.m_dblHours = 5.0;

    m_strMoonFile = cmm::genResourceFileDir() + "moon.jpg";
}


SkyDome::~SkyDome(void)
{
    unInitialize();
}


void SkyDome::unInitialize(void)
{
    if(!m_bInitialized) return;
    if(m_pAtmosphereNode.valid())
    {
        osg::StateSet *pStateSet = m_pAtmosphereNode->asGroup()->getChild(0)->getStateSet();
        pStateSet->removeAttribute(osg::StateAttribute::PROGRAM);
    }
    m_pAtmosphereNode = NULL;

    m_pMoonTransform = NULL;

    if(m_pStarsTransform.valid())
    {
        osg::StateSet *pStateSet = m_pStarsTransform->asGroup()->getChild(0)->asGroup()->getChild(0)->getStateSet();
        pStateSet->removeAttribute(osg::StateAttribute::PROGRAM);
    }
    m_pStarsTransform = NULL;

    if(m_pSunTransform.valid())
    {
        osg::StateSet *pStateSet = m_pSunTransform->asGroup()->getChild(0)->asGroup()->getChild(0)->getStateSet();
        pStateSet->removeAttribute(osg::StateAttribute::PROGRAM);
    }
    m_pSunTransform = NULL;

    m_bInitialized = false;
}


void SkyDome::initialize(void)
{
    if(m_bInitialized)  return;

    m_pEphemerisProvider = new DefaultEphemerisProvider();

    // intialize the default settings:
    m_pLight = new osg::Light(0);
    m_pLight->setAmbient(osg::Vec4(m_dblAmbientBrightness, m_dblAmbientBrightness, m_dblAmbientBrightness, 1.0f));
    m_pLight->setDiffuse(osg::Vec4(m_dblDiffuseBrightness, m_dblDiffuseBrightness, m_dblDiffuseBrightness, 1.0f));
    m_pLight->setSpecular(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    m_bStarsVisible = true;
    m_bMoonVisible  = true;

    // set up the uniform that conveys the normalized light position in world space
    m_pLightPosUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "atmos_v3LightPos");

    // set up the astronomical parameters:
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    m_dblInnerRadius = pEllipsoidModel->getRadiusPolar();
    m_dblOuterRadius = pEllipsoidModel->getRadiusPolar() * 1.015f;
    m_dblSunDistance = m_dblInnerRadius * 6000.0f;

    // make the sky elements (don't change the order here)
    m_pAtmosphereNode = createAtmosphere();
    m_pAtmosphereNode->setName("Skydome Atmosphere");

    m_pSunTransform = new osg::MatrixTransform();
    m_pSunTransform->addChild(createSun());
    m_pSunTransform->setName("Skydome SunTransform");

    m_pMoonTransform = new osg::MatrixTransform();
    m_pMoonTransform->addChild(createMoon());
    m_pMoonTransform->setNodeMask(m_bMoonVisible ? ~0 : 0);
    m_pMoonTransform->setName("Skydome MoonTransform");

    m_pStarsTransform = new osg::MatrixTransform();
    m_pStarsTransform->addChild(createStars());
    m_pStarsTransform->setNodeMask(m_bStarsVisible ? ~0 : 0);
    m_pStarsTransform->setName("Skydome StarsTransform");

    addChild(m_pSunTransform.get());
    addChild(m_pMoonTransform.get());
    addChild(m_pStarsTransform.get());
    addChild(m_pAtmosphereNode.get());

    setNodeMask(0xFF000000);

    m_strLightMode = "sky_light";
    m_bInitialized = true;
    setDateTime(m_DateTime);
}


void SkyDome::traverse(osg::NodeVisitor &nv)
{
    if(!m_bInitialized) return;

    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if(!pCullVisitor)
    {
        osg::Group::traverse(nv);
        return;
    }

    // If there's a custom projection matrix clamper installed, remove it temporarily.
    // We dont' want it mucking with our sky elements.
    osg::ref_ptr<osg::CullSettings::ClampProjectionMatrixCallback> pProjMatrixCallback = pCullVisitor->getClampProjectionMatrixCallback();
    pCullVisitor->setClampProjectionMatrixCallback(NULL);

    osg::Group::traverse(nv);

    // restore a custom clamper.
    if(pProjMatrixCallback.valid())
    {
        pCullVisitor->setClampProjectionMatrixCallback(pProjMatrixCallback.get());
    }

    const osg::RenderInfo &infoRender = pCullVisitor->getRenderInfo();
    osg::View *pView = const_cast<osg::View *>(infoRender.getView());
    if(m_strLightMode.compare("head_light") == 0)
    {
        m_pLight->setPosition(osg::Vec4(0.0f, 0.0f, 1.0f, 0.0f));
        m_pLight->setDirection(osg::Vec3(0.0f, 0.0f, -1.0f));
        m_pLight->setDiffuse(osg::Vec4(m_dblDiffuseBrightness, m_dblDiffuseBrightness, m_dblDiffuseBrightness, 1.0f));
        m_pLight->setAmbient(osg::Vec4(m_dblAmbientBrightness, m_dblAmbientBrightness, m_dblAmbientBrightness, 1.0f));
        pView->setLightingMode(osg::View::HEADLIGHT);
    }
    else if(m_strLightMode.compare("sky_light") == 0)
    {
        osg::Vec3d ptSunPos = m_pEphemerisProvider->getSunPosition(m_DateTime);
        ptSunPos.normalize();
        m_pLight->setPosition(osg::Vec4(ptSunPos, 0.0));
        m_pLight->setDirection(-ptSunPos);
        reCalcSkyLight(pCullVisitor->getEyePoint());
        pView->setLightingMode(osg::View::SKY_LIGHT);
    }
    pView->setLight(m_pLight.get());
}


void SkyDome::reCalcSkyLight(const osg::Vec3 &ptEyePos)
{
    if(!m_bInitialized)     return;
    if(!m_pLight.valid())   return;

    static double dblActualAmbient = 0.0;
    static double dblActualDiffuse = 0.0;
    double  dblAmbient    = dblActualAmbient;
    double  dblDiffuse    = dblActualDiffuse;
    bool    bStarsVisible = m_bStarsVisible;
    bool    bMoonVisible  = m_bMoonVisible;

    osg::Vec3d  vecEyeDir = ptEyePos;
    const double dblLength = vecEyeDir.normalize();
    if(dblLength > m_dblOuterRadius)
    {
        // the camera is in the outer space, only diffuse, no ambient(ambient is very weak)
        dblDiffuse = m_dblDiffuseBrightness;
        dblAmbient = m_dblAmbientBrightness * 0.2;
    }
    else
    {
        // now the camera is in the atmosphere
        // so the intensity of light is decided by the time, it is day time? or night time?

        const osg::Vec4 &ptSunPos = m_pLight->getPosition();
        osg::Vec3 vecSunDir(ptSunPos.x(), ptSunPos.y(), ptSunPos.z());
        vecSunDir.normalize();
        const double dblRatio = vecSunDir * vecEyeDir;
        if(dblRatio > 0.0)
        {
            // it is day time, so, we need both diffuse and ambient
            dblDiffuse = m_dblDiffuseBrightness;                // the diffuse is used immediately
            double dblRatio1 = (m_dblOuterRadius - dblLength) / (m_dblOuterRadius - m_dblInnerRadius);
            dblRatio1 = dblRatio1 * 0.9 + 0.1;
            dblAmbient = m_dblAmbientBrightness * sqrt(fabs(dblRatio * dblRatio1));     // the ambient is withered by the ratio
            //if(dblRatio > 0.1)  bStarsVisible = false;
            //if(dblRatio > 0.2)  bMoonVisible  = false;
        }
        else
        {
            // it is night time, both ambient and diffuse is very weak
            dblDiffuse = m_dblDiffuseBrightness * 0.1;
            dblAmbient = m_dblAmbientBrightness * 0.1;
        }
    }

    m_pLight->setDiffuse(osg::Vec4(dblDiffuse, dblDiffuse, dblDiffuse, 1.0f));
    m_pLight->setAmbient(osg::Vec4(dblAmbient, dblAmbient, dblAmbient, 1.0f));
}


bool SkyDome::setLightMode(const std::string &strMode)
{
    if(!m_bInitialized) return false;

    if(strMode.compare("head_light") == 0)
    {
        m_strLightMode = strMode;
    }
    else if(strMode.compare("sky_light") == 0)
    {
        m_strLightMode = strMode;
    }
    else
    {
        return false;
    }
    return true;
}


void SkyDome::setAmbientBrightness(double dblValue)
{
    if(!m_bInitialized) return;

    m_dblAmbientBrightness = osg::clampBetween(dblValue, 0.0, 1.0);
    if(m_strLightMode.compare("head_light") == 0)
    {
        m_pLight->setAmbient(osg::Vec4(m_dblAmbientBrightness, m_dblAmbientBrightness, m_dblAmbientBrightness, 1.0f));
    }
}


void SkyDome::setDiffuseBrightness(double dblValue)
{
    if(!m_bInitialized) return;

    m_dblDiffuseBrightness = osg::clampBetween(dblValue, 0.0, 1.0);
    if(m_strLightMode.compare("head_light") == 0)
    {
        m_pLight->setDiffuse(osg::Vec4(m_dblDiffuseBrightness, m_dblDiffuseBrightness, m_dblDiffuseBrightness, 1.0f));
    }
}


void SkyDome::setSunPosition(const osg::Vec3d &pos)
{
    if(m_strLightMode.compare("sky_light") == 0 && m_pLight.valid())
    {
        m_pLight->setPosition(osg::Vec4(pos, 0));

        const osg::Vec3d vecDir = -pos;
        m_pLight->setDirection(vecDir);
    }
    if(m_pLightPosUniform.valid())
    {
        m_pLightPosUniform->set(pos / pos.length());
    }
    if(m_pSunTransform.valid())
    {
        m_pSunTransform->setMatrix(osg::Matrix::translate(pos * m_dblSunDistance));
    }
}


void SkyDome::setSunPosition(double lat_degrees, double long_degrees)
{
    double x, y, z;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(
        osg::RadiansToDegrees(lat_degrees),
        osg::RadiansToDegrees(long_degrees),
        0, 
        x, y, z);

    const osg::Vec3d up  = pEllipsoidModel->computeLocalUpVector(x, y, z);
    setSunPosition(up);
}


void SkyDome::setDateTime(const cmm::DateTime &dateTime)
{
    if(!m_bInitialized) return;

    m_DateTime = dateTime;

    if(!m_pEphemerisProvider.valid())
    {
        return;
    }

    osg::Vec3d ptSunPos = m_pEphemerisProvider->getSunPosition(m_DateTime);
    setSunPosition(ptSunPos);

    const osg::Vec3d ptMoonthPos = m_pEphemerisProvider->getMoonPosition(m_DateTime);
    if(m_pMoonTransform.valid())
    {
        m_pMoonTransform->setMatrix(osg::Matrixd::translate(ptMoonthPos));
    }

    // position the stars:
    const double dblTimeRatio = dateTime.m_dblHours / 24.0; // 0..1
    const double dblRotZ      = -osg::PI + TWO_PI * dblTimeRatio;

    if(m_pStarsTransform.valid())
    {
        m_pStarsTransform->setMatrix(osg::Matrixd::rotate(-dblRotZ, 0.0, 0.0, 1.0));
    }
}


void SkyDome::setMoonVisible(bool bVisible)
{
    if(!m_bInitialized) return;

    m_bMoonVisible = bVisible;
    if(m_pMoonTransform.valid())
    {
        m_pMoonTransform->setNodeMask(bVisible ? ~0 : 0);
    }
}


void SkyDome::setStarsVisible(bool bVisible)
{
    if(!m_bInitialized) return;

    m_bStarsVisible = bVisible;
    if(m_pStarsTransform.valid())
    {
        m_pStarsTransform->setNodeMask(bVisible ? ~0 : 0);
    }
}


void SkyDome::setSunVisible(bool bVisible)
{
    if(!m_bInitialized) return;

    m_bSunVisible = bVisible;
    if(m_pSunTransform.valid())
    {
        m_pSunTransform->setNodeMask(bVisible ? ~0 : 0);
    }
}


void SkyDome::setAtmosphereVisible(bool bVisible)
{
    if(!m_bInitialized) return;

    m_bAmtosphereVisiible = bVisible;
    if(m_pAtmosphereNode.valid())
    {
        m_pAtmosphereNode->setNodeMask(bVisible ? ~0 : 0);
    }
}


osg::Node *SkyDome::createAtmosphere(void)
{
    // create some skeleton geometry to shade:
    osg::ref_ptr<osg::Geometry> pGeometry = makeEllipsoidGeometry(osg::EllipsoidModel::instance(), m_dblOuterRadius);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode();
    pGeode->addDrawable(pGeometry);

    osg::StateSet *pStateSet = pGeode->getOrCreateStateSet();

    // configure the state set:
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pStateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON);
    //set->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    pStateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no depth write
    pStateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
    pStateSet->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ONE), osg::StateAttribute::ON);

    {
        // next, create and add the shaders:
        osg::ref_ptr<osg::Program> pProgram = new osg::Program();
        osg::ref_ptr<osg::Shader> pVS = new osg::Shader(osg::Shader::VERTEX, s_atmosphereVertexSource);
        pProgram->addShader(pVS);

        osg::ref_ptr<osg::Shader> pFS = new osg::Shader(osg::Shader::FRAGMENT, s_atmosphereFragmentSource);
        pProgram->addShader(pFS);

        pStateSet->setAttributeAndModes(pProgram, osg::StateAttribute::ON);

        // apply the uniforms:
        float r_wl = ::powf(.65f, 4.0f);
        float g_wl = ::powf(.57f, 4.0f);
        float b_wl = ::powf(.475f, 4.0f);
        osg::Vec3 RGB_wl(1.0f/r_wl, 1.0f/g_wl, 1.0f/b_wl);
        float Kr = 0.0025f;
        float Kr4PI = Kr * 4.0f * osg::PI;
        float Km = 0.0015f;
        float Km4PI = Km * 4.0f * osg::PI;
        float ESun = 15.0f;
        float MPhase = -.095f;
        float RayleighScaleDepth = 0.25f;
        int   Samples = 2;
        float Weather = 1.0f;

        float Scale = float(1.0 / (m_dblOuterRadius - m_dblInnerRadius));

        // TODO: replace this with a UBO.

        pStateSet->getOrCreateUniform("atmos_v3InvWavelength", osg::Uniform::FLOAT_VEC3)->set(RGB_wl);
        pStateSet->getOrCreateUniform("atmos_fInnerRadius",    osg::Uniform::FLOAT)->set(float(m_dblInnerRadius));
        pStateSet->getOrCreateUniform("atmos_fInnerRadius2",   osg::Uniform::FLOAT)->set(float(m_dblInnerRadius * m_dblInnerRadius));
        pStateSet->getOrCreateUniform("atmos_fOuterRadius",    osg::Uniform::FLOAT)->set(float(m_dblOuterRadius));
        pStateSet->getOrCreateUniform("atmos_fOuterRadius2",   osg::Uniform::FLOAT)->set(float(m_dblOuterRadius * m_dblOuterRadius));
        pStateSet->getOrCreateUniform("atmos_fKrESun",         osg::Uniform::FLOAT)->set(Kr * ESun);
        pStateSet->getOrCreateUniform("atmos_fKmESun",         osg::Uniform::FLOAT)->set(Km * ESun);
        pStateSet->getOrCreateUniform("atmos_fKr4PI",          osg::Uniform::FLOAT)->set(Kr4PI);
        pStateSet->getOrCreateUniform("atmos_fKm4PI",          osg::Uniform::FLOAT)->set(Km4PI);
        pStateSet->getOrCreateUniform("atmos_fScale",          osg::Uniform::FLOAT)->set(Scale);
        pStateSet->getOrCreateUniform("atmos_fScaleDepth",     osg::Uniform::FLOAT)->set(RayleighScaleDepth);
        pStateSet->getOrCreateUniform("atmos_fScaleOverScaleDepth", osg::Uniform::FLOAT)->set(Scale / RayleighScaleDepth);
        pStateSet->getOrCreateUniform("atmos_g",               osg::Uniform::FLOAT)->set(MPhase);
        pStateSet->getOrCreateUniform("atmos_g2",              osg::Uniform::FLOAT)->set(MPhase * MPhase);
        pStateSet->getOrCreateUniform("atmos_nSamples",        osg::Uniform::INT)->set(Samples);
        pStateSet->getOrCreateUniform("atmos_fSamples",        osg::Uniform::FLOAT)->set((float)Samples);
        pStateSet->getOrCreateUniform("atmos_fWeather",        osg::Uniform::FLOAT)->set(Weather);
    }

    // A nested camera isolates the projection matrix calculations so the node won't 
    // affect the clip planes in the rest of the scene.
    osg::ref_ptr<osg::Camera> pCamera = new osg::Camera();
    pCamera->getOrCreateStateSet()->setRenderBinDetails(BIN_ATMOSPHERE, "RenderBin");
    pCamera->setRenderOrder(osg::Camera::NESTED_RENDER);
    pCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    pCamera->addChild(pGeode);

    return pCamera.release();
}


osg::Node *SkyDome::createSun(void)
{
    osg::Billboard *pBillboard = new osg::Billboard();
    pBillboard->setMode(osg::Billboard::POINT_ROT_EYE);
    pBillboard->setNormal(osg::Vec3(0, 0, 1));

    const float fSunRadius = m_dblInnerRadius * 100.0f;
    pBillboard->addDrawable(makeDiscGeometry(fSunRadius * 80.0f)); 

    osg::StateSet *pStateSet = pBillboard->getOrCreateStateSet();
    pStateSet->setMode(GL_BLEND, 1);

    pStateSet->getOrCreateUniform("sunAlpha", osg::Uniform::FLOAT)->set(1.0f);

    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    pStateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), osg::StateAttribute::ON);

    {
        osg::ref_ptr<osg::Program> pProgram = new osg::Program();
        osg::ref_ptr<osg::Shader> pVS = new osg::Shader(osg::Shader::VERTEX, s_sunVertexSource);
        pProgram->addShader(pVS);
        osg::ref_ptr<osg::Shader> pFS = new osg::Shader(osg::Shader::FRAGMENT, s_sunFragmentSource);
        pProgram->addShader(pFS);
        pStateSet->setAttributeAndModes(pProgram, osg::StateAttribute::ON);
    }

    // A nested camera isolates the projection matrix calculations so the node won't 
    // affect the clip planes in the rest of the scene.
    osg::ref_ptr<osg::Camera> pCamera = new osg::Camera();
    pCamera->getOrCreateStateSet()->setRenderBinDetails(BIN_SUN, "RenderBin");
    pCamera->setRenderOrder(osg::Camera::NESTED_RENDER);
    pCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    pCamera->addChild(pBillboard);

    return pCamera.release();
}


osg::Node *SkyDome::createMoon(void)
{
    osg::ref_ptr<osg::EllipsoidModel> pEllipsoidModel = new osg::EllipsoidModel(1738140.0, 1735970.0);

    osg::ref_ptr<osg::Geometry> pGeometry = makeEllipsoidGeometry(pEllipsoidModel.get(), pEllipsoidModel->getRadiusEquator(), true);
    osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(m_strMoonFile);
    if(pImage.valid())
    {
        osg::ref_ptr<osg::Texture2D> pTexture = new osg::Texture2D(pImage.get());
        pTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        pTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        pTexture->setResizeNonPowerOfTwoHint(false);
        osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();
        pStateSet->setTextureAttributeAndModes(0, pTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    osg::StateSet *pStateSet = pGeode->getOrCreateStateSet();
    pStateSet->setAttributeAndModes(new osg::Program(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    pStateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON);
    pStateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), osg::StateAttribute::ON);
    pStateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);

    pGeode->addDrawable(pGeometry.get());

    osg::ref_ptr<osg::Camera> pCamera = new osg::Camera();
    pCamera->getOrCreateStateSet()->setRenderBinDetails(BIN_MOON, "RenderBin");
    pCamera->setRenderOrder(osg::Camera::NESTED_RENDER);
    pCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    pCamera->addChild(pGeode.get());

    return pCamera.release();
}


osg::Node *SkyDome::createStars(void)
{
    m_dblStarRadius = 20000.0 * (m_dblSunDistance > 0.0 ? m_dblSunDistance : m_dblOuterRadius);

    double dblMinMag = DBL_MAX, dblMaxMag = DBL_MIN;

    const unsigned nStarsCount = sizeof(gs_defaultStarData) / sizeof(StarData);

    std::vector<double>     vecMagnitudes;
    vecMagnitudes.reserve(nStarsCount);
    osg::ref_ptr<osg::Vec3Array> pVtxArray = new osg::Vec3Array();
    pVtxArray->reserve(nStarsCount);
    for(unsigned n = 0u; n < nStarsCount; n++)
    {
        const StarData &star = gs_defaultStarData[n];
        if(star.m_dblMagnitude < m_dblStarDetailLevel)
        {
            continue;
        }

        const osg::Vec3d point = getPositionFromRADecl(star.m_dblRightAscension, star.m_dblDeclination, m_dblStarRadius);
        pVtxArray->push_back(point);

        vecMagnitudes.push_back(star.m_dblMagnitude);

        if(dblMinMag > star.m_dblMagnitude)    dblMinMag = star.m_dblMagnitude;
        if(dblMaxMag < star.m_dblMagnitude)    dblMaxMag = star.m_dblMagnitude;
    }

    osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array();
    pColorArray->reserve(pVtxArray->size());
    const double dblMagDistance = dblMaxMag - dblMinMag;
    for(std::vector<double>::const_iterator itorMag = vecMagnitudes.begin(); itorMag != vecMagnitudes.end(); ++itorMag)
    {
        const double dblMag = *itorMag;
        const double dblColor = (dblMag - dblMinMag) / dblMagDistance;
        pColorArray->push_back(osg::Vec4(dblColor, dblColor, dblColor, 1.0f));
    }

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setUseVertexBufferObjects(true);

    pGeometry->setVertexArray(pVtxArray.get());
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, pVtxArray->size()));

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pGeometry);
    osg::StateSet *pStateSet = pGeode->getOrCreateStateSet();

    {
        pStateSet->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
        pStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);

        osg::ref_ptr<osg::Program> pProgram = new osg::Program;
        pProgram->addShader(new osg::Shader(osg::Shader::VERTEX, s_starVertexSource));
        pProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, s_starFragmentSource));
        pStateSet->setAttributeAndModes(pProgram.get(), osg::StateAttribute::ON);
    }

    pStateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), osg::StateAttribute::ON);
    pStateSet->setMode(GL_BLEND, 1);


    // A separate camera isolates the projection matrix calculations.
    osg::ref_ptr<osg::Camera> pCamera = new osg::Camera();
    pCamera->getOrCreateStateSet()->setRenderBinDetails(BIN_STARS, "RenderBin");
    pCamera->setRenderOrder(osg::Camera::NESTED_RENDER);
    pCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    pCamera->addChild(pGeode);

    return pCamera.release();
}


