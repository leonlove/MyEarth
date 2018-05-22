#ifndef HOLE_INFO_H_B2E31D0B_1353_4340_87AB_C948ADEEE1E5_INCLUDE
#define HOLE_INFO_H_B2E31D0B_1353_4340_87AB_C948ADEEE1E5_INCLUDE

#include <vector>
#include <osg/Vec3d>
#include <osg/Geometry>
#include <ParameterSys/IHole.h>

struct HoleInfo
{
    HoleInfo(void) : 
        m_bCircleHole(true), 
        m_ptCenter(0.0, 0.0, 0.0),
        m_dblRadius(0.0),
        m_nHint(3u)
    {
    }

    HoleInfo(const HoleInfo &info) :
        m_bCircleHole(info.m_bCircleHole),
        m_ptCenter(info.m_ptCenter),
        m_dblRadius(info.m_dblRadius),
        m_nHint(info.m_nHint),
        m_vecPolygon(info.m_vecPolygon)
    {
    }

    ~HoleInfo(void){}

    const HoleInfo &operator=(const HoleInfo &info)
    {
        if(this == &info)   return *this;
        m_bCircleHole = info.m_bCircleHole;
        m_ptCenter = info.m_ptCenter;
        m_dblRadius = info.m_dblRadius;
        m_nHint = info.m_nHint;
        m_vecPolygon = info.m_vecPolygon;
        return *this;
    }

    bool            m_bCircleHole;
    osg::Vec3d      m_ptCenter;
    double          m_dblRadius;
    unsigned        m_nHint;
    std::vector<osg::Vec3d>     m_vecPolygon;
};

typedef std::vector<HoleInfo> HoleInfoList;

osg::Geometry *createSheetWithHoles(const std::vector<osg::Vec3d> &vecVertices, const HoleInfoList &listHoleInfo);
osg::Geometry *createCircleWithHoles(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint, const HoleInfoList &listHoleInfo);
void Hole2Vertices(const HoleInfo &hole, std::vector<osg::Vec3d> &vecVertices);
void genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, unsigned nHint, bool bClockWise, bool bClosed, std::vector<osg::Vec3d> &vecVertices);

#endif