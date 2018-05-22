#ifndef POINT_PARAMETER_NODE_H_63EF7935_1B88_4E8D_B8E2_9506413E10A4_INCLUDE
#define POINT_PARAMETER_NODE_H_63EF7935_1B88_4E8D_B8E2_9506413E10A4_INCLUDE
//
//#include "ParameterNode.h"
//#include "HoleInfo.h"
//#include <osg/Geometry>
//#include <osg/Texture2D>
//
//class PointParameterNode : public ParameterNode
//{
//public:
//    explicit PointParameterNode(param::IParameter *pParameter);
//    virtual ~PointParameterNode(void);
//
//public:
//
//    const osg::Vec3d &getPosition()   { return m_ptPosition; }
//    void setInterPosition(const osg::Vec3d &InterPosition) 
//    {
//        m_bHasIntered   = true;
//        m_InterPosition = InterPosition;
//    }
//
//protected:
//    virtual bool initFromParameter();
//    virtual osg::Node *createNodeByParameter();
//    virtual void addChildByDetail(osg::LOD *pLOD, param::IDetail *pDetail);
//
//    virtual void traverse(osg::NodeVisitor& nv);
//
//    bool IHole2HoleInfo(const param::IHole *pHole, double dblOffset, HoleInfo &hole) const;
//
//protected:
//    osg::Node *createPrismDetail(const param::IDetail *pDetail) const;
//    osg::Node *createPointDetail(const param::IDetail *pDetail) const;
//    osg::Node *createCubeDetail(const param::IDetail *pDetail) const;
//    osg::Node *createCylinderDetail(const param::IDetail *pDetail) const;
//    osg::Node *createSphereDetail(const param::IDetail *pDetail) const;
//    osg::Node *createRoundTableDetail(const param::IDetail *pDetail) const;
//    osg::Node *createPipeConnectorDetail(const param::IDetail *pDetail) const;
//    osg::Node *createImageDetail(const param::IDetail *pDetail) const;
//    osg::Node *createSectorDetail(const param::IDetail *pDetail) const;
//    osg::Node *createPyramidDetail(const param::IDetail *pDetail) const;
//    osg::Node *createBubbleTextDetail(const param::IDetail *pDetail) const;
//    osg::Node *createPolygonDetail(const param::IDetail *pDetail) const;
//
//protected:
//    osg::Vec3d                  m_ptPosition;
//    bool                        m_bTransedOntoGlobe;
//    double                      m_dblPitchAngle;
//    double                      m_dblAzimuthAngle;
//    osg::Vec3d                  m_InterPosition;
//};

#endif
