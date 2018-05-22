#ifndef ROUND_TABLE_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define ROUND_TABLE_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

    class RoundTableDetail : public DynModelDetail, public IRoundTableDetail
    {
    public:
        explicit RoundTableDetail(void);
        explicit RoundTableDetail(unsigned int nDataSetCode);
        virtual ~RoundTableDetail(void);

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
        virtual void        setRadiusTop(double dblRadius)      {   m_dblTopRadius = dblRadius;     }
        virtual double      getRadiusTop(void) const            {   return m_dblTopRadius;          }
        virtual void        setRadiusBottom(double dblRadius)   {   m_dblBottomRadius = dblRadius;  }
        virtual double      getRadiusBottom(void) const         {   return m_dblBottomRadius;       }
        virtual void        setTopVisible(bool bVisible)        {   m_bTopVisible = bVisible;       }
        virtual bool        getTopVisible(void) const           {   return m_bTopVisible;           }
        virtual void        setBottomVisible(bool bVisible)     {   m_bBottomVisible = bVisible;    }
        virtual bool        getBottomVisible(void) const        {   return m_bBottomVisible;        }

        virtual double      getBoundingSphereRadius(void) const;

    protected:
        bool        m_bTopVisible;
        bool        m_bBottomVisible;
        double      m_dblTopRadius;
        double      m_dblBottomRadius;
        double      m_dblHeight;
        unsigned    m_nEdgeCount;
};

}

#endif