#ifndef IHOLE_H_
#define IHOLE_H_

#include <OpenSP/Ref.h>
#include <Common\deuMath.h>
#include <common/DEUBson.h>

namespace param
{

class IHole : public OpenSP::Ref
{
public:
    virtual const   std::string &getHoleType(void) const                                = 0;

    virtual void    setOnTopFace(bool isOn)                                             = 0;
    virtual bool    isOnTopFace(void) const                                             = 0;

    virtual void    setCircle(const cmm::math::Point2d &center, double radius)          = 0;
    virtual bool    getCircle(cmm::math::Point2d &center, double &radius) const         = 0;

    virtual void    setRectangle(const cmm::math::Point2d &center, double width, double height, double azimuthAngle)    = 0;
    virtual bool    getRectangle(cmm::math::Point2d &center, double &width, double &height, double &azimuthAngle) const  = 0;

    virtual void    addPolygonVertex(const cmm::math::Point2d &v)                       = 0;
    virtual void    clearPolygonVertex(void)                                            = 0;
    virtual bool    getPolygonVertex(size_t i, cmm::math::Point2d &v) const             = 0;
    virtual bool    getAllPolygonVertex(std::vector<cmm::math::Point2d> &vecVeterx) const   = 0;
    virtual unsigned getPolygonVerticesCount(void) const                                = 0;

    virtual void    toBson(bson::bsonDocument &bsonDoc) const                           = 0;
    virtual bool    fromBson(bson::bsonDocument &bsonDoc)                               = 0;
};

}
#endif