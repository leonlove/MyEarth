#ifndef SPHERE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define SPHERE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class SphereDetail : public DynModelDetail, public ISphereDetail
{
public:
    explicit SphereDetail(void);
    explicit SphereDetail(unsigned int nDataSetCode);
    virtual ~SphereDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string  &getStyle(void) const { return SPHERE_DETAIL; }

public:
    virtual void        setRadius(double dblRadius) { m_dblRadius = dblRadius; }
    virtual double      getRadius(void) const { return m_dblRadius; }

    virtual double      getBoundingSphereRadius(void) const;
protected:
    double m_dblRadius;
};

}

#endif

