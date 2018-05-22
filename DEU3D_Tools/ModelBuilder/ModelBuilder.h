#ifndef MODEL_BUILDER_H_6F5EC6B0_BF88_45D6_ACEB_259554258D90_INCLUDE
#define MODEL_BUILDER_H_6F5EC6B0_BF88_45D6_ACEB_259554258D90_INCLUDE

#include "IModelBuilder.h"
#include <osgDB\FileNameUtils>
#include <osgDB\ReadFile>
#include <osgDB\ReaderWriter>
#include <osg\CoordinateSystemNode>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "DYCanvasList.h"

//LOD分级
typedef struct LODSegment
{
    double dLOD1;// 0    - 50米
    double dLOD2;// 50   - 200米
    double dLOD3;// 200  - 800米
    double dLOD4;// 800  - 2000米
    double dLOD5;// 2000 - 20000米

    int    nValidLodLevels;
    bool   bOptimize;

    LODSegment()
    {
        nValidLodLevels = 4;
        bOptimize       = true;
    }
}LODSEGMENT;

class GeodeVisitor;
class ModelBuilder:public IModelBuilder
{
public:
    ModelBuilder();
    ~ModelBuilder();

public:
    virtual bool initialize(const std::string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *pEventAdapter, OpenSP::sp<cmm::IDEUException> e = NULL);
    virtual bool setProjectionInfo(const std::string &strCoordSys, const std::string &strProj, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    virtual bool setOffset(double dx, double dy, double dz);
    virtual void setShareModel(bool bShare){ m_bShareModel = bShare; }
    virtual void buildIveModel(const std::string &strFile, OpenSP::sp<cmm::IDEUException> e = NULL);
    virtual void buildParamModel(param::IParameter *pParameter, OpenSP::sp<cmm::IDEUException> e = NULL,const std::map<std::string,std::string>& attrMap = std::map<std::string,std::string>());
    virtual bool buildDetail(param::IDetail *detail, OpenSP::sp<cmm::IDEUException> e = NULL);
    virtual bool buildImage(const std::string &strImageFile, ID &idImage);
    virtual bool buildModel(const std::string &strIveFile, ID &idModel);
    virtual param::IParameter* getParameterByID(const ID id);
    virtual bool updateParameter(param::IParameter* pIParameter);
    virtual void writeConfigFile(OpenSP::sp<cmm::IDEUException> e = NULL);
    virtual unsigned int getDatasetCode(){ return m_datasetCode; }
    virtual void setUseMultiRefPoint(bool bUse) { m_bUseMultiRefPoint = bUse;   }
    virtual bool isUseMultiRefPoint(void) const { return m_bUseMultiRefPoint;   }
    virtual std::vector<std::string> getLodConfigParams();
    virtual bool addLodConfigParam(const std::string& strLodConfigParam);
    virtual void setSelectedLodName(const std::string& strSelectedLodName);
    virtual std::string getLodConfigParamByName(const std::string& strLodConfigName);
    virtual void stop();
    virtual bool deleteLodConfigParam(const std::string& strLodConfigName);

public:
    unsigned findLowerNumber(unsigned n) const;
    void writeLog(const char *pInfo);
    void updateProgress();
    bool writeToDB(param::IParameter *pParameter, deudbProxy::IDEUDBProxy *db,const std::map<std::string,std::string>& attrMap = std::map<std::string,std::string>());
    LODSEGMENT getLodSegment();
    bool getStopFlag(){ return m_bStop; }
    std::vector<ID>& getVecModelID();
    std::vector<osg::BoundingSphere>& getVecBoundingSphere();

    const string& getTempFilePath();
    const DYFileToMat3x3& getFileToMat3x3();
    const DYFileToID& getFileToID();

private:
    virtual bool shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin);
    virtual bool moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist);

    void initLodConfigParam();
    bool readLodConfigFile(const string& strLodConfigFileName);
    bool writeLodConfigFile(const string& strLodConfigFileName);
    vector<double> getDistanceVec(const std::string& strLodConfigParam);

    void DelTempTextureFolder(const string& strDirPath, bool bRemoveDirFlag);
    void CountSpliceShareImg(string strImagePath, osg::ref_ptr<GeodeVisitor>& pGeodeVisitor);
    void RemoveTempTexture(bool bRemoveDirFlag);

protected:
    bool                m_bInitialized;
    bool                m_bShareModel;
    bool                m_bUseMultiRefPoint;
    std::string         m_strCoordination;
    std::string         m_strProjection;
    double              m_dEastOffset;
    double              m_dNorthOffset;
    double              m_dCentreLongitude;
    unsigned short      m_datasetCode;
    cmm::math::Point3d  m_offset;
    std::string         m_strTargetDB;

    OpenSP::sp<ea::IEventAdapter>       m_pEventAdapter;
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pTargetDB;

    map<string, vector<double> >        m_mapLodNameAndDistance;
    string                              m_strSelectedLodName;
    LODSEGMENT                          m_LodSegment;
    bool                                m_bStop;
    std::vector<ID>                     m_models;
    std::vector<osg::BoundingSphere>    m_SphereVec;

private:
    unsigned int    m_nModeCount;
    unsigned int    m_nProgress;

    std::string		m_strTempFilePath;
    std::ofstream   m_of_file;
	DYFileToMat3x3	m_mapFileToMat3x3;
	DYFileToID		m_mapFileToID;
    
};

#endif
