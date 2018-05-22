#ifndef I_SYMBOL_OBJECT_H_43CB72B5_431C_435C_9750_0E409A72476F_INCLUDE
#define I_SYMBOL_OBJECT_H_43CB72B5_431C_435C_9750_0E409A72476F_INCLUDE

#include "Export.h"

#include <string>
#include <vector>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/Common.h>
#include <common/DEUBson.h>
#include <Common/deuMath.h>
#include "IHole.h"

namespace param
{

const std::string STATIC_DETAIL             = "Static_Detail";
const std::string IMAGE_DETAIL              = "Image_Detail";
const std::string POINT_DETAIL              = "Point_Detail";
const std::string LINE_DETAIL               = "Line_Detail";
const std::string FACE_DETAIL               = "Face_Detail";
const std::string CUBE_DETAIL               = "Cube_Detail";
const std::string CYLINDER_DETIAL           = "Cylinder_Detail";
const std::string SPHERE_DETAIL             = "Sphere_Detail";
const std::string PRISM_DETAIL              = "Prism_Detail";
const std::string PYRAMID_DETAIL            = "Pyramid_Detail";
const std::string PIPECONNECTOR_DETAIL      = "PipeConnector_Detail";
const std::string RECT_PIPECONNECTOR_DETAIL = "RectPipeConnector_Detail";
const std::string POLYGON_PIPECONNECTOR_DETAIL = "PolygonPipeConnector_Detail";
const std::string SECTOR_DETAIL             = "Sector_Detail";
const std::string HOLETYPE_CIRCLE           = "Circle";
const std::string HOLETYPE_RECTANGLE        = "Rectangle";
const std::string HOLETYPE_POLYGON          = "Polygon";
const std::string BUBBLETEXT_DETAIL         = "BubbleText_Detail";
const std::string POLYGON_DETAIL            = "Polygon_Detail";
const std::string ROUNDTABLE_DETAIL         = "RoundTable_Detail";
const std::string POINTCLOUD_DETAIL         = "PointCloud_Detail";

class IDetail : public OpenSP::Ref
{
public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc)       = 0;
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const   = 0;
    virtual const ID           &getID(void) const                           = 0;
    virtual const std::string  &getStyle(void) const                        = 0;
    virtual double              getBoundingSphereRadius(void)const          = 0;
    virtual bool                isImageStretched()                          = 0;
    virtual void                stretchImage(bool bStretch)                 = 0;
};

class IDynModelDetail : public virtual IDetail
{
public:
    virtual void                    setImageID(const ID &id)                     = 0;
    virtual const ID               &getImageID(void) const                       = 0;

    virtual void                    setDynModelColor(const cmm::FloatColor &clr) = 0;
    virtual const cmm::FloatColor  &getDynModelColor(void) const                 = 0;

    virtual IHole *                 generateHole(void)                   = 0;
    virtual void                    getHoleList(std::vector<IHole *> &holeList) const = 0;
};

class IPipeConnectorDetail : public virtual IDynModelDetail
{
public:
    virtual void                setType(const std::string &type = "normal") = 0;//可选值:normal、endblock(盲板)、pipehat(管帽)、weld(焊头)
    virtual std::string         getType()const = 0;
    virtual bool                addJoint(const cmm::math::Point3d &posTarget, double dblLength, double dblRadius) = 0;
    virtual bool                setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, double dblRadius) = 0;
    virtual bool                getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, double &dblRadius) const = 0;
    virtual unsigned            getJointsCount(void) const                = 0;
};

class IRectPipeConnectorDetail : public virtual IDynModelDetail
{
public:
    virtual void                setType(const std::string &type = "normal") = 0;//可选值:normal、endblock(盲板)、pipehat(管帽)、weld(焊头)
    virtual std::string         getType()const = 0;
    virtual bool                addJoint(const cmm::math::Point3d &posTarget, double dblLength, double dblWidth, double dblHeight) = 0;
    virtual bool                setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, double dblWidth, double dblHeight) = 0;
    virtual bool                getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, double &dblWidth, double &dblHeight) const = 0;
    virtual unsigned            getJointsCount(void) const                = 0;
};

class IPolygonPipeConnectorDetail : public virtual IDynModelDetail
{
public:
    virtual void                setType(const std::string &type = "normal") = 0;//可选值:normal、endblock(盲板)、pipehat(管帽)、weld(焊头)
    virtual std::string         getType()const = 0;
    virtual bool                addJoint(const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon) = 0;
    virtual bool                setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon) = 0;
    virtual bool                getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, cmm::math::Polygon2 &polygon) const = 0;
    virtual unsigned            getJointsCount(void) const                = 0;
};

class ICubeDetail : public virtual IDynModelDetail
{
public:
    virtual void        setCubeSize(double dblLength, double dblWidth, double dblHeight)            = 0;
    virtual void        getCubeSize(double &dblLength, double &dblWidth, double &dblHeight) const   = 0;

    virtual void        setTopVisible(bool bVisible) = 0;
    virtual bool        getTopVisible(void) const = 0;

