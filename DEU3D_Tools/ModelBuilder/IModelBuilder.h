#ifndef I_MODEL_BUILDER_H_EBC7E389_D8D3_433C_908B_0B37829DDC8C_INCLUDE
#define I_MODEL_BUILDER_H_EBC7E389_D8D3_433C_908B_0B37829DDC8C_INCLUDE

#include "Export.h"
#include <string>
#include <list>
#include <map>
#include <Common\Common.h>
#include <OpenSP\Ref.h>
#include <ParameterSys\IParameter.h>
#include <EventAdapter\IEventAdapter.h>
#include <Common\DEUException.h>
#include <ParameterSys\IDetail.h>

struct SpatialRefInfo
{
    //东偏
    double m_dEastOffset;

    //北偏
    double m_dNorthOffset;

    //中央经线
    double m_dCentreLongitude;

    //坐标系
    std::string m_strCoordination;

    //投影
    std::string m_strProjection;
};

struct LineStyle
{
    double              min_visible;
    double              max_visible;
    std::string         name;
    double              width; 
    cmm::FloatColor     color;
    std::string         img_file;
};

struct FaceStyle
{
    double              min_visible;
    double              max_visible;
    double              borderwidth;
    cmm::FloatColor     facecolor;
    cmm::FloatColor     bordercolor;
    bool                bordervisible;
};

//typedef void (*LogNotifyFunc)(const char *info, void* param);
//typedef void (*ProgressNotifyFunc)(unsigned progress, unsigned total, void* param);

class IModelBuilder:public OpenSP::Ref
{
public:
    virtual bool initialize(const std::string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *pEventAdapter, OpenSP::sp<cmm::IDEUException> e = NULL) = 0;
    virtual bool setProjectionInfo(const std::string &strCoordSys, const std::string &strProj, double dEastOffset, double dNorthOffset, double dCentreLongitude) = 0;
    virtual bool setOffset(double dx, double dy, double dz)                                                 = 0;
    virtual void setShareModel(bool bShare)                                                                 = 0;
    virtual void setUseMultiRefPoint(bool bUse)                                                             = 0;
    virtual bool isUseMultiRefPoint(void) const                                                             = 0;
    virtual void buildIveModel(const std::string &strFile, OpenSP::sp<cmm::IDEUException> e = NULL)         = 0;
    virtual void buildParamModel(param::IParameter *pParameter,  OpenSP::sp<cmm::IDEUException> e = NULL,const std::map<std::string,std::string>& attrMap = std::map<std::string,std::string>())   = 0;
    virtual bool buildDetail(param::IDetail *detail,  OpenSP::sp<cmm::IDEUException> e = NULL)              = 0;
    virtual bool buildImage(const std::string &strImageFile, ID &idImage)                                   = 0;
    virtual bool buildModel(const std::string &strIveFile, ID &idModel)                                     = 0;
    virtual param::IParameter* getParameterByID(const ID id)                                                = 0;
    virtual bool updateParameter(param::IParameter* pIParameter)                                            = 0;
    virtual void writeConfigFile(OpenSP::sp<cmm::IDEUException> e = NULL)                                   = 0;
    virtual unsigned int getDatasetCode()                                                                   = 0;

    virtual bool shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin) = 0;
    virtual bool moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist)  = 0;

    virtual std::vector<std::string> getLodConfigParams()                                                   = 0;
    virtual bool addLodConfigParam(const std::string& strLodConfigParam)                                    = 0;
    virtual void setSelectedLodName(const std::string& strSelectedLodName)                                  = 0;
    virtual std::string getLodConfigParamByName(const std::string& strLodConfigName)                        = 0;
    virtual void stop()                                                                                     = 0;

    virtual bool deleteLodConfigParam(const std::string& strLodConfigName)                                  = 0;
};

MB_EXPORTS IModelBuilder* createModelBuilder(void);

#endif
