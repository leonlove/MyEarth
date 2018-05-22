#ifndef DYNFACE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DYNFACE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{


class DynFaceDetail: public Detail, public IDynFaceDetail
{
public:
    explicit DynFaceDetail(void);
    explicit DynFaceDetail(unsigned int nDataSetCode);
    explicit DynFaceDetail(const ID &id) : Detail(id) {}
    virtual ~DynFaceDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string  &getStyle(void) const { return FACE_DETAIL; }

public:
    virtual void                    setFaceColor(const cmm::FloatColor &clr) { memcpy(&m_FaceClr, &clr, sizeof(cmm::FloatColor)); }
    virtual const cmm::FloatColor  &getFaceColor(void) const { return m_FaceClr; }

    virtual void                    setBorderColor(const cmm::FloatColor &clr) { memcpy(&m_BorderClr, &clr, sizeof(cmm::FloatColor)); }
    virtual const cmm::FloatColor  &getBorderColor(void) const { return m_BorderClr; }

    virtual void                    setBorderWidth(double dblBorderWidth) { m_dblBorderWidth = dblBorderWidth; }
    virtual double                  getBorderWidth(void) const { return m_dblBorderWidth; }

    virtual double                  getBoundingSphereRadius(void) const{   return -1;}
protected:
    double          m_dblBorderWidth;
    cmm::FloatColor m_FaceClr;
    cmm::FloatColor m_BorderClr;
};

}

#endif