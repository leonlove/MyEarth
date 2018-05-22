#ifndef LINE_PARAMETER_H_88DAC155_2543_4F11_98EA_A360C77E715C_INCLUDE
#define LINE_PARAMETER_H_88DAC155_2543_4F11_98EA_A360C77E715C_INCLUDE

#include "ILineParameter.h"
#include "Parameter.h"
#include <vector>

namespace param
{

class LineParameter : public Parameter, public ILineParameter
{
public:
    explicit LineParameter(const ID &id);
    virtual ~LineParameter(void);

    virtual cmm::math::Sphered  getBoundingSphere(void) const;

protected:
    virtual osg::Node                       *createParameterNode(void) const;
    virtual bool                            fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                            toBson(bson::bsonDocument &bsonDoc) const;

protected:
    virtual void                            addCoordinate(const cmm::math::Point3d &point);
    virtual bool                            insertCoordinate(unsigned nIndex, const cmm::math::Point3d &point);
    virtual bool                            setCoordinate(unsigned nIndex, const cmm::math::Point3d &point);
    virtual bool                            removeCoordinate(unsigned nIndex);
    virtual unsigned                        getCoordinateCount(void) const;
    virtual const                           cmm::math::Point3d &getCoordinate(unsigned nIndex) const;
    virtual void                            setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates);
    virtual const std::vector<cmm::math::Point3d> &getCoordinates(void) const;

    virtual void                            addPart(unsigned nOffset, unsigned nCount);
    virtual bool                            getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const;
    virtual unsigned int                    getPartCount(void) const;

    virtual void                            setTerrainMagnet(bool bTerrainMagnet) { m_bTerrainMagnet = bTerrainMagnet; }
    virtual bool                            getTerrainMagnet(void) const { return m_bTerrainMagnet; }

	virtual void                            addColor(const cmm::math::Vector3d &color);
protected:
    std::vector<cmm::math::Point3d>             m_vecCoordinates;
    std::vector<std::pair<unsigned, unsigned> > m_vecParts;
    bool                                        m_bTerrainMagnet;
	std::vector<cmm::math::Vector3d>            m_vecColors;
};

}

#endif

