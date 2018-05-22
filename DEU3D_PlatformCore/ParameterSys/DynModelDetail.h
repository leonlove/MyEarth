#ifndef DYN_MODEL_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DYN_MODEL_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{

class DynModelDetail : public Detail, public virtual IDynModelDetail
{
public:
    explicit DynModelDetail(void);
    explicit DynModelDetail(unsigned int nDataSetCode, DeuObjectIDType type);
    virtual ~DynModelDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const = 0;

public:
    virtual bool                    fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                    toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual void                    setImageID(const ID &id) { m_ImgID = id; }
    virtual const ID               &getImageID(void) const { return m_ImgID; }

    virtual void                    setDynModelColor(const cmm::FloatColor &clr) { memcpy(&m_Color, &clr, sizeof(cmm::FloatColor)); }
    virtual const cmm::FloatColor  &getDynModelColor(void) const { return m_Color; }

    virtual IHole *                 generateHole(void);
    virtual void                    getHoleList(std::vector<IHole *> &holeList) const;

protected:
    std::vector<osg::Vec3>          Hole2Vertex(const IHole *pHole, const osg::Vec3d &vPoint, double dblOffset, double dblAzimuthAngle = 0.0) const;
    void                            genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, std::vector<osg::Vec3> &vecVertices) const;

protected:
    ID                              m_ImgID;
    cmm::FloatColor                 m_Color;
    std::vector<OpenSP::sp<IHole> > m_vecHoles;
};

}

#endif

