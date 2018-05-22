#ifndef CUBE_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define CUBE_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class CubeDetail : public DynModelDetail, public ICubeDetail
{
public:
    explicit CubeDetail(void);
    explicit CubeDetail(unsigned int nDataSetCode);
    virtual ~CubeDetail(void);

protected:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

protected:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

protected:
    virtual const std::string   &getStyle(void) const { return CUBE_DETAIL; }

protected:
    virtual void        setCubeSize(double dblLength, double dblWidth, double dblHeight) { m_dblLength = dblLength, m_dblWidth = dblWidth, m_dblHeight = dblHeight; }
    virtual void        getCubeSize(double &dblLength, double &dblWidth, double &dblHeight) const { dblLength = m_dblLength, dblWidth = m_dblWidth, dblHeight = m_dblHeight; }

    virtual void        setTopVisible(bool bVisible)        {   m_bTopVisible = bVisible;       }
    virtual bool        getTopVisible(void) const           {   return m_bTopVisible;           }
    virtual void        setBottomVisible(bool bVisible)     {   m_bBottomVisible = bVisible;    }
    virtual bool        getBottomVisible(void) const        {   return m_bBottomVisible;        }

    virtual double      getBoundingSphereRadius(void) const;

protected:
    double  m_dblLength;
    double  m_dblWidth;
    double  m_dblHeight;
    bool    m_bTopVisible;
    bool    m_bBottomVisible;
};

}

#endif

