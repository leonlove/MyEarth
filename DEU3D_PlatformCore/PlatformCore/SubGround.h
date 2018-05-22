#ifndef SUB_GROUND_H_E742EC02_72B9_401F_9A2B_67E179A17D66_INCLUDE
#define SUB_GROUND_H_E742EC02_72B9_401F_9A2B_67E179A17D66_INCLUDE

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Plane>
#include <osgUtil/Radial.h>
#include <osg/Texture2D>
#include <common/Common.h>

class SubGround : public osg::Group
{
public:
    explicit SubGround(void);
    virtual ~SubGround(void);

public:
    bool        initialize(osg::Node *pPlanetNode);

    void        setTerrainOpacity(double dblTransparency);
    double      getTerrainOpacity(void) const;

    void        setGroundColor(const cmm::FloatColor &color);
    const       cmm::FloatColor &getGroundColor(void) const  {   return m_clrGroundColor;   }

    void        setSubGroundDepth(double dblDepth)  {   m_dblSubGroundDepth = dblDepth; }
    double      getSubGroundDepth(void) const       {   return m_dblSubGroundDepth; }

protected:
    void        applyTerrainTransparency(double dblTransparency, double dblEyeHeight = FLT_MAX);
    bool        hitPlanetScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest) const;
    osg::Geometry *createBoardGeom(void);

protected:
    virtual void traverse(osg::NodeVisitor &nv);
    virtual osg::BoundingSphere computeBound(void) const;

protected:
    osg::ref_ptr<osg::MatrixTransform>  m_pSubGroundTrans;
    osg::ref_ptr<osg::Geode>            m_pSubGroundGeode;
    osg::ref_ptr<osg::Geometry>         m_pBoardGeometry;
    osg::ref_ptr<osg::Texture2D>        m_pSubGroundTexture;
    osg::ref_ptr<osg::Image>            m_pSubGroundImage;

    osg::ref_ptr<osg::Node>             m_pPlanetNode;
    double                              m_dblSubGroundDepth;
    unsigned                            m_nUpdateSpeed;

    osg::BoundingSphere                 m_bsBoundingSphere;

    double                              m_dblTerrainOpacity;
    cmm::FloatColor                     m_clrGroundColor;
};

#endif
