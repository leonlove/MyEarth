#ifndef SMOKE_STATE_H_B873C0E4_6B31_4686_B2A7_C6DC69F01E8F_INCLUDE
#define SMOKE_STATE_H_B873C0E4_6B31_4686_B2A7_C6DC69F01E8F_INCLUDE

#include "ISmokeState.h"
#include "StateBase.h"

//此为零时添加解决烟雾内存问题的，待正式解决后需要删除
#include "SceneGraphOperator.h"
//

class SmokeState : virtual public ISmokeState, public StateBase
{
public:
    explicit SmokeState(const std::string &strName);
    //此为零时添加解决烟雾内存问题的，待正式解决后需要删除
    explicit SmokeState(const std::string &strName, SceneGraphOperator *pSceneGraphOperator);
    //
    virtual ~SmokeState(void);
public:
    virtual const       std::string &getType(void) const   {   return cmm::STATE_SMOKE;  }
public:
    virtual void setSmokeImageFile(const std::string &strSmokeFile);
    virtual void setSmokeSize(double dblSize);
    virtual const cmm::FloatColor getSmokeColor() const;
    virtual void setSmokeColor(const cmm::FloatColor &clr);
    virtual void setSmokeDirection(double dblAzimuth, double dblPitch);
    virtual void getSmokeDirection(double &dblAzimuth, double &dblPitch);
public:
    virtual bool applyState(osg::Node *pNode, bool bApply);
protected:
    osg::Node *createEffect(void);
protected:
    std::string m_strSmokeImageFile;
    double      m_dblSize;
    osg::Vec4   m_color;
    double m_dblAzimuth, m_dblPitch;

    //此为零时添加解决烟雾内存问题的，待正式解决后需要删除
    SceneGraphOperator *m_pSceneGraphOperator;
    //

public:
};

#endif
