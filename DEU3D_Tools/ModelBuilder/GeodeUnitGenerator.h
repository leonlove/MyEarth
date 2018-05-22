#ifndef GEODE_UNIT_GENERATOR_H_597B29DB_EF5E_4D40_A4E6_4E2DB6AFBF49_INCLUDE
#define GEODE_UNIT_GENERATOR_H_597B29DB_EF5E_4D40_A4E6_4E2DB6AFBF49_INCLUDE

#include <osg/NodeVisitor>
#include <osg/Geode>

class GeodeUnitGenerator
{
public:
    explicit GeodeUnitGenerator(void);
    virtual ~GeodeUnitGenerator(void);

public:
    double getBubbleRadius(void) const                      {   return m_dblBubbleRadius;       }
    void   setBubbleRadius(double dbl)                      {   m_dblBubbleRadius = dbl;        }
    const  osg::Vec3dArray *findBubbleCenters(osg::Geode& geode);

protected:
    void traverseBoundingBox_X(void) const;
    void traverseBoundingBox_Y(double x) const;
    void traverseBoundingBox_Z(double x, double y) const;
    bool validBubble(const osg::BoundingBoxd &box) const;

protected:
    osg::ref_ptr<osg::Vec3dArray>   m_pBubbleCenters;
    osg::ref_ptr<osg::Vec3dArray>   m_pGeodeVertices;
    osg::BoundingBoxd               m_GeodeBoundingBox;
    double                          m_dblBubbleRadius;
};

#endif
