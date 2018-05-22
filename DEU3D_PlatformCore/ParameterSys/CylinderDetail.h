#ifndef CYLINDER_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define CYLINDER_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class CylinderDetail : public DynModelDetail, public ICylinderDetail
{
public:
    explicit CylinderDetail(void);
    explicit CylinderDetail(unsigned int nDataSetCode);
    virtual ~CylinderDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string  &getStyle(void) const { return CYLINDER_DETIAL; }

public:
    virtual void        setHeight(double dblHeight)         {   m_dblHeight = dblHeight;        }
    virtual double      getHeight(void) const               {   return m_dblHeight;             }
    virtual void        setRadius(double dblRadius)         {   m_dblRadius = dblRadius;        }
    virtual double      getRadius(void) const               {   return m_dblRadius;             }
    virtual void        setTopVisible(bool bVisible)        {   m_bTopVisible = bVisible;       }
    virtual bool        getTopVisible(void) const           {   return m_bTopVisible;           }
    virtual void        setBottomVisible(bool bVisible)     {   m_bBottomVisible = bVisible;    }
    virtual bool        getBottomVisible(void) const        {   return m_bBottomVisible;        }

    virtual double      getBoundingSphereRadius(void) const;

protected:
    bool        m_bTopVisible;
    bool        m_bBottomVisible;
    double      m_dblRadius;
    double      m_dblHeight;
    unsigned    m_nEdgeCount;
};

}

#endif