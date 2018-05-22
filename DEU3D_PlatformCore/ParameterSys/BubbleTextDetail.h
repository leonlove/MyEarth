#ifndef BUBBLE_TEXT_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define BUBBLE_TEXT_DETAIL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "DynModelDetail.h"

namespace param
{

class BubbleTextDetail:public DynModelDetail, public IBubbleTextDetail
{
public:
    explicit BubbleTextDetail();
    BubbleTextDetail(unsigned int nDataSetCode);
    ~BubbleTextDetail();

public:
    virtual osg::Node               *createDetailNode(const CreationInfo *pInfo) const;

    virtual const std::string       &getStyle(void) const{return BUBBLETEXT_DETAIL;}

    virtual bool                    fromBson(bson::bsonDocument &bsonDoc);
    virtual bool                    toBson(bson::bsonDocument &bsonDoc) const;

    virtual IHole *                 generateHole(void){return NULL;}
    virtual double                  getBoundingSphereRadius(void) const;

public:
    virtual void                    setContent(const std::string &t){m_strText = t;}
    virtual const std::string      &getContent(void)const{return m_strText;}

    virtual void                    setFont(const std::string &f){m_strFont = f;};
    virtual const std::string      &getFont(void)const{return m_strFont;};

    virtual void                    setTextSize(double size){m_dSize = size;};
    virtual double                  getTextSize(void)const{return m_dSize;};

    virtual void                    setTextColor(const cmm::FloatColor &clr){m_clrText = clr;}
    virtual const cmm::FloatColor  &getTextColor(void) const{return m_clrText;}

    virtual void                    setBorderColor(const cmm::FloatColor &clr){m_clrBorder = clr;}
    virtual const cmm::FloatColor  &getBorderColor(void) const{return m_clrBorder;}

    virtual void                    setBorderWidth(double dblBorderWidth){m_dBorderWidth;}
    virtual double                  getBorderWidth(void) const{return m_dBorderWidth;}

    virtual void                    setBorderVisible(bool bBorderVisible){m_bBorderVisible = bBorderVisible;}
    virtual bool                    getBorderVisible(void) const{return m_bBorderVisible;}

    virtual void                    setBkVisible(bool bVisible){m_bBkVisible = bVisible;}
    virtual bool                    getBkVisible(void) const {return m_bBkVisible;}

    virtual void                    setOrientateEye(bool bOrientateEye) { m_bOrientateEye = bOrientateEye; }
    virtual bool                    getOrientateEye(void) const { return m_bOrientateEye; }

    virtual cmm::math::Vector3d     getOffset(){return m_vOffset;}
    virtual void                    setOffset(const cmm::math::Vector3d & offset){m_vOffset = offset;}

protected:
    cmm::math::Vector3d m_vOffset;
    std::string     m_strText;
    std::string     m_strFont;
    double          m_dSize;
    cmm::FloatColor m_clrBorder;
    cmm::FloatColor m_clrText;
    double          m_dBorderWidth;
    bool            m_bBorderVisible;
    bool            m_bBkVisible;
    bool            m_bOrientateEye;
};

}

#endif