    virtual void        setBottomVisible(bool bVisible) = 0;
    virtual bool        getBottomVisible(void) const = 0;
};

class IRoundTableDetail : public virtual IDynModelDetail
{
public:
    virtual void        setHeight(double dblHeight)         = 0;
    virtual double      getHeight(void) const               = 0;
    virtual void        setRadiusTop(double dblRadius)      = 0;
    virtual double      getRadiusTop(void) const            = 0;
    virtual void        setRadiusBottom(double dblRadius)   = 0;
    virtual double      getRadiusBottom(void) const         = 0;
    virtual void        setTopVisible(bool bVisible)        = 0;
    virtual bool        getTopVisible(void) const           = 0;
    virtual void        setBottomVisible(bool bVisible)     = 0;
    virtual bool        getBottomVisible(void) const        = 0;
};

class ICylinderDetail : public virtual IDynModelDetail
{
public:
    virtual void        setHeight(double dblHeight)         = 0;
    virtual double      getHeight(void) const               = 0;
    virtual void        setRadius(double dblRadius)         = 0;
    virtual double      getRadius(void) const               = 0;
    virtual void        setTopVisible(bool bVisible)        = 0;
    virtual bool        getTopVisible(void) const           = 0;
    virtual void        setBottomVisible(bool bVisible)     = 0;
    virtual bool        getBottomVisible(void) const        = 0;
};

class IPrismDetail : public virtual IDynModelDetail
{
public:
    virtual void                            setTopImageID(const ID &id)                 = 0;
    virtual const ID                       &getTopImageID() const                       = 0;

    virtual void                            setBottomImageID(const ID &id)              = 0;
    virtual const ID                       &getBottomImageID() const                    = 0;

    virtual void                            setHeight(double dblHeight)                 = 0;
    virtual double                          getHeight(void) const                       = 0;

    virtual void        setTopVisible(bool bVisible)        = 0;
    virtual bool        getTopVisible(void) const           = 0;
    virtual void        setBottomVisible(bool bVisible)     = 0;
    virtual bool        getBottomVisible(void) const        = 0;

    virtual void                            addVertex(const cmm::math::Point2d &vtx)    = 0;
    virtual void                            clearVertices(void)                         = 0;
    virtual std::vector<cmm::math::Point2d> &getVertices(void)                          = 0;
    virtual const std::vector<cmm::math::Point2d> &getVertices(void) const              = 0;
};

class ISphereDetail : public virtual IDynModelDetail
{
public:
    virtual void        setRadius(double dblRadius)     = 0;
    virtual double      getRadius(void) const           = 0;
};

class ISectorDetail : public virtual IDynModelDetail
{
public:
    virtual void                    setBorderWidth(double dblWidth)             = 0;
    virtual double                  getBorderWidth(void) const                  = 0;
    virtual void                    setBorderColor(const cmm::FloatColor &clr)  = 0;
    virtual const cmm::FloatColor  &getBorderColor(void) const                  = 0;
    virtual void                    setBeginAngle(double dblAngle)              = 0;
    virtual double                  getBeginAngle(void) const                   = 0;
    virtual void                    setEndAngle(double dblAngle)                = 0;
    virtual double                  getEndAngle(void) const                     = 0;
    virtual void                    setRadius1(double dblRadius)                = 0;
    virtual double                  getRadius1(void) const                      = 0;
    virtual void                    setRadius2(double dblRadius)                = 0;
    virtual double                  getRadius2(void) const                      = 0;
};

class IPyramidDetail : public virtual IDynModelDetail
{
public:
    virtual void        setBottomImageID(const ID &id) = 0;
    virtual const       ID &getBottomImageID(void) const = 0;

    virtual void        setBottomVisible(bool bVisible) = 0;
    virtual bool        getBottomVisible(void) const = 0;

    virtual void        addBottomVertex(const cmm::math::Point3d &vtx) = 0;
    virtual const       std::vector<cmm::math::Point3d> &getBottomVertices(void) const = 0;
    virtual unsigned    getBottomVerticesCount(void) const = 0;
};

class IStaticModelDetail : public virtual IDetail
{
public:
    virtual void        setModelID(const ID &id)        = 0;
    virtual const ID   &getModelID(void) const          = 0;
    virtual void        setAsOnGlobe(bool)              = 0;
    virtual bool        isOnGlobe()                     = 0;
};

class IDynPointDetail : public virtual IDetail
{
public:
    virtual void                    setPointColor(const cmm::FloatColor &clr)   = 0;
    virtual const cmm::FloatColor  &getPointColor() const                       = 0;

    virtual void                    setPointSize(double dblSize)                = 0;
    virtual double                  getPointSize() const                        = 0;
};

class IDynPointCloudDetail : public virtual IDetail
{
public:
	virtual void                    setPointColor(const cmm::FloatColor &clr)   = 0;
	virtual const cmm::FloatColor  &getPointColor() const                       = 0;

	virtual void                    setPointSize(double dblSize)                = 0;
	virtual double                  getPointSize() const                        = 0;
};

