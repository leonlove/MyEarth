#ifndef DISTANCE_PIXEL_COMPUTER_H_2C8F6EC6_0651_4D1C_BCB7_367DFC441A29_INCLUDE
#define DISTANCE_PIXEL_COMPUTER_H_2C8F6EC6_0651_4D1C_BCB7_367DFC441A29_INCLUDE

#include <osgUtil/Export>

#include <osg/Referenced>
#include <osg/Matrixd>
#include <osg/Vec3d>
#include <osg/Viewport>

namespace osgUtil
{

class OSGUTIL_EXPORT DistancePixelComputer : public osg::Referenced
{
public:
    explicit DistancePixelComputer(void);
    virtual ~DistancePixelComputer(void);

public:
    void    setEnviroment(double dblFovy, double dblWidth, double dblHeight);

public:
    double  calcPixelSizeByDistance(double dblObjectRadius, double dblDistance) const;
    double  calcDistanceByPixelSize(double dblObjectRadius, double dblPixelSize) const;

protected:
    osg::Matrixd        m_mtxProjection;
    osg::Matrixd        m_mtxWindow;
    osg::ref_ptr<osg::Viewport>     m_pViewport;
};

}

#endif
