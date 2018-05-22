#ifndef DYNLINE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DYNLINE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{


class DynLineDetail : public Detail, public IDynLineDetail
{
public:
    explicit DynLineDetail(void);
    explicit DynLineDetail(unsigned int nDataSetCode);
    explicit DynLineDetail(const ID &id) : Detail(id) {}
    virtual ~DynLineDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string   &getStyle(void) const { return LINE_DETAIL; }

public:
    virtual void                    setLineStyle(const std::string &strStyle) { m_strStyle = strStyle; }
    virtual const std::string      &getLineStyle(void) const { return m_strStyle; }

    virtual void                    setLineWidth(double dblLineWidth) { m_dblLineWidth = dblLineWidth; }
    virtual double                  getLineWidth(void) const { return m_dblLineWidth; }

    virtual void                    setLineImage(const ID &id) { m_LineImgID = id; }
    virtual const ID               &getLineImage(void) const { return m_LineImgID; }

    virtual void                    setLineColor(const cmm::FloatColor &clr) { memcpy(&m_LineClr, &clr, sizeof(cmm::FloatColor)); }
    virtual const cmm::FloatColor  &getLineColor(void) const { return m_LineClr; }

    virtual double                  getBoundingSphereRadius(void) const{   return -1;}

private:
	osg::Node*						createAsNonIntegration(const PolyCreationInfo* pPolyInfo) const;

protected:
    std::string     m_strStyle;
    double          m_dblLineWidth;
    ID              m_LineImgID;
    cmm::FloatColor m_LineClr;
};

}

#endif