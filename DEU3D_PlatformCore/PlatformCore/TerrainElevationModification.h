#ifndef TERRAIN_MODIFICATION_H_D4F6AA9C_34B7_4D0F_9A96_789460FF80C1_INCLUDE
#define TERRAIN_MODIFICATION_H_D4F6AA9C_34B7_4D0F_9A96_789460FF80C1_INCLUDE

#include "ITerrainElevationModification.h"
#include "TerrainModification.h"

namespace cmm{
    namespace image{
        class IDEUImage;
    }
}

class TerrainElevationModification : public ITerrainElevationModification, virtual public TerrainModification
{
public:
    explicit TerrainElevationModification(const std::string &strType, ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainElevationModification(void);

public:
    virtual void    setElevation(double dblElevation);
    virtual double  getElevation(void) const;
    virtual void    setSmoothInterval(double dblSmooth);
    virtual double  getSmoothInterval(void) const;

    virtual bool    modifyTerrainTile(osg::Node *pTerrainTileNode) const;

protected:
    virtual bool    shouldBeModified(const cmm::math::Polygon2 &polygonTile, const cmm::math::Box2d &bbTile) const;

protected:
    double  calcDistanceOnEarth(const cmm::math::Polygon2 &polygon, const cmm::math::Point2d &ptTest) const;
    double  calcDistanceOnEarth(const cmm::math::Point2d &point0, const cmm::math::Point2d &point1) const;
    void    fixTile(cmm::image::IDEUImage *pTileDEMImage) const;

protected:
    double          m_dblElevation;
    double          m_dblSmoothInterval;
};


#endif

