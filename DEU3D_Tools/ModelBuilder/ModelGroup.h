#ifndef _MODELGROUP_H
#define _MODELGROUP_H

#include <osg/MatrixTransform>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <common/deuMath.h>
#include <osg/Geode>
#include "ModelBuilder.h"

class GeodeVisitor;
class DYVert;
class DYIntList;

typedef struct DetailInfo
{
    ID idDetail;
    double dMinRange;
    double dMaxRange;
}DETAILINFO;

class ModelGroup
{
public:
    ModelGroup(GeodeVisitor* pGeodeVisitor, deudbProxy::IDEUDBProxy *pDB, bool bMultiRefCenter);
    ~ModelGroup(void);

public:
    void setModel(osg::ref_ptr<osg::MatrixTransform> pMatrixTransform);
    bool saveModel();

private:
    bool saveGeodes();

    void saveGeodes_5level_source(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);
	void saveGeodes_1level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);
	void saveGeodes_2level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);
    void saveGeodes_3level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);
    void saveGeodes_4level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);
    //void saveGeodes_5level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg);

    bool changeGeode( osg::ref_ptr<osg::Geode>& pNewGeode );
    bool saveGeode(int nLODIndex, ID& geodeID, osg::Geode* pGeode, ID& idDetail);
    bool saveDetail(const ID& id, ID& idDetail);
    bool savePointParameter(const std::vector<cmm::math::Point3d> &vecBubbleCenters);

    osg::ref_ptr<osg::Image> saveImage(int nLODIndex, bool bShare, osg::ref_ptr<osg::Image>& pSrcImage);

    ID   getGeodID();
    int  getImageScaleS(int nLODIndex, osg::ref_ptr<osg::Image>& pSrcImage);
    int  getImageScaleT(int nLODIndex, osg::ref_ptr<osg::Image>& pSrcImage);
    bool isShareImage(osg::Geode* pGeode);
    void coordinateTransform(osg::Vec3d src, osg::Vec3d &dst) const;
	double getBoundRadius();
    double getBoundRadius(osg::Geode* pGeode);
    void generateReferencedCenter(const osg::Geode *pGeode, double dblMinRange, std::vector<cmm::math::Point3d> &vecBubbleCenters) const;

    void UseSpaliceTexture(osg::ref_ptr<osg::Geometry>& geometry);
    bool CreateLodFromGeode(osg::ref_ptr<osg::Geode>& pGeode, float fAlpha);
    bool AnalysisVertex(osg::ref_ptr<osg::Geometry> geometry, std::vector<DYVert>& vecDYVert);
    bool AnalysisVertIndex(osg::ref_ptr<osg::Geometry> geometry, DYIntList& indexList);
    bool ModifyGeometry(osg::ref_ptr<osg::Geometry>& geometry, const std::vector<DYVert>& vecDYVertOut, const std::vector<std::pair<int, DYIntList> >& vecModeAndIndexList);

    void ModifyTextureInfo(osg::ref_ptr<osg::Geometry>& geometry, const std::vector<DYVert> &vecDYVertOut);
    void RemoveTextureAttribute(osg::ref_ptr<osg::Geode> pGeode);
    bool MergeDrawable(osg::ref_ptr<osg::Geode>& pGeode);
    bool MergeDrawableForLastLevel(osg::ref_ptr<osg::Geode>& pGeode);
	bool VertexRender(osg::ref_ptr<osg::Drawable>& drawAble);

	void Merge(osg::ref_ptr<osg::Geode>& pGeode, osg::ref_ptr<osg::Geometry>& geometry, vector<osg::ref_ptr<osg::Drawable>>& vecDrawable);
    std::string getTextureName(const osg::ref_ptr<osg::Geometry>& geometry);
    bool UseSpaliceTextureForLevel1(osg::ref_ptr<osg::Geode>& pGeode);
    bool getUniqueVertexs( osg::ref_ptr<osg::Geometry>& geometry );

private:
    bool                    m_bMultiRefCenter;
    GeodeVisitor*           m_pGeodeVisitor;
    osg::MatrixTransform*   m_pMatrixTransform;
    std::vector<DETAILINFO> m_vecDetail;
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pTargetDB;

    cmm::math::Point3d      m_centerPoint;
    unsigned int            m_resolutionH;
    unsigned int            m_resolutionV;
    std::map<std::string, osg::ref_ptr<osg::Image> > m_mapSpliceImage;
    int                     m_nCurrentLODIndex;
    osg::Vec3d              m_ModelCenter;
    double                  m_ModelRadius;
};

#endif
