#ifndef PARAMETER_NODE_H_FCBEC075_87C0_4AE5_9DF9_5FC3D7BF179F_INCLUDE
#define PARAMETER_NODE_H_FCBEC075_87C0_4AE5_9DF9_5FC3D7BF179F_INCLUDE

#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>
#include <osg/CoordinateSystemNode>
#include <osg/Texture>
#include <ParameterSys/IParameter.h>
#include <ParameterSys/IDetail.h>

class ParameterNode : public osg::MatrixTransform
{
public:
    explicit ParameterNode(param::IParameter *pParameter);
    virtual ~ParameterNode(void);

public:
    static ParameterNode    *createParameterNode(param::IParameter *pParameter);
    bool                    hasIntered(void) { return m_bHasIntered; }

public:
    static osg::Texture *bindTexture(const ID &idImage, osg::StateSet *pStateSet);

protected:
    virtual bool initFromParameter();

protected:

protected:
    OpenSP::sp<param::IParameter>       m_pParameter;
    bool                                m_bFollowTerrain;
    clock_t                             m_nLastSendTime;
    int                                 m_nActPro;
    unsigned int                        m_nOrder;
    double                              m_dblHeight;
    //是否已经插值
    bool                                m_bHasIntered;
    OpenSP::sp<OpenSP::Ref>             m_pParmRectifyRequest;
};

#endif
