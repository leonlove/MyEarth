#ifndef INTERSECTION_EX_VISITOR_H
#define INTERSECTION_EX_VISITOR_H 1
/*
#include "Export"
#include <osgUtil/IntersectionVisitor>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/Geode>

namespace cmmOSG
{

class OSGUTIL_EXPORT IntersectionVisitor : public osgUtil::IntersectionVisitor
{
public:
    explicit IntersectionVisitor(osgUtil::Intersector* intersector = 0, osgUtil::IntersectionVisitor::ReadCallback* readCallback = 0);
    virtual ~IntersectionVisitor(void);

public:
    enum InterTagetType
    {
        ITT_ALL,
        ITT_TERRAIN,
        ITT_MODEL
    };

public:
    virtual void apply(osg::MatrixTransform &matrixtransform);
    virtual void apply(osg::Group &group);

public:
    inline void setInterTagetType(InterTagetType eInterTagetType) {    m_eInterTagetType = eInterTagetType;    }

    inline InterTagetType getInterTagetType() {    return m_eInterTagetType;    }

protected:

protected:
    InterTagetType m_eInterTagetType;
};

}
*/
#endif