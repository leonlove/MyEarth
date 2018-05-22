#include "TextCenterLayouter.h"
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osgViewer/View>
#include <IDProvider/Definer.h>

TextCenterLayouter::TextCenterLayouter(void)
{
    m_pElementFinder = new ElementFinder;
    m_nUpdateSpeed = 5u;
}


TextCenterLayouter::~TextCenterLayouter(void)
{
}


void TextCenterLayouter::ElementFinder::apply(osg::Geode &node)
{
    const unsigned nDrawableCount = node.getNumDrawables();
    for(unsigned n = 0u; n < nDrawableCount; n++)
    {
        osg::Drawable *pDrawable = node.getDrawable(n);

        osg::Geometry *pGeometry = dynamic_cast<osg::Geometry *>(pDrawable);
        if(pGeometry != NULL)
        {
            const osg::Array *pArray = pGeometry->getVertexArray();
            if(!pArray) continue;

            const osg::Vec3Array  *pVec3Array  = dynamic_cast<const osg::Vec3Array *>(pArray);
            const osg::Vec3dArray *pVec3dArray = dynamic_cast<const osg::Vec3dArray *>(pArray);
            if(pVec3Array != NULL || pVec3dArray != NULL)
            {
                m_vecVertexInfo.resize(m_vecVertexInfo.size() + 1u);

                VertexArrayInfo &infoArray = m_vecVertexInfo.back();
                infoArray.m_pVertexArray = pArray;
                infoArray.m_NodePath = getNodePath();

                continue;
            }

            continue;
        }

        osgText::Text *pText = dynamic_cast<osgText::Text *>(pDrawable);
        if(pText)
        {
            m_vecTextObjects.push_back(pText);
            continue;
        }
    }
}


void TextCenterLayouter::gatherMatricesOfNodePath(const osg::NodePath &nodePath, osg::Matrixd &mtxTransform) const
{
    osg::NodePath::const_reverse_iterator itorNode = nodePath.rbegin();
    for( ; itorNode != nodePath.rend(); ++itorNode)
    {
        const osg::Node *pNodeInPath = *itorNode;
        const osg::MatrixTransform *pMtxTrans = dynamic_cast<const osg::MatrixTransform *>(pNodeInPath);
        if(!pMtxTrans)  continue;

        mtxTransform *= pMtxTrans->getMatrix();
    }
}


void TextCenterLayouter::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    const osg::NodeVisitor::VisitorType eVisitorType = pNodeVisitor->getVisitorType();
    const unsigned nFrameNumber = pNodeVisitor->getFrameStamp()->getFrameNumber();
    if(nFrameNumber % m_nUpdateSpeed)
    {
        if(eVisitorType == osg::NodeVisitor::UPDATE_VISITOR)
        {
            osg::NodeCallback::traverse(pNode, pNodeVisitor);
        }
        return;
    }

    if(eVisitorType == osg::NodeVisitor::EVENT_VISITOR)
    {
        osgGA::GUIEventHandler::operator()(pNode, pNodeVisitor);
    }
    else if(eVisitorType == osg::NodeVisitor::UPDATE_VISITOR)
    {
        doUpdateTraverse(pNode, pNodeVisitor);
    }
    //else
    //{
    //    osgGA::GUIEventHandler::operator()(pNode, pNodeVisitor);
    //}
}


void TextCenterLayouter::doUpdateTraverse(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    const ID &id = pNode->getID();
    if(id.ObjectID.m_nType != PARAM_LINE_ID && id.ObjectID.m_nType != PARAM_FACE_ID)
    {
        osg::NodeCallback::traverse(pNode, pNodeVisitor);
        return;
    }

    m_pElementFinder->clear();
    osg::Group *pNodeGroup = dynamic_cast<osg::Group *>(pNode);
    if(pNodeGroup)
    {
        const unsigned nChildrenCount = pNodeGroup->getNumChildren();
        for(unsigned n = 0u; n < nChildrenCount; n++)
        {
            osg::Node *pChildNode = pNodeGroup->getChild(n);
            pChildNode->accept(*m_pElementFinder);
        }
    }
    else
    {
        pNode->accept(*m_pElementFinder);
    }

    const std::vector<VertexArrayInfo>     &vecVertexInfo  = m_pElementFinder->getVertexInfo();
    std::vector<osgText::Text *>           &vecTextObjects = m_pElementFinder->getTextObjects();
    if(vecVertexInfo.empty() || vecTextObjects.empty())
    {
        osg::NodeCallback::traverse(pNode, pNodeVisitor);
        return;
    }

    const osg::NodePath &nodePath = pNodeVisitor->getNodePath();
    osg::Matrixd    mtxTransform;
    gatherMatricesOfNodePath(nodePath, mtxTransform);

    const osg::Vec3d ptMostSuitable = findMostSuitable(mtxTransform, vecVertexInfo);

    std::vector<osgText::Text *>::iterator itorText = vecTextObjects.begin();
    for( ; itorText != vecTextObjects.end(); ++itorText)
    {
        osgText::Text *pText = *itorText;
        pText->setPosition(ptMostSuitable);
    }

    osg::NodeCallback::traverse(pNode, pNodeVisitor);
}


