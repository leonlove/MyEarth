#ifndef HOLE_H_AB31BF00_4599_49A2_85D9_E980BA48286E_INCLUDE
#define HOLE_H_AB31BF00_4599_49A2_85D9_E980BA48286E_INCLUDE

#include "IHole.h"
#include <assert.h>

namespace param
{

class Hole : public IHole
{
public:
    Hole(void) : 
    m_ptCenter(0.0, 0.0), 
    m_dblRadius(0.0),
    m_dblWidth(0.0),
    m_dblHeight(0.0)
    {
    }

    Hole(const Hole &hole)
        : m_ptCenter(hole.m_ptCenter),
        m_dblRadius(hole.m_dblRadius),
        m_dblWidth(hole.m_dblWidth),
        m_dblHeight(hole.m_dblHeight),
        m_dblAzimuthAngle(hole.m_dblAzimuthAngle),
        m_vecPolygon(hole.m_vecPolygon)
    {}
    ~Hole(void){}

    virtual const Hole &operator=(const Hole &hole)
    {
        if(this == &hole)   return *this;
        m_strHoleType     = hole.m_strHoleType;
        m_ptCenter        = hole.m_ptCenter;
        m_dblRadius       = hole.m_dblRadius;
        m_dblWidth        = hole.m_dblWidth;
        m_dblHeight       = hole.m_dblHeight;
        m_dblAzimuthAngle = hole.m_dblAzimuthAngle;
        m_vecPolygon      = hole.m_vecPolygon;
        return *this;
    }

    virtual const std::string &getHoleType(void) const
    {
        return m_strHoleType;
    }

    virtual void setOnTopFace(bool isOn) { m_bOnTopFace = isOn; }
    virtual bool isOnTopFace(void) const { return m_bOnTopFace; }

    virtual void setCircle(const cmm::math::Point2d &center, double radius)
    {
        m_ptCenter = center;
        m_dblRadius = radius;
        m_strHoleType = HOLETYPE_CIRCLE;
        m_vecPolygon.clear();
    }

    virtual bool getCircle(cmm::math::Point2d &center, double &radius)const
    {
        if(m_strHoleType != HOLETYPE_CIRCLE)   return false;
        center = m_ptCenter;
        radius = m_dblRadius;
        return true;
    }

    virtual void setRectangle(const cmm::math::Point2d &center, double width, double height, double dblAzimuthAngle)
    {
        m_ptCenter        = center;
        m_dblWidth        = width;
        m_dblHeight       = height;
        m_dblAzimuthAngle = dblAzimuthAngle;
        m_strHoleType     = HOLETYPE_RECTANGLE;
        m_vecPolygon.clear();
    }

    virtual bool getRectangle(cmm::math::Point2d &center, double &width, double &height, double &dblAzimuthAngle) const
    {
        if(m_strHoleType != HOLETYPE_RECTANGLE)   return false;
        center = m_ptCenter;
        width = m_dblWidth;
        height = m_dblHeight;
        dblAzimuthAngle = m_dblAzimuthAngle;
        return true;
    }

    virtual void addPolygonVertex(const cmm::math::Point2d &v)
    {
        m_strHoleType = HOLETYPE_POLYGON;
        m_vecPolygon.push_back(v);
    }


    virtual bool getPolygonVertex(size_t i, cmm::math::Point2d &v)const
    {
        if(m_strHoleType != HOLETYPE_POLYGON) return false;
        if (i >= m_vecPolygon.size())   return false;

        v = m_vecPolygon[i];

        return true;
    }

    virtual bool getAllPolygonVertex(std::vector<cmm::math::Point2d> &vecVeterx) const
    {
        if(m_strHoleType != HOLETYPE_POLYGON) return false;

        vecVeterx.assign(m_vecPolygon.begin(), m_vecPolygon.end());
        return true;
    }

    virtual void clearPolygonVertex(void)
    {
        if(m_strHoleType != HOLETYPE_POLYGON) return;
        m_vecPolygon.clear();
    }

    virtual unsigned getPolygonVerticesCount(void) const
    {
        if(m_strHoleType != HOLETYPE_POLYGON) return 0u;
        return m_vecPolygon.size();
    }

