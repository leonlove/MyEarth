#ifndef IAnimationModel_H
#define IAnimationModel_H

#include <OpenThreads\Thread>
#include <string>
#include <IDProvider\ID.h>
#include <common\Common.h>

#include "NavigationParam.h"

class IAnimationModel: virtual public OpenSP::Ref
{
public:
    //根据ID，从服务器获取模型
    virtual bool    loadFrom(const ID &idModel) = 0;//下载数据，完成后挂到场景中

    //根据路径从本地获取模型
    virtual bool    loadFrom(const std::string &strLocalPath) = 0;//从本地磁盘载入osg节点，挂到场景中

    //设置姿态
    virtual void    setPose(const CameraPose &pose) = 0;        //将pose转成变换矩阵（模型节点未完成载入时跳过）

    //设置提前时间（相对相机位置)
    virtual void    setObserverPosX(double dblOffset) = 0;
    virtual double  getObserverPosX(void) const = 0;

    virtual void    setObserverPosY(double dblOffset) = 0;
    virtual double  getObserverPosY(void) const = 0;

    virtual void    setObserverPosZ(double dblOffset) = 0;
    virtual double  getObserverPosZ(void) const = 0;

    virtual void    setFirstPerson(bool bFirst) = 0;
    virtual bool    isFirstPerson(void) const = 0;

    virtual void    setVisible(bool visible) = 0;
    virtual bool    getVisible(void) const = 0;

    virtual void	setFrustum(double x0, double y0, double z0, 
							   double x1, double y1, double z1, 
							   double x2, double y2, double z2, 
							   double x3, double y3, double z3) = 0;

	virtual void	getFrustum(double &x0, double &y0, double &z0, 
							   double &x1, double &y1, double &z1, 
							   double &x2, double &y2, double &z2, 
							   double &x3, double &y3, double &z3)const = 0;

    virtual void    setFrustumColor(const cmm::FloatColor &clr) = 0;
    virtual const   cmm::FloatColor getFrustumColor(void) const = 0;

    virtual void    showFrustum(bool bShow) = 0;
    virtual bool    isFrustumShown(void) const = 0;

    virtual void    setForcePrivatePitch(bool bForce) = 0;
    virtual bool    isForcePrivatePitch(void) const = 0;
    virtual void    setPrivatePitch(double dblPitch) = 0;
    virtual double  getPrivatePitch(void) const = 0;

    virtual void    setForcePrivateAzimuth(bool bForce) = 0;
    virtual bool    isForcePrivateAzimuth(void) const = 0;
    virtual void    setPrivateAzimuth(double dblAzimuth) = 0;
    virtual double  getPrivateAzimuth(void) const = 0;
};

#endif