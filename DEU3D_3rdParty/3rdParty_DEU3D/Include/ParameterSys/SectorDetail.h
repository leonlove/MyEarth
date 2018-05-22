#ifndef SECTOR_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define SECTOR_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class SectorDetail : public DynModelDetail, public ISectorDetail
{
public:
    explicit SectorDetail(void);
    explicit SectorDetail(unsigned int nDataSetCode);
    virtual ~SectorDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;
    virtual double              getBoundingSphereRadius(void) const;
    virtual const std::string  &getStyle(void) const { return SECTOR_DETAIL; }

public:
    virtual void                    setBorderWidth(double dblWidth)             {   m_dblBorderWidth = dblWidth;    }
    virtual double                  getBorderWidth(void) const                  {   return m_dblBorderWidth;        }
    virtual void                    setBorderColor(const cmm::FloatColor &clr)  {   m_clrBorderColor = clr;         }
    virtual const cmm::FloatColor  &getBorderColor(void) const                  {   return m_clrBorderColor;        }
    virtual void                    setBeginAngle(double dblAngle)              {   m_dblBeginAngle = dblAngle;     }
    virtual double                  getBeginAngle(void) const                   {   return m_dblBeginAngle;         }
    virtual void                    setEndAngle(double dblAngle)                {   m_dblEndAngle = dblAngle;       }
    virtual double                  getEndAngle(void) const                     {   return m_dblEndAngle;           }
    virtual void                    setRadius1(double dblRadius)                {   m_dblRadius1 = dblRadius;       }
    virtual double                  getRadius1(void) const                      {   return m_dblRadius1;            }
    virtual void                    setRadius2(double dblRadius)                {   m_dblRadius2 = dblRadius;       }
    virtual double                  getRadius2(void) const                      {   return m_dblRadius2;            }

protected:
    double          m_dblBorderWidth;
    double          m_dblBeginAngle;
    double          m_dblEndAngle;
    double          m_dblRadius1;
    double          m_dblRadius2;
    cmm::FloatColor m_clrBorderColor;
};

}

#endif