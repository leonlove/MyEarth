#ifndef POLYGON_PIPE_CONNECTOR_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define POLYGON_PIPE_CONNECTOR_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class PolygonPipeConnectorDetail: public DynModelDetail, public virtual IPolygonPipeConnectorDetail
{
public:
    PolygonPipeConnectorDetail(void);
    PolygonPipeConnectorDetail(unsigned int nDataSetCode);
    ~PolygonPipeConnectorDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

    virtual void                setType(const std::string &type = "normal"){m_strType = type;};//¿ÉÑ¡Öµ:normal¡¢endblock¡¢pipehat¡¢weld
    virtual std::string         getType()const{return m_strType;};

    virtual const std::string  &getStyle(void) const { return PIPECONNECTOR_DETAIL; }
    virtual double              getBoundingSphereRadius(void) const;

    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual bool                addJoint(const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon);
    virtual bool                setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon);
    virtual bool                getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, cmm::math::Polygon2 &polygon) const;
    virtual unsigned            getJointsCount(void) const {return m_vecJoints.size();}

protected:
    struct Joint
    {
        double              m_dblLength;
        cmm::math::Polygon2 m_polygon;
        cmm::math::Point3d  m_posTarget;
    };
    std::vector<Joint>      m_vecJoints;
    std::string             m_strType;
};

}

#endif