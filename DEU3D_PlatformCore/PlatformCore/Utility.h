#ifndef UTILITY_H__
#define UTILITY_H__

#include "IUtility.h"
#include <osg/ref_ptr>
#include <IDProvider/ID.h>
#include <osg/Image>
#include <osgDB/WriteFile>
#include <osg/CoordinateSystemNode>
#include <osg/Program>

#include "IState.h"
#include <common/IDEUImage.h>
#include "DEUSceneViewer.h"
#include <ParameterSys\IParameter.h>
class DEUPlatformCore;

class Utility : public IUtility
{
public:
    explicit Utility(DEUPlatformCore *pDEUPlatformCore);
    virtual ~Utility(void);

protected:  // Methods from IUtility
    virtual IAnimationModel                *createAnimationModel(void);

    virtual INavigationKeyframe            *createNavigationKeyframe(void);
    virtual INavigationPath                *createNavigationPath(void);
    virtual INavigationPath                *createNavigationPathByCoords(const std::vector<cmm::math::Point3d> &vecCoords, bool bFixedAzimuth, double dblAzimuth, bool bFixedPitch, double dblPitch);
    virtual INavigationPathContainer       *createNavigationPathContainer(void);
    virtual CameraPose                      createCameraPoseByRect(const cmm::math::Box2d &box);
    virtual bool                            getViewArea(unsigned nIndex, cmm::math::Box2d &box);

    virtual cmm::math::Polygon2             enlargePolygon(cmm::math::Polygon2 &polygon, double dblDist);

    virtual cmm::math::Polygon2             generalCircle(cmm::math::Point2d &center, double dblRadius, double dblHits);
    virtual cmm::math::Polygon2             generalBufferArea(cmm::math::Point2d &point1, cmm::math::Point2d &point2, double dblDistance, double dblHits = 0.1);
    virtual cmm::math::Polygon2             generalCubeConnector(cmm::math::Point2d &point1, cmm::math::Point2d &point2, cmm::math::Point2d &point3, cmm::math::Point2d &point4, double dblDistance, double dblHits = 0.1);

    virtual bool                            shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin);
    virtual bool                            moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist);

    virtual void                            convRect2Polygon(const cmm::math::Point2d &center, double dblWidth, double dblHeight, double dblAzimuth, cmm::math::Polygon2 &polygon);

    virtual std::string getVersion(void) const;

protected:
    osg::ref_ptr<DEUPlatformCore>           m_pPlatformCore;
    std::map<ID, osg::Node*>                m_pTempModels;
};


class EarthLightModel
{
public:
    explicit EarthLightModel(void);
    virtual ~EarthLightModel(void){}

public:
    static bool bindEarthLightModel(osg::StateSet *pStateSet, char T = 0);
    static void unBindEarthLightModel(osg::StateSet *pStateSet);
    static void setSampleStatus(osg::StateSet *pStateSet, char T = 0);
    static void setSampleStatus(osg::StateSet *pStateSet, unsigned int nIndex, bool hasTexture);
    static void resetSampleStatus(osg::StateSet *pStateSet);
    static std::string getHasTextrueName(unsigned int nIndex);

protected:
    static void createEarthLightModel(void);

protected:
    static const std::string ms_strVertexShader;
    static const std::string ms_strFragmentShader;

    static const std::string ms_strTexture[8];

    static const std::string ms_strHasTexture[8];

    static osg::ref_ptr<osg::Program>  ms_pProgram;
    static OpenThreads::Mutex           ms_Mutex;
};


bool computeIntersection(const osg::Node *pIntersectTargetNode, const osg::Camera *pCamera, const osg::Vec2d &ptMousePos, osg::Vec3d &vIntersection, osg::Node **ppInterNode = NULL);
bool getScreenRadial(const osg::Camera *pCamera, const osg::Vec2d &ptPosNormalize, osgUtil::Radial3 &ray);
bool hitScene(const osg::Node *pHitTargetNode, const osg::Camera *pCamera, const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, osg::Node **ppHitNode = NULL);
osg::Vec3d calcWorldCoordByCameraPose(const CameraPose &pose);
osg::Vec3d calcCamDirByCameraPose(const CameraPose &pose);

#endif