    virtual void toBson(bson::bsonDocument &bsonDoc) const
    {
        bsonDoc.AddBoolElement("IsOnTopFace", m_bOnTopFace);
        bsonDoc.AddStringElement("HoleType",  m_strHoleType.c_str());
        if(m_strHoleType == HOLETYPE_CIRCLE)
        {
            bsonDoc.AddDblElement("CenterX", m_ptCenter.x());
            bsonDoc.AddDblElement("CenterY", m_ptCenter.y());
            bsonDoc.AddDblElement("Radius",  m_dblRadius);
        }
        else if(m_strHoleType == HOLETYPE_RECTANGLE)
        {
            bsonDoc.AddDblElement("CenterX", m_ptCenter.x());
            bsonDoc.AddDblElement("CenterY", m_ptCenter.y());
            bsonDoc.AddDblElement("Width",   m_dblWidth);
            bsonDoc.AddDblElement("Height",  m_dblHeight);
            bsonDoc.AddDblElement("AzimuthAngle", m_dblAzimuthAngle);
        }
        else if(m_strHoleType == HOLETYPE_POLYGON)
        {
            bson::bsonArrayEle *arr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Polygon"));
            for(size_t i = 0; i < m_vecPolygon.size(); i++)
            {
                arr->AddDblElement(m_vecPolygon[i].x());
                arr->AddDblElement(m_vecPolygon[i].y());
            }
        }
        else assert(false);
    }

    virtual bool fromBson(bson::bsonDocument &bsonDoc)
    {
        bson::bsonBoolEle *pOnTopFace = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("IsOnTopFace"));
        if (pOnTopFace) m_bOnTopFace = pOnTopFace->BoolValue();
        else return false;

        bson::bsonStringEle *pHoleType = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("HoleType"));
        if (!pHoleType) return false;

        m_strHoleType = pHoleType->StrValue();
        if(m_strHoleType == HOLETYPE_CIRCLE)
        {
            bson::bsonDoubleEle *pCenterX = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("CenterX"));
            if (pCenterX) m_ptCenter.x() = pCenterX->DblValue();
            else return false;

            bson::bsonDoubleEle *pCenterY = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("CenterY"));
            if (pCenterY) m_ptCenter.y() = pCenterY->DblValue();
            else return false;

            bson::bsonDoubleEle *pRadius = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("Radius"));
            if (pRadius) m_dblRadius = pRadius->DblValue();
            else return false;
        }
        else if(m_strHoleType == HOLETYPE_RECTANGLE)
        {
            bson::bsonDoubleEle *pCenterX = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("CenterX"));
            if (pCenterX) m_ptCenter.x() = pCenterX->DblValue();
            else return false;

            bson::bsonDoubleEle *pCenterY = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("CenterY"));
            if (pCenterY) m_ptCenter.y() = pCenterY->DblValue();
            else return false;

            bson::bsonDoubleEle *pWidth = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("Width"));
            if (pWidth) m_dblWidth = pWidth->DblValue();
            else return false;

            bson::bsonDoubleEle *pHeight = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("Height"));
            if (pHeight) m_dblHeight = pHeight->DblValue();
            else return false;

            bson::bsonDoubleEle *pAzimuth = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("AzimuthAngle"));
            if (pAzimuth) m_dblAzimuthAngle = pAzimuth->DblValue();
            else return false;
        }
        else if(m_strHoleType == HOLETYPE_POLYGON)
        {
            bson::bsonArrayEle *pPolygon = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement("Polygon"));
            if (!pPolygon) return false;

            for(unsigned int i = 0u; i < pPolygon->ChildCount();)
            {
                bson::bsonDoubleEle *x = dynamic_cast<bson::bsonDoubleEle*>(pPolygon->GetElement(i++));
                bson::bsonDoubleEle *y = dynamic_cast<bson::bsonDoubleEle*>(pPolygon->GetElement(i++));

                if (!x || !y) return false;
                m_vecPolygon.push_back(cmm::math::Vector2d(x->DblValue(), y->DblValue()));
            }
        }
        else return false;
        return true;
    }

protected:
    bool                            m_bOnTopFace;
    std::string                     m_strHoleType;
    cmm::math::Point2d              m_ptCenter;
    double                          m_dblRadius;
    double                          m_dblWidth;
    double                          m_dblHeight;
    double                          m_dblAzimuthAngle;
    std::vector<cmm::math::Point2d> m_vecPolygon;
};

}
#endif