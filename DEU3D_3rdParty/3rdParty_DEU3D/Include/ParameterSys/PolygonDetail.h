#ifndef POLYGON_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define POLYGON_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class PolygonDetail : public DynModelDetail, public IPolygonDetail
{
public:
    PolygonDetail(void);
    PolygonDetail(unsigned int nDataSetCode);
    ~PolygonDetail(void);

public:
    virtual osg::Node               *createDetailNode(const CreationInfo *pInfo) const;

    virtual const std::string       &getStyle(void) const{return POLYGON_DETAIL;}

    virtual bool                    fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                    toBson(bson::bsonDocument &bsonDoc) const;

    virtual IHole *                 generateHole(){return NULL;}
    virtual double                  getBoundingSphereRadius(void) const;

public:
    virtual void                    addVertex(const cmm::math::Vector2d &v){m_vecVertices.push_back(v);}
    virtual bool                    getVertex(unsigned int i, cmm::math::Vector2d &v)const{if (i < m_vecVertices.size()) {v = m_vecVertices[i]; return true; } else return false;}
    virtual unsigned int            getVertexCount()const{return m_vecVertices.size();}

public:
    virtual void                    setBorderColor(const cmm::FloatColor &clr){m_clrBorder = clr;}
    virtual const cmm::FloatColor  &getBorderColor(void) const{return m_clrBorder;}

    virtual void                    setBorderWidth(double dblBorderWidth){m_dblBorderWidth;}
    virtual double                  getBorderWidth(void) const{return m_dblBorderWidth;}

    virtual void                    setFaceVisible(bool bVisible){m_bFaceVisible = bVisible;}
    virtual bool                    getFaceVisible(void) const {return m_bFaceVisible;}

protected:
    cmm::FloatColor                     m_clrBorder;
    double                              m_dblBorderWidth;
    bool                                m_bBorderVisible;
    bool                                m_bFaceVisible;
    std::vector<cmm::math::Vector2d>    m_vecVertices;
};

}

#endif