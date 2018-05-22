#ifndef I_NAVIGATION_PATH_H_70939CA7_91B0_4736_8EE3_E05E6B2DEBFB_INCLUDE
#define I_NAVIGATION_PATH_H_70939CA7_91B0_4736_8EE3_E05E6B2DEBFB_INCLUDE

#include <OpenSP/Ref.h>
#include "NavigationParam.h"
#include <common/IDEUImage.h>
enum NavigationMode
{
    NM_PARALLEL_SCALE_ALIGNMENT = 0,    // translation and rotation start at same time, end at same time
    NM_PARALLEL_RIGHT_ALIGNMENT,        // translation and rotation end at same time
    NM_PARALLEL_LEFT_ALIGNMENT,            // translation and rotation start at same time
    NM_SERIAL_RT,                        // translation start at the time of rotation end
    NM_SERIAL_TR,                        // rotation start at the time of translation end
};


///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：NavigationKeyframe的抽象基类       
//
///////////////////////////////////////////////////////////
class INavigationKeyframe : virtual public OpenSP::Ref
{
public:
	  //camera pose
    virtual const CameraPose &getCameraPose(void) const = 0;
    virtual CameraPose &getCameraPose(void) = 0;
    virtual void setCameraPose(const CameraPose &pose) = 0;

    virtual const std::string &getName(void) const = 0;
    virtual void  setName(const std::string &strName) = 0;

	  //translation speed or duration time, if speed, 'meter/sec' unit, else 'sec' unit
    virtual double getTrans_TimeOrSpeed() = 0; 
	virtual void setTrans_TimeOrSpeed(double dTrans_TimeOrSpeed) = 0;

      //rotation speed or duration time, if speed, 'degree/sec' unit, else 'sec' unit
	virtual double getRotate_TimeOrSpeed() = 0; 
	virtual void setRotate_TimeOrSpeed(double dRotate_TimeOrSpeed) = 0;
	  
	//the two arguments above used for duration time or speed ?
	virtual bool getArgForTime() = 0;
    virtual void setArgForTime(bool bArgForTime) = 0;

	
    virtual const std::string &getUserDefinitionData() const = 0;
    virtual void setUserDefinitionData(const std::string &strUserDefinitionData) = 0;


    virtual const cmm::image::IDEUImage *getThumbnail(void) const = 0;
	virtual cmm::image::IDEUImage *getThumbnail(void) = 0;
	virtual void setThumbnail(cmm::image::IDEUImage *pImage) = 0;
};

///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：NavigationPath的抽象基类       
//
///////////////////////////////////////////////////////////
class INavigationPath : virtual public OpenSP::Ref
{
public:
    virtual void replaceItem(unsigned int nIndex, INavigationKeyframe * pKeyframe) = 0;
	
	virtual INavigationKeyframe * getItem(unsigned int nIndex) = 0;
	virtual const INavigationKeyframe * getItem(unsigned int nIndex) const = 0;
	
	virtual void deleteItem(unsigned int nIndex) = 0;
    
    virtual void appendItem(INavigationKeyframe * pKeyframe) = 0;   

	virtual void appendItemByCameraPos(const CameraPose &pose, double dLongitude, double dLatitude, double dHeight) = 0;

	virtual void insertItem(unsigned int nIndex, INavigationKeyframe * pKeyframe) = 0;

	virtual unsigned int getItemCount() const = 0;

    virtual void clear() = 0;

	virtual const std::string &getPathName() const = 0;

    virtual void setPathName(const std::string &strName) = 0;
};

///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：NavigationPathContainer的抽象基类       
//
///////////////////////////////////////////////////////////
class INavigationPathContainer : virtual public OpenSP::Ref
{
public:
    virtual void replaceItem(unsigned int nIndex, INavigationPath * pNavigationPath) = 0;
	
	virtual INavigationPath * getItem(unsigned int nIndex) = 0;
	virtual const INavigationPath * getItem(unsigned int nIndex) const = 0;

    virtual void deleteItem(unsigned int nIndex) = 0;
    
    virtual void appendItem(INavigationPath * pNavigationPath) = 0;   

	virtual void insertItem(unsigned int nIndex, INavigationPath * pNavigationPath) = 0;

	virtual unsigned int getItemCount(void) const = 0;

    virtual void clear() = 0;

	virtual bool saveToFile(const std::string &strFileName) const = 0;

	virtual bool loadFromFile(const std::string &strFileName) = 0;
};


#endif

