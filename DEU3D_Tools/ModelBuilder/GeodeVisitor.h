#ifndef _GEODEVISITOR_H
#define _GEODEVISITOR_H

#include <osg\Geode>
#include <IDProvider\ID.h>
#include <IDProvider\Definer.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include "ModelBuilder.h"
#include "DYCanvasList.h"

//该结构用于纹理被多个Geode和多级LOD共享的情况
typedef struct ShareImg
{
    ID idLOD1;//第一级LOD共享纹理图片名
    ID idLOD2;//第二级LOD共享纹理图片名
    ID idLOD3;//第三级LOD共享纹理图片名
    ID idLOD4;//第四级LOD共享纹理图片名
    ID idLOD5;//第五级LOD共享纹理图片名
    std::set<int> setLODIndex;  //已经处理到第nLOD级图片
}SHAREIMG;

typedef struct ShareGeode
{
    ID idGeode;
    ID idDetail;
}SHAREGEODE;

typedef std::map<std::string, SHAREIMG> ShareImgMap;
typedef std::vector<SHAREGEODE> ShareGeodeVec;

class GeodeVisitor : public osg::NodeVisitor
{
public:
    explicit GeodeVisitor(ModelBuilder* pModelBuilder);
    ~GeodeVisitor();

public:
    void setTargetDB(deudbProxy::IDEUDBProxy *pDB);
    void setDataSetCode(int nDataSetCode);
    int  getDataSetCode(void) const;
    void setUseMultiRefPoint(bool bUse);
    void setOffset(const osg::Vec3d& offset);
    void getOffset(osg::Vec3d& offset);
    void setSpatialRefInfo(const SpatialRefInfo& sri);
    void getSpatialRefInfo(SpatialRefInfo& sri);
    void setShareModel(bool bShareModel);
    bool getShareModel() const;

    void setGetNum(bool bGetNum);
    int  getGeodesCount();
    void countShareImg(const osg::Geode& node);
    bool isShareImg(const std::string& fileName);
    bool isShareGeode();

    void setLODSegment(LODSEGMENT& LODSeg);
    void getLODSegment(LODSEGMENT& LODSeg);
    bool getShareGeode(const int nLODIndex, SHAREGEODE& node);
    void addShareGeode(const int nLODIndex, const SHAREGEODE &node);
    bool getShareImg(const int nLODIndex, const std::string& strName, ID& idLOD);
    void addShareImg(const std::string& strName, const int nLODIndex, const ID& idLOD);

    void addModel(const ID& modelId, const osg::BoundingSphere& sphere);
    void writeConfigurationFile(const std::string& fileName);

    virtual void apply(osg::Geode &node);

    ModelBuilder* getModelBuilder() { return m_pModelBuilder; }

    double getMaxScale() { return m_dMaxScale; }

    void addSpliceShareImg(const string& strImageFileName);
    bool getMultiPathList(){ return m_bIsMultiPathList; }

private:
    string ExportOsgImage(osg::Image* image, bool bMultiTextureFlag);

private:
    int             m_nDataSetCode;
    int             m_nGeodesCount;
    bool            m_bGetNum;
    bool            m_bShareModel;
    bool            m_bShareGeode;
    bool            m_bMultiRefPoint;
    bool            m_bIsMultiPathList;
    double          m_dMaxScale;
    LODSEGMENT      m_LodSegment;
    ShareGeodeVec   m_ShareGeodes;
    ShareImgMap     m_ShareImgMap;

    osg::Vec3d      m_offset;
    SpatialRefInfo  m_sriInfo;

    std::vector<ID> m_models;
    std::vector<osg::BoundingSphere> m_SphereVec;
    std::map<const std::string, int> m_ImgRefCount;

    std::set<osg::Geode*>               m_setFinishGeode;
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pTargetDB;

    ModelBuilder* m_pModelBuilder;

};

#endif

//情况一：一个Geode被多个NodePath引用，并且每个Geode内的纹理图片不被共享
//        这种情况所有的Img都是内嵌在Geode内部存储，此时只需要考虑LOD分级新产生的Geode即可
//        数据结构std::vector<SHAREGEODE> m_ShareGeodes，用于存放这一批NodePath可能产生的多级LOD，以用来被共享使用
//         ------------            ------------            ------------
//        |  NodePath  |          |  NodePath  |          |  NodePath  |
//         ------------            ------------            ------------
//               \                      |                       /
//                 \                    |                     /
//                   \                  |                   /
//                     \                |                 /
//                       \              |               /
//                         \            |             /
//                            ---------------------
//                           |  Geode1  |   Img1   |  一级LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode2  |   Img2   |  二级LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode3  |   Img3   |  三级LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode4  |   Img4   |  四级LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode5  |   Img5   |  五级LOD
//                            ---------------------


//情况二：一个Geode被多个NodePath引用，并且每个Geode内的纹理图片被另一个Geode共享
//        这种情况所有的Img都是以文件名的形式在Geode内部存储，此时不仅需要考虑LOD分级新产生的Geode，还要考虑纹理图片被共享的情况
//        数据结构std::vector<SHAREGEODE> m_ShareGeodes，用于存放这一批NodePath可能产生的多级LOD，以用来被共享使用
//        数据结构std::map<std::string, SHAREIMG> m_ShareImgMap，用于存放被共享的纹理图片在多级LOD中的对应关系，以用来被其他Geode使用
//         ------------           ------------            ------------          ------------           ------------          ------------
//        |  NodePath  |         |  NodePath  |          |  NodePath  |        |  NodePath  |         |  NodePath  |        |  NodePath  |
//         ------------           ------------            ------------          ------------           ------------          ------------
//               \                     |                      /                       \                     |                    /
//                 \                   |                    /                           \                   |                  /
//                   \                 |                  /                               \                 |                /
//                     \               |                /                                   \               |              /
//                       \             |              /                                       \             |            /
//                         \           |            /                                           \           |          /
//                            --------------------                   ----------                   --------------------
//                           | GeodeA1 | ImgName1 |   -----------   |   Img1   |   -----------   | ImgName1 | GeodeB1 |  一级LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA2 | ImgName2 |   -----------   |   Img2   |   -----------   | ImgName2 | GeodeB2 |  二级LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA3 | ImgName3 |   -----------   |   Img3   |   -----------   | ImgName3 | GeodeB3 |  三级LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA4 | ImgName4 |   -----------   |   Img4   |   -----------   | ImgName4 | GeodeB4 |  四级LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA5 | ImgName5 |   -----------   |   Img5   |   -----------   | ImgName5 | GeodeB5 |  五级LOD
//                            --------------------                   ----------                   --------------------
