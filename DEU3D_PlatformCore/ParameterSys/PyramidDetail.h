#ifndef PYRAMID_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define PYRAMID_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class PyramidDetail : public DynModelDetail, public IPyramidDetail
{
public:
    explicit PyramidDetail(void);
    explicit PyramidDetail(unsigned int nDataSetCode);
    virtual ~PyramidDetail(void);

public:
    virtual osg::Node                       *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual const std::string              &getStyle(void) const { return PYRAMID_DETAIL; }
    virtual bool                            fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                            toBson(bson::bsonDocument &bsonDoc) const;

    virtual double                          getBoundingSphereRadius(void) const;

    virtual void        setBottomImageID(const ID &id)                  {   m_BottomImgID = id;                         }
    virtual const       ID &getBottomImageID(void) const                {   return m_BottomImgID;                       }

    virtual void        setBottomVisible(bool bVisible)                 {   m_bBottomVisible = bVisible;                }
    virtual bool        getBottomVisible(void) const                    {   return m_bBottomVisible;                    }

    virtual void        addBottomVertex(const cmm::math::Point3d &vtx)  {   m_vecBottomVertices.push_back(vtx);         }
    virtual const       std::vector<cmm::math::Point3d> &getBottomVertices(void) const  {   return m_vecBottomVertices; }
    virtual unsigned    getBottomVerticesCount(void) const              {   return m_vecBottomVertices.size();          }

protected:
    ID      m_BottomImgID;
    bool    m_bBottomVisible;
    std::vector<cmm::math::Point3d>     m_vecBottomVertices;
};

}

#endif