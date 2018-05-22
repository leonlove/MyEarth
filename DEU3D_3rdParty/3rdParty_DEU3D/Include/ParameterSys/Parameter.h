#ifndef PARAMETER_H_6AEA06F0_756E_44B9_BE03_39E655C2B94A_INCLUDE
#define PARAMETER_H_6AEA06F0_756E_44B9_BE03_39E655C2B94A_INCLUDE

#include "IParameter.h"

#include <OpenSP/sp.h>
#include <osg/Node>

#include <vector>
#include <string>
#include <Common/memPool.h>

namespace param
{

class PARAM_EXPORT Parameter : virtual public IParameter
{
public:
    explicit Parameter(const ID &id);
    virtual ~Parameter(void);

public:
    virtual osg::Node *createParameterNode(void) const = 0;
    virtual bool       fromBson(bson::bsonDocument &bsonDoc);
    virtual bool       toBson(bson::bsonDocument &bsonDoc) const;

protected:
    virtual const ID               &getID(void) const { return m_ID; }
    virtual void                    setID(const ID &id){m_ID = id;}

    virtual void                    setFollowByTerrain(bool bFollow) { m_bFollowByTerrain = bFollow; }
    virtual bool                    isFollowByTerrain(void) const { return m_bFollowByTerrain; }

    virtual void                    setCoverOrder(unsigned nOrder) { m_nCoverOrder = nOrder; }
    virtual unsigned int            getCoverOrder(void) const { return m_nCoverOrder; }

    virtual void                    setHeight(double dblHeight) { m_dblHeight = dblHeight; }
    virtual double                  getHeight(void) const { return m_dblHeight; }

    virtual void                    setActProperty(unsigned nIndex) { if (nIndex < getPropertyCount()) m_nActProperty = nIndex; }
    virtual unsigned                getActProperty(void) const { return m_nActProperty; }

    virtual unsigned int            addProperty(const std::string &strKey, const std::string &strProp);
    virtual void                    getProperty(unsigned int nIndex, std::string &strKey, std::string &strProp) const;
    virtual unsigned int            getPropertyCount(void) const;
    virtual unsigned int            findProperty(const std::string &strKey, std::string &strVal) const;

    virtual void                    addDetail(const ID &id, double dMinRange, double dMaxRange);
    virtual bool                    getDetail(unsigned i, ID &id, double &dMinRange, double &dMaxRange) const;
    virtual unsigned                getNumDetail(void) const {return m_vecDetails.size();}
    virtual double                  getMaxRange(void) const;

protected:
    void                            readProperties(bson::bsonDocument &bsonProp);
    void                            writeProperties(bson::bsonDocument &bsonProp) const;
    bool                            getSortedDetails(std::vector<ID> &vecDetails, std::vector<std::pair<double, double> > &vecRanges) const;

protected:
    ID                                                  m_ID;
    unsigned int                                        m_nCoverOrder;
    double                                              m_dblHeight;
    unsigned int                                        m_nActProperty;
    std::vector<std::pair<std::string, std::string> >   m_vecProperties;

    std::vector<ID>                                     m_vecDetails;
    std::vector<std::pair<double, double> >             m_vecVisibleRange;
    bool                                                m_bFollowByTerrain;
};

}

#endif
