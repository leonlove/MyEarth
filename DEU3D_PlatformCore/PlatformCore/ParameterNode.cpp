#include "ParameterNode.h"

#include <ParameterSys/IDetail.h>
#include <IDProvider\Definer.h>
#include <Common/Common.h>
#include <osgDB/ReadFile>
#include <osg/SharedObjectPool>
#include <IDProvider/Definer.h>
#include "PointParameterNode.h"
#include "LineParameterNode.h"
#include "FaceParameterNode.h"

ParameterNode::ParameterNode(param::IParameter *pParameter) :
    m_pParameter(pParameter),
    m_bHasIntered(false),
    m_nLastSendTime(0)
{
}

ParameterNode::~ParameterNode(void)
{
}

bool ParameterNode::initFromParameter()
{
    if(m_pParameter == NULL)
    {
        return false;
    }
    m_bFollowTerrain    = m_pParameter->isFollowByTerrain();
    m_nActPro           = m_pParameter->getActProperty();
    m_dblHeight         = m_pParameter->getHeight();
    m_nOrder            = m_pParameter->getCoverOrder();
    return true;
}

ParameterNode *ParameterNode::createParameterNode(param::IParameter *pParameter)
{
    ID id = pParameter->getID();
    osg::ref_ptr<ParameterNode> pParameterNode = NULL;
    if(id.ModelID.m_nType == PARAM_POINT_ID)
    {
        osg::ref_ptr<PointParameterNode> pPointParameterNode = new PointParameterNode(pParameter);
        pParameterNode = pPointParameterNode;
    }
    else if(id.ModelID.m_nType == PARAM_LINE_ID)
    {
        osg::ref_ptr<LineParameterNode> pLineParameterNode = new LineParameterNode(pParameter);
        pParameterNode = pLineParameterNode;
    }
    else if(id.ModelID.m_nType == PARAM_FACE_ID)
    {
        osg::ref_ptr<FaceParameterNode> pFaceParameterNode = new FaceParameterNode(pParameter);
        pParameterNode = pFaceParameterNode;
    }

    pParameterNode->initFromParameter();
    return pParameterNode.release();
}


osg::Texture *ParameterNode::bindTexture(const ID &idImage, osg::StateSet *pStateSet)
{
    if(!idImage.isValid())  return NULL;
    if(idImage.ModelID.m_nType != SHARE_IMAGE_ID && idImage.ModelID.m_nType != IMAGE_ID)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(idImage);
    if(!pImage.valid())
    {
        return NULL;
    }

    osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
    osg::ref_ptr<osg::Texture> pTexture;
    if(!pPool->findObject(pImage.get(), pTexture))
    {
        //printf("未查找到影像--%s！, 类型为--%d\n", strImgName.c_str(), idImage.ModelID.m_nType);
        pTexture = new osg::Texture2D(pImage.get());
        pPool->addTexture(pImage.get(), pTexture);
    }

    pTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
    pTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);

    if(pStateSet)
    {
        pStateSet->setTextureAttributeAndModes(0u, pTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }

    return pTexture.release();
}