class IDynLineDetail : public virtual IDetail
{
public:
    virtual void                    setLineStyle(const std::string &strStyle)   = 0;
    virtual const std::string      &getLineStyle() const                        = 0;

    virtual void                    setLineWidth(double dblLineWidth)           = 0;
    virtual double                  getLineWidth(void) const                    = 0;

    virtual void                    setLineImage(const ID &id)                  = 0;
    virtual const ID               &getLineImage(void) const                    = 0;

    virtual void                    setLineColor(const cmm::FloatColor &clr)    = 0;
    virtual const cmm::FloatColor  &getLineColor(void) const                    = 0;
};

class IDynFaceDetail: public virtual IDetail
{
public:
    virtual void                    setFaceColor(const cmm::FloatColor &clr)    = 0;
    virtual const cmm::FloatColor  &getFaceColor(void) const                    = 0;

    virtual void                    setBorderColor(const cmm::FloatColor &clr)  = 0;
    virtual const cmm::FloatColor  &getBorderColor(void) const                  = 0;

    virtual void                    setBorderWidth(double dblBorderWidth)       = 0;
    virtual double                  getBorderWidth(void) const                  = 0;
};

class IDynImageDetail : public virtual IDetail
{
public:
    virtual void                    setImageID(const ID &id)                                = 0;
    virtual const ID               &getImageID(void) const                                  = 0;

    virtual void                    setImageFilePath(const std::string &path)               = 0;
    virtual const std::string      &getImageFilePath(void) const                            = 0;

    virtual void                    setImageSize(float fltWidth, float fltHeight)           = 0;
    virtual void                    getImageSize(float &dblWidth, float &fltHeight) const   = 0;

    virtual void                    setText(const std::string &strText)                     = 0;
    virtual const std::string      &getText(void)                                           = 0;

    virtual void                    setTextFont(const std::string &strTextFont)             = 0;
    virtual const std::string      &getTextFont(void)                                       = 0;

    virtual void                    setTextSize(float fltTextSize)                          = 0;
    virtual float                   getTextSize(void)                                       = 0;

    virtual void                    setOrientateEye(bool bOrientateEye)                     = 0;
    virtual bool                    getOrientateEye(void) const                             = 0;

    virtual void                    setLockSize(bool bLockSize)                             = 0;
    virtual bool                    getLockSize(void) const                                 = 0;
};

class IBubbleTextDetail : public virtual IDynModelDetail
{
public:
    virtual void                    setContent(const std::string &t)            = 0;
    virtual const std::string      &getContent()const                           = 0;

    virtual void                    setFont(const std::string &f)               = 0;
    virtual const std::string      &getFont(void)const                          = 0;

    virtual void                    setTextSize(double size)                    = 0;
    virtual double                  getTextSize(void)const                      = 0;

    virtual void                    setTextColor(const cmm::FloatColor &clr)    = 0;
    virtual const cmm::FloatColor  &getTextColor(void) const                    = 0;

    virtual void                    setBorderColor(const cmm::FloatColor &clr)  = 0;
    virtual const cmm::FloatColor  &getBorderColor(void) const                  = 0;

    virtual void                    setBorderWidth(double dblBorderWidth)       = 0;
    virtual double                  getBorderWidth(void) const                  = 0;

    virtual void                    setBorderVisible(bool bBorderVisible)       = 0;
    virtual bool                    getBorderVisible(void) const                = 0;

    virtual void                    setBkVisible(bool bVisible)                 = 0;
    virtual bool                    getBkVisible(void) const                    = 0;

    virtual void                    setOrientateEye(bool bOrientateEye)         = 0;
    virtual bool                    getOrientateEye(void) const                 = 0;

    virtual cmm::math::Vector3d     getOffset()                                 = 0;
    virtual void                    setOffset(const cmm::math::Vector3d & offset) = 0;
};

class IPolygonDetail : public virtual IDynModelDetail
{
public:
    virtual void                    addVertex(const cmm::math::Vector2d &v)                 = 0;
    virtual bool                    getVertex(unsigned int i, cmm::math::Vector2d &v)const  = 0;
    virtual unsigned int            getVertexCount()const                                   = 0;

public:
    virtual void                    setBorderColor(const cmm::FloatColor &clr)  = 0;
    virtual const cmm::FloatColor  &getBorderColor(void) const                  = 0;

    virtual void                    setBorderWidth(double dblBorderWidth)       = 0;
    virtual double                  getBorderWidth(void) const                  = 0;

    virtual void                    setFaceVisible(bool bVisible)               = 0;
    virtual bool                    getFaceVisible(void) const                  = 0;
};


class IPolyPipeDetail : public virtual IDynModelDetail
{
public:
    virtual void        addVertex(const cmm::math::Vector2d &v) = 0;
    virtual void        getVertex(unsigned int i, cmm::math::Vector2d &v) const = 0;
    virtual unsigned    getVerticesCount(void) const = 0;
};


PARAM_EXPORT IDetail *createDetail(const std::string &strDetail, unsigned int nDatasetCode);
PARAM_EXPORT IDetail *createDetail(const ID &id);

}

#endif