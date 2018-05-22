#include "HudLayouter.h"
#include <osgUtil/CullVisitor>
#include <osg/CameraNode>
#include <osgViewer/View>

HudLayouter::HudLayouter(void)
{
    m_bHorzAbsolute = false;
    m_bVertAbsolute = false;
    m_bLayoutCamera = false;

    m_nRefreshSpeed = 5u;

    m_fltX = 0.0f;
    m_fltY = 0.0f;
    m_fltWidth = 1.0f;
    m_fltHeight = 1.0f;
}


HudLayouter::~HudLayouter(void)
{
}


void HudLayouter::setReference(bool bHorzAbs, bool bVertAbs)
{
    m_bHorzAbsolute = bHorzAbs;
    m_bVertAbsolute = bVertAbs;
}


void HudLayouter::setPosition(float x, float y, float width, float height)
{
    m_fltX = x;
    m_fltY = y;
    m_fltWidth = width;
    m_fltHeight = height;
}


void HudLayouter::setLayoutCamera(bool bLayout)
{
    m_bLayoutCamera = bLayout;
}


void HudLayouter::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    static unsigned nFrameCount = 0u;
    if(++nFrameCount % m_nRefreshSpeed != 0u)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    if(!pCullVisitor)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osg::CameraNode *pHudCamera = dynamic_cast<osg::CameraNode *>(pNode);
    if(!pHudCamera)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    const osg::Camera *pViewCamera = findViewportCamera(pNode, pCullVisitor);
    osg::Viewport *pHudViewport = pHudCamera->getViewport();
    if(!pHudViewport)
    {
        pHudCamera->setViewport(0, 0, 1, 1);
        pHudViewport = pHudCamera->getViewport();
    }

    calcViewport(pViewCamera->getViewport(), pHudViewport);

    if(m_bLayoutCamera)
    {
        double dblLeft = -1.0, dblRight = 1.0;
        if(m_bHorzAbsolute)
        {
            dblLeft = -0.5 * pHudViewport->width();
            dblRight = 0.5 * pHudViewport->width();
        }

        double dblBottom = -1.0, dblTop = 1.0;
        if(m_bVertAbsolute)
        {
            dblBottom = -0.5 * pHudViewport->height();
            dblTop = 0.5 * pHudViewport->height();
        }
        pHudCamera->setProjectionMatrixAsOrtho2D(dblLeft, dblRight, dblBottom, dblTop);
    }

    traverse(pNode, pNodeVisitor);
}


void HudLayouter::calcViewport(const osg::Viewport *pViewport, osg::Viewport *pHudViewport)
{
    int nX, nWidth;
    if(m_bHorzAbsolute)
    {
        if(m_fltX >= 0.0f)
        {
            nX = int(m_fltX + 0.5f) + pViewport->x();
        }
        else
        {
            // 负值则表示折返
            nX = int(pViewport->width() - int(m_fltX + 0.5f)) + pViewport->x();
        }
        nWidth = osg::clampBetween(int(m_fltWidth + 0.5f), 0, int(pViewport->width()));
    }
    else
    {
        if(m_fltX >= 0.0f)
        {
            nX = int(pViewport->width() * m_fltX + pViewport->x() + 0.5f);
        }
        else
        {
            // 负值则表示折返
            nX = int(pViewport->width() * (1.0f - m_fltX) + pViewport->x() + 0.5f);
        }
        nWidth = osg::clampBetween(int(pViewport->width() * m_fltWidth + 0.5f), 0, int(pViewport->width()));
    }


    int nY, nHeight;
    if(m_bVertAbsolute)
    {
        if(m_fltY >= 0.0f)
        {
            nY = int(m_fltY + 0.5f) + pViewport->y();
        }
        else
        {
            // 负值则表示折返
            nY = int(pViewport->height() - int(m_fltY + 0.5f)) + pViewport->y();
        }
        nHeight = osg::clampBetween(int(m_fltHeight + 0.5f), 0, int(pViewport->height()));
    }
    else
    {
        if(m_fltY >= 0.0f)
        {
            nY = int(pViewport->height() * m_fltY + pViewport->y() + 0.5f);
        }
        else
        {
            nY = int(pViewport->height() * (1.0f - m_fltY) + pViewport->y() + 0.5f);
        }
        nHeight = osg::clampBetween(int(pViewport->height() * m_fltHeight + 0.5f), 0, int(pViewport->height()));
    }

    //nY -= 80;
    pHudViewport->setViewport(nX, nY, nWidth, nHeight);
}


osg::Camera *HudLayouter::findViewportCamera(osg::Node *pNode, osgUtil::CullVisitor *pCullVisitor)
{
    osg::Camera *pViewportCamera = NULL;
    osg::NodePathList listParentPath = pNode->getParentalNodePaths();
    osg::NodePathList::iterator itorPath = listParentPath.begin();
    for(; itorPath != listParentPath.end(); ++itorPath)
    {
        osg::NodePath &path = *itorPath;
        osg::NodePath::reverse_iterator itorNode = path.rbegin();
        for(; itorNode != path.rend(); ++itorNode)
        {
            osg::Node *pCamNode = *itorNode;
            if(pCamNode == pNode)    continue;

            osg::Camera *pCamera = dynamic_cast<osg::Camera *>(pCamNode);
            if(!pCamera)        continue;

            const osg::Transform::ReferenceFrame eFrame = pCamera->getReferenceFrame();
            if(eFrame != osg::Transform::ABSOLUTE_RF)    continue;
            pViewportCamera = pCamera;
            break;
        }
        if(itorNode != path.rend())    break;
    }


    if(!pViewportCamera)
    {
        osg::RenderInfo &info = pCullVisitor->getRenderInfo();
        osgViewer::View *pView = dynamic_cast<osgViewer::View *>(info.getView());
        if(pView->getName() == "1")
        {
            pViewportCamera = pView->getCamera();
        }
    }

    return pViewportCamera;
}


void HudLayouter::setRefreshSpeed(unsigned nRefreshSpeed)
{
    m_nRefreshSpeed = osg::clampAbove(nRefreshSpeed, 1u);
}
