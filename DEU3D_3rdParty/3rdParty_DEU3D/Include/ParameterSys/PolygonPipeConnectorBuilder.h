#ifndef POLYGON_PIPE_CONNECTOR_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE
#define POLYGON_PIPE_CONNECTOR_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE

#include <OpenSP/Ref.h>
#include <osg/Node>
#include <osg/Vec3d>
#include <osg/Drawable>
#include <osg/Geode>
#include <common/deuMath.h>

class PolygonPipeConnectorBuilder : public OpenSP::Ref
{
public:
    explicit PolygonPipeConnectorBuilder(void);
    virtual ~PolygonPipeConnectorBuilder(void);

public:
    osg::Node  *buildPipeConnector(void);

    void        setType(const std::string &strType);
    void        setCorner(const osg::Vec3d &ptCorner);
    void        setColor(const osg::Vec4 &color);

    void        setElbowHint(double dblHint);
    double      getElbowHint(void) const                {   return m_dblElbowHint;                  }
    void        setCornerRadiusRatio(double dblRatio)   {   m_dblCornerRadiusRatio = dblRatio;      }
    double      getCornerRadiusRatio(void) const        {   return m_dblCornerRadiusRatio;          }

    void        addPort(const osg::Vec3d &ptPosition, const cmm::math::Polygon2 &polygon);
    void        clearJoints(void);

protected:
    struct Port
    {
        osg::Vec3d              m_ptPosition;
        cmm::math::Polygon2     m_polygon;
    };

    osg::Geode    *createElbow(const Port &port0, const Port &port1) const;

    struct PolygonFace
    {
        osg::Vec3d      m_ptCenter;
        osg::Vec3d      m_vecNormal;
        std::vector<osg::Vec3d>     m_vecVertices;
    };

    osg::Drawable *createElbowSeg(const PolygonFace &face0, const PolygonFace &face1, osg::Vec3d &vecRightDir = osg::Vec3d()) const;
    osg::Geode    *createHoop(const PolygonFace &face, double dblLength, const osg::Vec3d &vecRecommendRightDir = osg::Vec3d()) const;
    osg::Drawable *createPolygonFace(const PolygonFace &face, const osg::Vec3d &vecRecommendRightDir) const;

    osg::Node     *createNormalConnector(void) const;
    osg::Node     *createEndBlocker(const Port &port) const;
    osg::Node     *createPipeHat(const Port &port) const;
    osg::Node     *createWeld(void) const;

protected:
    std::vector<Port>   m_vecJoints;
    osg::ref_ptr<osg::Vec4Array>    m_pColorArray;
    osg::Vec3d          m_ptCorner;
    double              m_dblElbowHint;
    double              m_dblElbowHintApply;
    double              m_dblCornerRadiusRatio;
    std::string         m_strType;

};



#endif
