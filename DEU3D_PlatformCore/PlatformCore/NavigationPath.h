#ifndef NAVIGATION_PATH_H_4C7D218B_BB04_496B_8C3A_26E8EB857E50_INCLUDE
#define NAVIGATION_PATH_H_4C7D218B_BB04_496B_8C3A_26E8EB857E50_INCLUDE

#include "INavigationPath.h"

///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：漫游路径关键帧       
//
///////////////////////////////////////////////////////////
class NavigationKeyframe : public INavigationKeyframe
{
public:
    NavigationKeyframe(void);
    NavigationKeyframe(const NavigationKeyframe &param);
    virtual ~NavigationKeyframe();

          //camera pose
    virtual const CameraPose &getCameraPose(void) const;
    virtual CameraPose &getCameraPose(void);
    virtual void setCameraPose(const CameraPose &pose);

      //translation speed or duration time, if speed, 'meter/sec' unit, else 'sec' unit
    virtual double getTrans_TimeOrSpeed(); 
    virtual void setTrans_TimeOrSpeed(double dTrans_TimeOrSpeed);

      //rotation speed or duration time, if speed, 'degree/sec' unit, else 'sec' unit
    virtual double getRotate_TimeOrSpeed(); 
    virtual void setRotate_TimeOrSpeed(double dRotate_TimeOrSpeed);
      
    //the two arguments above used for duration time or speed ?
    virtual bool getArgForTime();
    virtual void setArgForTime(bool bArgForTime);

    
    virtual const std::string &getUserDefinitionData() const;
    virtual void setUserDefinitionData(const std::string &strUserDefinitionData);

    virtual const std::string &getName(void) const;
    virtual void  setName(const std::string &strName);

    virtual const cmm::image::IDEUImage *getThumbnail(void) const;
    virtual cmm::image::IDEUImage *getThumbnail(void);
    virtual void setThumbnail(cmm::image::IDEUImage *pImage);

public:
    bool fromBson(bson::bsonElement &val);
    bool toBson(bson::bsonElement &val) const;

private:
    bool toBsonCamera(bson::bsonElement &val) const;
    bool fromBsonCamera(bson::bsonElement &val);
public:
    CameraPose      m_CameraPose;                // camera pose

    double          m_dblTrans_TimeOrSpeed;        // translation speed or duration time, if speed, 'meter/sec' unit, else 'sec' unit
    double          m_dblRotate_TimeOrSpeed;    // rotation speed or duration time, if speed, 'degree/sec' unit, else 'sec' unit
    bool            m_bArgForTime;                // the two arguments above used for duration time or speed ?
    std::string     m_strName;
    std::string     m_strUserDefinitionData;
    OpenSP::sp<cmm::image::IDEUImage> m_pThumbnail;
};

///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：漫游路径       
//
///////////////////////////////////////////////////////////
typedef std::vector<OpenSP::sp<NavigationKeyframe>> ListNavigationKeyframe;
class NavigationPath : public INavigationPath
{
public:
    NavigationPath();
        
    NavigationPath(const NavigationPath &param);
    
    virtual ~NavigationPath();

    virtual void replaceItem(unsigned int nIndex, INavigationKeyframe * pKeyframe);
    
    virtual const INavigationKeyframe * getItem(unsigned int nIndex) const;
    virtual INavigationKeyframe       * getItem(unsigned int nIndex);

    virtual void deleteItem(unsigned int nIndex);
    
    virtual void appendItem(INavigationKeyframe * pKeyframe);   

	virtual void appendItemByCameraPos(const CameraPose &pose, double dLongitude, double dLatitude, double dHeight);

    virtual void insertItem(unsigned int nIndex, INavigationKeyframe * pKeyframe);

    virtual unsigned int getItemCount() const;

    virtual void clear();

    virtual const std::string &getPathName() const;

    virtual void setPathName(const std::string &strName);

public:
    bool fromBson(bson::bsonElement &val);
    bool toBson(bson::bsonElement &val) const;

private:
	double getDistance(double lat1, double long1, double lat2, double long2);
	void Region(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance);
	void Province(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance);
	void Country(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance);

private:
    std::string                            m_strPathName;
    ListNavigationKeyframe    m_ListNavigationKeyframe;
};

///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-25
//  功能简介：漫游路径容器       
//
///////////////////////////////////////////////////////////
typedef std::vector<OpenSP::sp<NavigationPath> > ListNavigationPath;
class NavigationPathContainer : public INavigationPathContainer
{
public:
    NavigationPathContainer();
    
    virtual ~NavigationPathContainer();

    virtual void replaceItem(unsigned int nIndex, INavigationPath * pNavigationPath);
    
    virtual INavigationPath * getItem(unsigned int nIndex);
    virtual const INavigationPath * getItem(unsigned int nIndex) const;
    
    virtual void deleteItem(unsigned int nIndex);
    
    virtual void appendItem(INavigationPath * pNavigationPath);   

    virtual void insertItem(unsigned int nIndex, INavigationPath * pNavigationPath);

    virtual unsigned int getItemCount() const;

    virtual void clear();

    virtual bool saveToFile(const std::string &strFileName) const;

    virtual bool loadFromFile(const std::string &strFileName);

private:
    ListNavigationPath    m_ListNavigationPath;
};

#endif