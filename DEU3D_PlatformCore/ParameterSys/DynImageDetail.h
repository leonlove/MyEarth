#ifndef DYNIMAGE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define DYNIMAGE_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{

class DynImageDetail : public Detail, public IDynImageDetail
{
public:
    explicit DynImageDetail(void);
    explicit DynImageDetail(unsigned int nDataSetCode);
    virtual ~DynImageDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool                fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                toBson(bson::bsonDocument &bsonDoc) const;

public:
    virtual const std::string  &getStyle(void) const { return IMAGE_DETAIL; }

public:
    virtual void                    setImageID(const ID &id) { m_ImgID = id; }
    virtual const ID               &getImageID(void) const { return m_ImgID; }

    virtual void                    setImageFilePath(const std::string &path) { m_strImagePath = path; }
    virtual const std::string      &getImageFilePath(void) const { return m_strImagePath; }

    virtual void                    setText(const std::string &strText) { m_strText = strText; }
    virtual const std::string      &getText(void) { return m_strText; }

    virtual void                    setTextFont(const std::string &strTextFont) { m_strTextFont = strTextFont; }
    virtual const std::string      &getTextFont(void) { return m_strTextFont; }

    virtual void                    setTextSize(float fltTextSize) { m_fltTextSize = fltTextSize; }
    virtual float                   getTextSize(void) { return m_fltTextSize;}

    virtual void                    setImageSize(float fltWidth, float fltHeight) { m_fltImageWidth = fltWidth; m_fltImageHeight = fltHeight; }
    virtual void                    getImageSize(float &fltWidth, float &fltHeight) const { fltWidth = m_fltImageWidth, fltHeight = m_fltImageHeight; }

    virtual void                    setOrientateEye(bool bOrientateEye) { m_bOrientateEye = bOrientateEye; }
    virtual bool                    getOrientateEye(void) const { return m_bOrientateEye; }

    virtual void                    setLockSize(bool bLockSize) { m_bLockSize = bLockSize; }
    virtual bool                    getLockSize(void) const { return m_bLockSize; }

    virtual double                  getBoundingSphereRadius(void) const;

protected:
    void init(void);

protected:
    ID          m_ImgID;
    float       m_fltImageWidth;
    float       m_fltImageHeight;
    float       m_fltTextSize;
    bool        m_bOrientateEye;
    bool        m_bLockSize;
    std::string m_strImagePath;
    std::string m_strText;
    std::string m_strTextFont;
};

}

#endif