#ifndef DYNPOINTCLOUD_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DYNPOINTCLOUD_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{

class DynPointCloudDetail : public Detail, public IDynPointCloudDetail
{
public:
    explicit DynPointCloudDetail(void);
    explicit DynPointCloudDetail(unsigned int nDataSetCode);
    explicit DynPointCloudDetail(const ID &id) : Detail(id) {}
    virtual ~DynPointCloudDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string  &getStyle(void) const { return POINTCLOUD_DETAIL; }

public:
    virtual void                    setPointColor(const cmm::FloatColor &clr) { memcpy(&m_PointClr, &clr, sizeof(cmm::FloatColor)); }
    virtual const cmm::FloatColor  &getPointColor(void) const { return m_PointClr; }

    virtual void                    setPointSize(double dblSize) { m_dblPointSize = dblSize; }
    virtual double                  getPointSize(void) const { return m_dblPointSize; }

    virtual double                  getBoundingSphereRadius(void) const;

private:
	osg::Geode*						createPointCloudNode(const PointCloudCreationInfo* pPointCloudInfo) const;

protected:
    double				m_dblPointSize;
    cmm::FloatColor		m_PointClr;

};

}

#endif
