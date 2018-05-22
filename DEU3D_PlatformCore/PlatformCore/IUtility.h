#ifndef IUTILITY_H__
#define IUTILITY_H__ 1

#include <Common\Common.h>
#include <Common/deuMath.h>
#include "NavigationParam.h"
#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include <string>
#include "IAnimationModel.h"
#include "INavigationPath.h"
#include <ParameterSys\Parameter.h>
#include <common\deuMath.h>

class IUtility : virtual public OpenSP::Ref
{
public:
    virtual IAnimationModel                *createAnimationModel(void) = 0;

    virtual INavigationKeyframe            *createNavigationKeyframe(void) = 0;
    virtual INavigationPath                *createNavigationPath(void) = 0;
    virtual INavigationPath                *createNavigationPathByCoords(const std::vector<cmm::math::Point3d> &vecCoords, bool bFixedAzimuth, double dblAzimuth, bool bFixedPitch, double dblPitch) = 0;
    virtual INavigationPathContainer       *createNavigationPathContainer(void) = 0;
    virtual CameraPose                      createCameraPoseByRect(const cmm::math::Box2d &box) = 0;
    virtual bool                            getViewArea(unsigned nIndex, cmm::math::Box2d &box) = 0;

    virtual cmm::math::Polygon2             enlargePolygon(cmm::math::Polygon2 &polygon, double dblDist) = 0;

    virtual cmm::math::Polygon2             generalCircle(cmm::math::Point2d &center, double dblRadius, double dblHits) = 0;
    virtual cmm::math::Polygon2             generalBufferArea(cmm::math::Point2d &point1, cmm::math::Point2d &point2, double dblDistance, double dblHits = 0.1) = 0;
    virtual cmm::math::Polygon2             generalCubeConnector(cmm::math::Point2d &point1, cmm::math::Point2d &point2, cmm::math::Point2d &point3, cmm::math::Point2d &point4, double dblDistance, double dblHits = 0.1) = 0;

    virtual bool shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin) = 0;
    virtual bool moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist) = 0;

    virtual void convRect2Polygon(const cmm::math::Point2d &center, double dblWidth, double dblHeight, double dblAzimuth, cmm::math::Polygon2 &polygon) = 0;
    virtual std::string getVersion(void) const = 0;
};

#endif