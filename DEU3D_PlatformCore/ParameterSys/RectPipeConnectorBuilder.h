#ifndef RECT_PIPE_CONNECTOR_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE
#define RECT_PIPE_CONNECTOR_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE

#include <OpenSP/Ref.h>
#include <osg/Node>
#include <osg/Vec3d>
#include <osg/Drawable>
#include <osg/Geode>

class RectPipeConnectorBuilder : public OpenSP::Ref
{
public:
    explicit RectPipeConnectorBuilder(void);
    virtual ~RectPipeConnectorBuilder(void);

public:
    osg::Node  *buildPipeConnector(void);

    void        setType(const std::string &strType);
    void        setCorner(const osg::Vec3d &ptCorner);
    void        setColor(const osg::Vec4 &color);

    void        setElbowHint(double dblHint);
    double      getElbowHint(void) const                {   return m_dblElbowHint;                  }
    void        setCornerRadiusRatio(double dblRatio)   {   m_dblCornerRadiusRatio = dblRatio;      }
    double      getCornerRadiusRatio(void) const        {   return m_dblCornerRadiusRatio;          }

    void        addPort(const osg::Vec3d &ptPosition, double dblWidth, double dblHeight);
    void        clearJoints(void);

protected:
    struct Port
    {
        osg::Vec3d      m_ptPosition;
        double          m_dblWidth;
        double          m_dblHeight;
    };

    osg::Geode    *createElbow(const Port &port0, const Port &port1, osg::Vec3d &vecLastWidthDir) const;

    struct RectFace
    {
        osg::Vec3d      m_ptCenter;
        osg::Vec3d      m_vecNormal;
        double          m_dblWidth;
        double          m_dblHeight;
    };

    osg::Drawable *createElbowSeg(const RectFace &rect0, const RectFace &rect1, osg::Vec3d &vecWidthDir = osg::Vec3d()) const;
    osg::Geode    *createHoop(const RectFace &rect, double dblLength, const osg::Vec3d &vecRecommendWidthDir = osg::Vec3d()) const;
    osg::Drawable *createRectFace(const RectFace &rect, const osg::Vec3d &vecRecommendWidthDir) const;

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
