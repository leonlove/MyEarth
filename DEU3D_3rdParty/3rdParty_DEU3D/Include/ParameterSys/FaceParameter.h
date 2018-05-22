#ifndef LINE_PARAMETER_H_823DE503_E4BB_46A4_B34A_C0E3A6B8F166_INCLUDE
#define LINE_PARAMETER_H_823DE503_E4BB_46A4_B34A_C0E3A6B8F166_INCLUDE

#include "IFaceParameter.h"
#include "Parameter.h"
#include <vector>

namespace param
{

class FaceParameter : public Parameter, public IFaceParameter
{
public:
    explicit FaceParameter(const ID &id);
    virtual ~FaceParameter(void);

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
    virtual const cmm::math::Point3d       &getCoordinate(unsigned nIndex) const;
    virtual void                            setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates);
    virtual const std::vector<cmm::math::Point3d> &getCoordinates(void) const;

    virtual void                            addPart(unsigned nOffset, unsigned nCount);
    virtual bool                            getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const;
    virtual unsigned int                    getPartCount(void) const;
protected:
    std::vector<cmm::math::Point3d>             m_vecCoordinates;
    std::vector<std::pair<unsigned, unsigned> > m_vecParts;
};

}

#endif

