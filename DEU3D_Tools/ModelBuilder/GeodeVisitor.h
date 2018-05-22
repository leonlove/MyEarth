#ifndef _GEODEVISITOR_H
#define _GEODEVISITOR_H

#include <osg\Geode>
#include <IDProvider\ID.h>
#include <IDProvider\Definer.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include "ModelBuilder.h"
#include "DYCanvasList.h"

//�ýṹ�����������Geode�Ͷ༶LOD��������
typedef struct ShareImg
{
    ID idLOD1;//��һ��LOD��������ͼƬ��
    ID idLOD2;//�ڶ���LOD��������ͼƬ��
    ID idLOD3;//������LOD��������ͼƬ��
    ID idLOD4;//���ļ�LOD��������ͼƬ��
    ID idLOD5;//���弶LOD��������ͼƬ��
    std::set<int> setLODIndex;  //�Ѿ�������nLOD��ͼƬ
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

//���һ��һ��Geode�����NodePath���ã�����ÿ��Geode�ڵ�����ͼƬ��������
//        ����������е�Img������Ƕ��Geode�ڲ��洢����ʱֻ��Ҫ����LOD�ּ��²�����Geode����
//        ���ݽṹstd::vector<SHAREGEODE> m_ShareGeodes�����ڴ����һ��NodePath���ܲ����Ķ༶LOD��������������ʹ��
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
//                           |  Geode1  |   Img1   |  һ��LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode2  |   Img2   |  ����LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode3  |   Img3   |  ����LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode4  |   Img4   |  �ļ�LOD
//                            ---------------------
//                            ---------------------
//                           |  Geode5  |   Img5   |  �弶LOD
//                            ---------------------


//�������һ��Geode�����NodePath���ã�����ÿ��Geode�ڵ�����ͼƬ����һ��Geode����
//        ����������е�Img�������ļ�������ʽ��Geode�ڲ��洢����ʱ������Ҫ����LOD�ּ��²�����Geode����Ҫ��������ͼƬ����������
//        ���ݽṹstd::vector<SHAREGEODE> m_ShareGeodes�����ڴ����һ��NodePath���ܲ����Ķ༶LOD��������������ʹ��
//        ���ݽṹstd::map<std::string, SHAREIMG> m_ShareImgMap�����ڴ�ű����������ͼƬ�ڶ༶LOD�еĶ�Ӧ��ϵ��������������Geodeʹ��
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
//                           | GeodeA1 | ImgName1 |   -----------   |   Img1   |   -----------   | ImgName1 | GeodeB1 |  һ��LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA2 | ImgName2 |   -----------   |   Img2   |   -----------   | ImgName2 | GeodeB2 |  ����LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA3 | ImgName3 |   -----------   |   Img3   |   -----------   | ImgName3 | GeodeB3 |  ����LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA4 | ImgName4 |   -----------   |   Img4   |   -----------   | ImgName4 | GeodeB4 |  �ļ�LOD
//                            --------------------                   ----------                   --------------------
//                            --------------------                   ----------                   --------------------
//                           | GeodeA5 | ImgName5 |   -----------   |   Img5   |   -----------   | ImgName5 | GeodeB5 |  �弶LOD
//                            --------------------                   ----------                   --------------------
