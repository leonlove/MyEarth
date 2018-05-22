#ifndef PipeConnector_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE
#define PipeConnector_BUILDER_H_82E5052E_194E_46B5_BDF6_3AC7ECEE885D_INCLUDE

#include <OpenSP/Ref.h>
#include <osg/Node>
#include <osg/Vec3d>
#include <osg/Drawable>
#include <osg/Geode>

class PipeConnectorBuilder : public OpenSP::Ref
{
public:
    explicit PipeConnectorBuilder(void);
    virtual ~PipeConnectorBuilder(void);

public:
    void        setType(const std::string &type){m_strType = type;}
    void        setCorner(const osg::Vec3d &ptCorner);
    void        addJoint(const osg::Vec3d &ptPort, double dblRadius);
    void        clearJoints(void);
    void        setColor(const osg::Vec4 &color);
    osg::Node  *buildPipeConnector(void);

    void        setPortHint(double dblHint);
    double      getPortHint(void) const                 {   return m_dblPortHint;   }
    void        setElbowHint(double dblHint);
    double      getElbowHint(void) const                {   return m_dblElbowHint;  }
    void        setCornerRadiusRatio(double dblRatio)   {   m_dblCornerRadiusRatio = dblRatio;  }
    double      getCornerRadiusRatio(void) const        {   return m_dblCornerRadiusRatio;      }

protected:
    struct Port
    {
        osg::Vec3d      m_ptPosition;
        double          m_dblRadius;
    };
    struct CircleFace
    {
        osg::Vec3d      m_ptCenter;
        osg::Vec3d      m_vecNormal;
        double          m_dblRadius;
    };

    osg::Geode    *createElbow(const Port &port0, const Port &port1) const;
    osg::Drawable *createElbowSeg(const CircleFace &circle0, const CircleFace &circle1) const;
    osg::Geode    *createHoop(const CircleFace &circle, double dbRadiusRatio = 1.1, double lengthRatio = 0.5) const;
    osg::Drawable *createCircleFace(const CircleFace &circle) const;
    osg::Node     *createEndBlocker(const Port &port)const;
    osg::Geode    *createPipeHat(const Port &port)const;
    osg::Node     *createSpecial(void);
protected:
    osg::ref_ptr<osg::Vec4Array>    m_pColorArray;
    osg::Vec3d          m_ptCorner;
    std::vector<Port>   m_vecJoints;
    double              m_dblPortHint;
    double              m_dblPortHintApply;
    double              m_dblElbowHint;
    double              m_dblElbowHintApply;
    double              m_dblCornerRadiusRatio;
    std::string         m_strType;
};

#endif
