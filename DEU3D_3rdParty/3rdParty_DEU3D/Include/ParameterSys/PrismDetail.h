#ifndef PRISM_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define PRISM_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class PrismDetail : public DynModelDetail, public IPrismDetail
{
public:
    explicit                    PrismDetail(void);
    explicit                    PrismDetail(unsigned int nDataSetCode);
    virtual                    ~PrismDetail(void);

public:
    virtual osg::Node          *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual const std::string  &getStyle(void) const { return PRISM_DETAIL; }
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual void                setTopImageID(const ID &id)         {   m_TopImgID = id;                }
    virtual const ID           &getTopImageID(void) const           {   return m_TopImgID;              }

    virtual void                setBottomImageID(const ID &id)      {   m_BottomImgID = id;             }
    virtual const ID           &getBottomImageID(void) const        {   return m_BottomImgID;           }

    virtual void                setHeight(double dblHeight)         {   m_dblHeight = dblHeight;        }
    virtual double              getHeight(void) const               {   return m_dblHeight;             }

    virtual void                setTopVisible(bool bVisible)        {   m_bTopVisible = bVisible;       }
    virtual bool                getTopVisible(void) const           {   return m_bTopVisible;           }
    virtual void                setBottomVisible(bool bVisible)     {   m_bBottomVisible = bVisible;    }
    virtual bool                getBottomVisible(void) const        {   return m_bBottomVisible;        }

    virtual double              getBoundingSphereRadius(void) const;

    virtual void                            addVertex(const cmm::math::Point2d &vtx)    {   m_vecVertices.push_back(vtx);   }
    virtual void                            clearVertices(void)                         {   m_vecVertices.clear();          }
    virtual std::vector<cmm::math::Point2d> &getVertices(void)                          {   return m_vecVertices;           }
    virtual const std::vector<cmm::math::Point2d> &getVertices(void) const              {   return m_vecVertices;           }

protected:
    osg::Node *createAsPointParameter(const PointCreationInfo *pPointInfo) const;
    osg::Node *createAsLineParameter(const PolyCreationInfo *pPolyInfo) const;

protected:
    ID      m_TopImgID;
    ID      m_BottomImgID;
    double  m_dblHeight;
    bool    m_bBottomVisible;
    bool    m_bTopVisible;
    std::vector<cmm::math::Point2d> m_vecVertices;
};

}

#endif
