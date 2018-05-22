#ifndef DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "IDetail.h"

#include <OpenSP\sp.h>
#include <osg/Node>
#include <osg/Vec4>
#include <osg/Material>
#include <osg/Point>
#include <osg/LineWidth>
#include <map>
#include <OpenThreads/Mutex>
#include <Common/memPool.h>

namespace param
{

class PARAM_EXPORT Detail : public virtual IDetail
{
public:
    explicit Detail(void)
    {
    }
    explicit Detail(const ID &id){m_id = id; m_bImageStretched = true;}
    virtual ~Detail(void) = 0 {}

public:
    class CreationInfo : public osg::Referenced
    {
    public:
        virtual ~CreationInfo(void) = 0 {}
        osg::ref_ptr<osg::Vec3dArray> m_pPoints;
    };

    class PointCreationInfo : public CreationInfo
    {
    public:
        double      m_dblAzimuthAngle;
        double      m_dblPitchAngle;
		double		m_dblRollAngle;
        osg::Vec3d  m_Scale;
    };

    class PolyCreationInfo : public CreationInfo
    {
    public:
        unsigned int m_nOffset;
        unsigned int m_nCount;
    };

	class PointCloudCreationInfo : public CreationInfo
	{
	public:
		osg::ref_ptr<osg::Vec3dArray> m_pColors;
	};

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const = 0;

protected:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;
    virtual const ID           &getID(void) const;
    bool                        isImageStretched(){return m_bImageStretched;}
    void                        stretchImage(bool bStretch){m_bImageStretched = bStretch;}

protected:
    const osg::Texture          *bindTexture(const ID &idImage, osg::StateSet *pStateSet) const;
protected:
    ID      m_id;
    bool    m_bImageStretched;
};

}

#endif
