#ifndef LINE_PARAMETER_NODE_H_E3F54234_7BC8_4027_82BE_A86E33307C9B_INCLUDE
#define LINE_PARAMETER_NODE_H_E3F54234_7BC8_4027_82BE_A86E33307C9B_INCLUDE

#include "ParameterNode.h"

class LineParameterNode : public ParameterNode 
{
public:
    explicit LineParameterNode(param::IParameter *pParameter);
    virtual ~LineParameterNode(void);

public:
    const std::vector<osg::ref_ptr<osg::Vec3dArray> > &getPoints() { return m_vecPoints; }
    bool isMagnet() { return m_bMagnet; }
    void setRectifiedNode(osg::Node *pNode)
    {
        m_bHasIntered   = true;
        m_pRectifiedNode = pNode;
    }
    osg::Node *createNodeByParameter(const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;

protected:
    virtual bool initFromParameter();
    virtual void addChildByDetail(osg::LOD *pLOD, const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;

    virtual void traverse(osg::NodeVisitor& nv);

protected:
    osg::Node *createLineDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
    osg::Node *createCylinderDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
    osg::Node *createCubeDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
    osg::Node *createArrowDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;

    osg::Quat  findCubePipeRotation(const osg::Vec3dArray *pPipeVertices, unsigned nPos) const;
protected:
    std::vector<osg::ref_ptr<osg::Vec3dArray> >         m_vecPoints;
    bool                                                m_bMagnet;
    osg::ref_ptr<osg::Node>                             m_pRectifiedNode;
};

#endif