osg::Vec3d TextCenterLayouter::findMostSuitable(const osg::Matrixd &mtxTransform, const std::vector<VertexArrayInfo> &vecVertexInfo) const
{
    double dblScore = -1.0;
    osg::Vec3d ptMostSuitable;
    std::vector<VertexArrayInfo>::const_iterator itorVtxInfo = vecVertexInfo.begin();
    for( ; itorVtxInfo != vecVertexInfo.end(); ++itorVtxInfo)
    {
        const VertexArrayInfo &infoVtx = *itorVtxInfo;

        osg::Matrixd matrix0;
        gatherMatricesOfNodePath(infoVtx.m_NodePath, matrix0);

        const osg::Matrixd matrix1 = mtxTransform * matrix0;

        double      dblCurScore = -1.0;
        osg::Vec3d  ptCurSuitable;

        const osg::Vec3Array *pVec3Array = dynamic_cast<const osg::Vec3Array *>(infoVtx.m_pVertexArray);
        if(pVec3Array)
        {
            dblCurScore = findMostSuitable(pVec3Array, matrix1, ptCurSuitable);
        }
        else
        {
            const osg::Vec3dArray *pVec3dArray = dynamic_cast<const osg::Vec3dArray *>(pVec3Array);
            if(pVec3dArray)
            {
                dblCurScore = findMostSuitable(pVec3dArray, matrix1, ptCurSuitable);
            }
        }

        if(dblCurScore > dblScore)
        {
            dblScore = dblCurScore;
            ptMostSuitable = ptCurSuitable;
        }
    }
    return ptMostSuitable;
}


double TextCenterLayouter::findMostSuitable(const osg::Vec3Array *pVtxArray, const osg::Matrixd &mtx, osg::Vec3d &ptSuitable) const
{
    double dblScore = -1.0;

    if(pVtxArray->empty())  return dblScore;

    osg::Vec3Array::const_iterator itorVtx = pVtxArray->begin();
    for( ; itorVtx != pVtxArray->end(); ++itorVtx)
    {
        const osg::Vec3 &vtx = *itorVtx;
        const double dblCurScore = getVertexScore(vtx, mtx);
        if(dblCurScore > dblScore)
        {
            ptSuitable = vtx;
            dblScore = dblCurScore;
        }
    }

    return dblScore;
}


double TextCenterLayouter::findMostSuitable(const osg::Vec3dArray *pVtxArray, const osg::Matrixd &mtx, osg::Vec3d &ptSuitable) const
{
    double dblScore = -1.0;

    if(pVtxArray->empty())  return dblScore;

    osg::Vec3dArray::const_iterator itorVtx = pVtxArray->begin();
    for( ; itorVtx != pVtxArray->end(); ++itorVtx)
    {
        const osg::Vec3 &vtx = *itorVtx;
        const double dblCurScore = getVertexScore(vtx, mtx);
        if(dblCurScore > dblScore)
        {
            ptSuitable = vtx;
            dblScore = dblCurScore;
        }
    }

    return dblScore;
}


double TextCenterLayouter::getVertexScore(const osg::Vec3d &vertex, const osg::Matrixd &mtx) const
{
    const osg::Vec3d point = vertex * mtx;

#if 0
    osg::Vec3d vecEye2Point = point - m_ptEyePosition;
    if(vecEye2Point.normalize() < FLT_EPSILON)
    {
        return -1.0;
    }

    const double dblScore = vecEye2Point * m_vecEyeDirection;
    return dblScore;
#endif
    return -1.0;
}
