#ifndef AnimationModel_H_
#define AnimationModel_H_
#include "IAnimationModel.h"
#include <osg\ref_ptr>
#include <osg\MatrixTransform>

#pragma warning (disable : 4250)
//动画模型(飞机，人等）
class AnimationModel : public IAnimationModel
{
public:
    explicit        AnimationModel(void);

    //根据ID，从服务器获取模型
    virtual bool    loadFrom(const ID  &idModel);            //开异步线程，下载数据，完成后挂到场景中

    //根据路径从本地获取模型
    virtual bool    loadFrom(const std::string &strLocalPath);//从本地磁盘载入osg节点，挂到场景中

    //设置姿态
    virtual void    setPose(const CameraPose &pose);//将pose转成变换矩阵（模型节点未完成载入时跳过）

    virtual void    setObserverPosX(double dblOffset)   {   m_ptObserverPos.x() = dblOffset;    }
    virtual double  getObserverPosX(void) const         {   return m_ptObserverPos.x();         }

    virtual void    setObserverPosY(double dblOffset)   {   m_ptObserverPos.y() = dblOffset;    }
    virtual double  getObserverPosY(void) const         {   return m_ptObserverPos.x();         }

    virtual void    setObserverPosZ(double dblOffset)   {   m_ptObserverPos.z() = dblOffset;    }
    virtual double  getObserverPosZ(void) const         {   return m_ptObserverPos.x();         }

    virtual void    setFirstPerson(bool bFirst);
    virtual bool    isFirstPerson(void) const;

    virtual void    setFrustum(double x0, double y0, double z0, 
                               double x1, double y1, double z1, 
                               double x2, double y2, double z2, 
                               double x3, double y3, double z3);

    virtual void    getFrustum(double &x0, double &y0, double &z0, 
                               double &x1, double &y1, double &z1, 
                               double &x2, double &y2, double &z2, 
                               double &x3, double &y3, double &z3)const;

    virtual void    setFrustumColor(const cmm::FloatColor &clr);
    virtual const   cmm::FloatColor getFrustumColor(void) const;

    virtual void    showFrustum(bool bShow);
    virtual bool    isFrustumShown(void) const;

    virtual void    setVisible(bool visible);
    virtual bool    getVisible(void) const;

    virtual void    setForcePrivatePitch(bool bForce)       {   m_bForcePrivatePitch = bForce;      }
    virtual bool    isForcePrivatePitch(void) const         {   return m_bForcePrivatePitch;        }
    virtual void    setPrivatePitch(double dblPitch)        {   m_dblPrivatePitch = dblPitch;       }
    virtual double  getPrivatePitch(void) const             {   return m_dblPrivatePitch;           }

    virtual void    setForcePrivateAzimuth(bool bForce)     {   m_bForcePrivateAzimuth = bForce;    }
    virtual bool    isForcePrivateAzimuth(void) const       {   return m_bForcePrivateAzimuth;      }
    virtual void    setPrivateAzimuth(double dblAzimuth)    {   m_dblPrivateAzimuth = dblAzimuth;   }
    virtual double  getPrivateAzimuth(void) const           {   return m_dblPrivateAzimuth;         }

public:
    CameraPose          getObserverPose(double dblPitchOffset = 0.0, double dblAzimuthOffset = 0.0) const;
    osg::Node          *getModelNode(void)          {   return m_pModelRoot.get();  }
    const osg::Node    *getModelNode(void) const    {   return m_pModelRoot.get();  }

protected:
    void    createFrustum(void);
    void    updateFrustum();
protected:
    osg::ref_ptr<osg::Group>            m_pModelRoot;
    osg::ref_ptr<osg::MatrixTransform>  m_pModelNode;

    osg::Vec3d      m_ptObserverPos;
    CameraPose      m_poseCurrent;

    bool            m_bFirstPerson;

    bool            m_bForcePrivatePitch;
    double          m_dblPrivatePitch;
    bool            m_bForcePrivateAzimuth;
    double          m_dblPrivateAzimuth;

    //视锥体
    std::vector<osg::Vec3d>                m_vecFrustum;        //底面顶点
    osg::ref_ptr<osg::MatrixTransform>    m_pFrustum;            //osg节点
};

//流程
//1.NaviModel *m = new NaviModel,将其保存在NavigationPathPlayer中
//2.漫游器初始化时，调用m->load()
//3.画面刷新时，根据时间，通过NavigationPathPlayer中提供的路径姿态，调用m.setPose(),完成模型姿态的改变
#endif