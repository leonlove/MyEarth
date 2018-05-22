#ifndef FACE_PARAMETER_NODE_H_9133400C_62FF_40D2_B068_BFF7775376A5_INCLUDE
#define FACE_PARAMETER_NODE_H_9133400C_62FF_40D2_B068_BFF7775376A5_INCLUDE

#include "ParameterNode.h"

class FaceParameterNode : public ParameterNode
{
public:
    explicit FaceParameterNode(param::IParameter *pParameter);
    virtual ~FaceParameterNode(void);

protected:
    virtual bool initFromParameter();
    osg::Node *createNodeByParameter(const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
    void addChildByDetail(osg::LOD *pLOD, const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
    osg::Node *createFaceDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const;
protected:
    std::vector<osg::ref_ptr<osg::Vec3dArray> >         m_vecPoints;
};

#endif
