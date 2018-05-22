#ifndef COMMON_MODEL_CREATER_H_C06E4523_D5D6_472C_8A60_04A4F62386EE_INCLUDE
#define COMMON_MODEL_CREATER_H_C06E4523_D5D6_472C_8A60_04A4F62386EE_INCLUDE

#include "Export"

#include <osg/Node>
#include <osg/CullFace>
#include <osg/Geometry>

namespace osgUtil
{

class OSGUTIL_EXPORT CommonModelCreater : public osg::Referenced
{
public:
    explicit CommonModelCreater();
    ~CommonModelCreater();
public:
    enum ModleType
    {
        CYLINDER    = 0,
        CUBE,
        CONE,
        SPHERE,
        RECT,
        Circle,
    };

    enum CoverType
    {
        NONE    = 0,
        TOP,
        BOTTOM,
        ALL
    };

    static CommonModelCreater *instance(void);

    //创建标准模型
    void createStandardModel(ModleType eModleType, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor, osg::ref_ptr<osg::Node> &pReturnNode);

    //创建上下底带洞的圆柱体
    osg::Node *createCylinderWithHole(float fRadius, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor);
    //创建上下底带洞的长方体
    osg::Node *createCubeWithHole(float fLength, float fWidth, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor);
    //创建上下底带洞的圆台
    osg::Node *createRoundTableWithHole(float fTopRadius, float fBottomRadius, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor);
    //创建上下底带洞的棱柱
    osg::Node *createPrismWithHole(const osg::Vec3Array *pVertex, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor);

    osg::Node *createSector(float fBeginAngle, float fEndAngle, float fRadius1, float fRadius2, bool bTexStretched, const osg::Vec4 &vFaceColor, const osg::Vec4 &vBorderColor, float fBorderWidth);
    //创建圆台
    osg::Node *createRoundTable(float fTopRadius, float fBottomRadius, float fHeight, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor);
    //创建棱柱
    osg::Node *createPrism(const osg::Vec3Array *pVertex, float fHeight, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor);

    //创建多边形
    osg::Geometry *createPolygon(const osg::Vec3Array *pVertex, bool bTexStretched, const osg::Vec4 &vColor);
    //创建线
    osg::Geometry *createLine(const osg::Vec3Array *pVertex, osg::PrimitiveSet::Mode eMode, float fLineWidth, bool bTexStretched, const osg::Vec4 &vColor);
    //创建点
    osg::Geometry *createPoint(const osg::Vec3 &point, float fPointSize, const osg::Vec4 &vColor);

protected:
    osg::Vec3Array *createCycleVertex(const osg::Vec3 &vCenter, float fRadius, unsigned int nSegments);
    osg::Vec3Array *createCycleNormal(float fTopAngle, unsigned int nSegments);

    //创建标准圆柱体
    osg::Node *createCylinder(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor);
    //创建标准长方体
    osg::Node *createCube(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor);
    //创建标准圆锥
    osg::Node *createCone(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor);
    //创建标准球体
    osg::Node *createSphere(bool bTexStretched, const osg::Vec4 &vColor);


protected:
    osg::Vec2Array *createTexCoordByVertex(const osg::Vec3Array *pVertexArray, bool bTexStretched);
    osg::Geometry *CommonModelCreater::createSheetWithHoles(const std::vector<osg::Vec3> &vecVertices, const std::vector<std::vector<osg::Vec3> > &vecHoleVertices, bool bTexStretched, const osg::Vec4 &vColor);

protected:
    class StandardModelTag
    {
    public:
        explicit StandardModelTag(const osg::Vec4 &vClr, unsigned int nTag) : m_vClr(vClr), m_nTag(nTag) {}
    public:
        inline const StandardModelTag   &operator =(const StandardModelTag &tag)
        {
            if(this == &tag)  return *this;
            m_vClr = tag.m_vClr;
            m_nTag  = tag.m_nTag;
            return *this;
        }
        inline bool                     operator==(const StandardModelTag &tag) const
        {
            if(m_vClr != tag.m_vClr)  return false;
            if(m_nTag  != tag.m_nTag)   return false;
            return true;
        }
        inline bool                     operator< (const StandardModelTag &tag) const
        {
            if(m_vClr < tag.m_vClr)       return true;
            else if(m_vClr != tag.m_vClr)  return false;

            if(m_nTag < tag.m_nTag)         return true;
            else if(m_nTag > tag.m_nTag)    return false;

            return false;
        }

    public:
        osg::Vec4 m_vClr;
        unsigned int m_nTag;
    };

    class StateSetTag
    {
    public:
        explicit StateSetTag(const float fSize, unsigned char nTag) : m_fize(fSize), m_nTag(nTag) {}
    public:
        inline const StateSetTag   &operator =(const StateSetTag &tag)
        {
            if(this == &tag)  return *this;
            m_fize = tag.m_fize;
            m_nTag  = tag.m_nTag;
            return *this;
        }
        inline bool                     operator==(const StateSetTag &tag) const
        {
            if(m_fize != tag.m_fize)  return false;
            if(m_nTag  != tag.m_nTag)   return false;
            return true;
        }
        inline bool                     operator< (const StateSetTag &tag) const
        {
            if(m_fize < tag.m_fize)       return true;
            else if(m_fize >= tag.m_fize)  return false;

            if(m_nTag < tag.m_nTag)         return true;
            else if(m_nTag > tag.m_nTag)    return false;

            return false;
        }

    public:
        float m_fize;
        unsigned char m_nTag;
    };

protected:
    void getColorArray(const osg::Vec4 &clr, osg::ref_ptr<osg::Vec4Array> &pColorArray);

protected:
    //共享的标准模型体
    OpenThreads::Mutex m_mtxStandardModel;
    std::map<StandardModelTag, osg::observer_ptr<osg::Node> > m_mapStandardModel;

    //共享的颜色数组
    OpenThreads::Mutex m_mtxColorArray;
    std::map<osg::Vec4, osg::observer_ptr<osg::Vec4Array> > m_mapColorArray;

    osg::ref_ptr<osg::StateSet>             m_pNormalState;
    osg::ref_ptr<osg::StateSet>             m_pNoNormalState;
    osg::ref_ptr<osg::StateSet>             m_pNormalAndAlphaState;
    osg::ref_ptr<osg::StateSet>             m_pNoNormalAndAlphaState;
};

}

#endif