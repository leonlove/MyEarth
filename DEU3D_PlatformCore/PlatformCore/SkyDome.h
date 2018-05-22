#ifndef SKY_DOME_H_7E308EFC_DFF1_4C5E_9205_D02CDAD9595A_INCLUDE
#define SKY_DOME_H_7E308EFC_DFF1_4C5E_9205_D02CDAD9595A_INCLUDE

#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/Uniform>
#include <osg/Group>
#include <osg/View>
#include "ISkyDome.h"

#pragma warning (disable : 4250)

class EphemerisProvider : public osg::Referenced
{
public:
    /**
    * Gets the moon position in geocentric coordinates at the given time
    */
    virtual osg::Vec3d getMoonPosition(const cmm::DateTime &dateTime) = 0;

    /**
    * Gets the sun position in geocentric coordinates at the given time
    */
    virtual osg::Vec3d getSunPosition(const cmm::DateTime &dateTime) = 0;
};


class SkyDome : public ISkyDome, public osg::Group
{
public:
    explicit SkyDome(void);
    virtual ~SkyDome(void);

public: // Methods from ISkyDome
    virtual void getDateTime(cmm::DateTime &dateTime) const     {   dateTime = m_DateTime;  }
    virtual void setDateTime(const cmm::DateTime &dateTime);

    /** The minimum brightness for non-sunlit areas. */
    virtual void   setAmbientBrightness(double dblValue);
    virtual void   setDiffuseBrightness(double dblValue);
    virtual double getAmbientBrightness(void) const             {   return m_dblAmbientBrightness;  }
    virtual double getDiffuseBrightness(void) const             {   return m_dblDiffuseBrightness;  }

    /** Whether the moon is visible */
    virtual void setMoonVisible(bool bVisible);
    virtual bool getMoonVisible(void) const                     {   return m_bMoonVisible;      }

    /** Whether the stars are visible */
    virtual void setStarsVisible(bool bVisible);
    virtual bool getStarsVisible(void) const                    {   return m_bStarsVisible;     }

    virtual void setSunVisible(bool bVisible);
    virtual bool getSunVisible(void) const                      {   return m_bSunVisible;     }

    virtual void setAtmosphereVisible(bool bVisible);
    virtual bool getAtmosphereVisible(void) const               {   return m_bAmtosphereVisiible;     }

    virtual bool                     setLightMode(const std::string &strMode);
    virtual const std::string       &getLightMode(void) const   {   return m_strLightMode;      }

public:
    void initialize(void);
    void unInitialize(void);

    osg::Light *getLight(void)                                  {   return m_pLight.get();      }
    const osg::Light *getLight(void) const                      {   return m_pLight.get();      }

protected:  // override virtual functions from osg::Group
    virtual void traverse(osg::NodeVisitor &nv);
    virtual osg::BoundingSphere computeBound(void) const        {   return osg::BoundingSphere();   }

protected:
    /** Sets the sun's position as a latitude and longitude. */
    void setSunPosition( double lat_degrees, double lon_degrees);
    void setSunPosition(const osg::Vec3d &pos);

    osg::Node *createAtmosphere(void);
    osg::Node *createSun(void);
    osg::Node *createMoon(void);
    osg::Node *createStars(void);

    void    reCalcSkyLight(const osg::Vec3 &ptEyePos);

protected:
    bool            m_bInitialized;
    std::string     m_strLightMode;

    cmm::DateTime   m_DateTime;

    double      m_dblInnerRadius;
    double      m_dblOuterRadius;
    double      m_dblSunDistance;
    double      m_dblStarRadius;
    double      m_dblStarDetailLevel;

    double      m_dblAmbientBrightness;
    double      m_dblDiffuseBrightness;

    std::string m_strMoonFile;

    osg::ref_ptr<osg::Light>            m_pLight;
    osg::ref_ptr<osg::Uniform>          m_pLightPosUniform;

    bool                                m_bStarsVisible;
    bool                                m_bMoonVisible;
    bool                                m_bSunVisible;
    bool                                m_bAmtosphereVisiible;

    osg::ref_ptr<osg::Node>             m_pAtmosphereNode;
    osg::ref_ptr<osg::MatrixTransform>  m_pSunTransform;
    osg::ref_ptr<osg::MatrixTransform>  m_pMoonTransform;
    osg::ref_ptr<osg::MatrixTransform>  m_pStarsTransform;

    osg::ref_ptr<osg::Uniform>          m_uniformStarAlpha;
    osg::ref_ptr<osg::Uniform>          m_uniformStarPointSize;

    osg::ref_ptr<EphemerisProvider>             m_pEphemerisProvider;
};


#endif
