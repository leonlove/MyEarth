#ifndef AnimationModel_H_
#define AnimationModel_H_
#include "IAnimationModel.h"
#include <osg\ref_ptr>
#include <osg\MatrixTransform>

#pragma warning (disable : 4250)
//����ģ��(�ɻ����˵ȣ�
class AnimationModel : public IAnimationModel
{
public:
    explicit        AnimationModel(void);

    //����ID���ӷ�������ȡģ��
    virtual bool    loadFrom(const ID  &idModel);            //���첽�̣߳��������ݣ���ɺ�ҵ�������

    //����·���ӱ��ػ�ȡģ��
    virtual bool    loadFrom(const std::string &strLocalPath);//�ӱ��ش�������osg�ڵ㣬�ҵ�������

    //������̬
    virtual void    setPose(const CameraPose &pose);//��poseת�ɱ任����ģ�ͽڵ�δ�������ʱ������

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

    //��׶��
    std::vector<osg::Vec3d>                m_vecFrustum;        //���涥��
    osg::ref_ptr<osg::MatrixTransform>    m_pFrustum;            //osg�ڵ�
};

//����
//1.NaviModel *m = new NaviModel,���䱣����NavigationPathPlayer��
//2.��������ʼ��ʱ������m->load()
//3.����ˢ��ʱ������ʱ�䣬ͨ��NavigationPathPlayer���ṩ��·����̬������m.setPose(),���ģ����̬�ĸı�
#endif