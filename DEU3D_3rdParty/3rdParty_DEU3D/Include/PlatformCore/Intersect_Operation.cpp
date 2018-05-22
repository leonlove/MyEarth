#include "Intersect_Operation.h"
#include <osgUtil/Radial.h>
#include <IDProvider/Definer.h>

void Intersect_Operation::waitForFinishing(std::vector<osg::Vec3d> &vecResults)
{
    m_blockFinished.block();
    vecResults.assign(m_vecResults.begin(), m_vecResults.end());
}

bool Intersect_Operation::doAction(SceneGraphOperator *pOperator)
{
    m_vecResults.clear();
    doScreenPointSelecting(getTerrainRootNode(pOperator));
    m_blockFinished.release();
    return true;
}


void Intersect_Operation::selectByScreenPoint(osgViewer::View *pView, const osg::Vec2s &point)
{
    m_pTargetView = pView;
    m_vecScreenSelectingCoord.clear();
    m_vecScreenSelectingCoord.push_back(point);
    m_blockFinished.reset();
}

void Intersect_Operation::doScreenPointSelecting(osg::Node *pTargetNode)
{
    const osg::Camera *pCurrentCamera = m_pTargetView->getCamera();
    const osg::Viewport *pViewport = pCurrentCamera->getViewport();

    const osg::Vec2 ptViewCenter(pViewport->width() * 0.5, pViewport->height() * 0.5);
    const osg::Vec2s &point = m_vecScreenSelectingCoord[0];
    osg::Vec2  ptPosNormalize(point.x(), point.y());
    ptPosNormalize.x() -= ptViewCenter.x();
    ptPosNormalize.x() /= ptViewCenter.x();
    ptPosNormalize.y() -= ptViewCenter.y();
    ptPosNormalize.y() /= ptViewCenter.y();

    const osg::Matrixd &mtxProj      = pCurrentCamera->getProjectionMatrix();
    const osg::Matrixd &mtxView      = pCurrentCamera->getViewMatrix();
    const osg::Matrixd &mtxInverseVP = osg::Matrixd::inverse(mtxView * mtxProj);

    const osg::Vec3d ptNear(ptPosNormalize.x(), ptPosNormalize.y(), -1.0f);
    const osg::Vec3d ptMid(ptPosNormalize.x(), ptPosNormalize.y(), 0.0f);

    const osg::Vec3d ptRayBegin = ptNear * mtxInverseVP;
    const osg::Vec3d ptRayEnd   = ptMid  * mtxInverseVP;
    osg::Vec3d       vecRayDir  = ptRayEnd - ptRayBegin;
    const double     fltLen     = vecRayDir.normalize();
    if(!vecRayDir.valid())      return;
    if(fltLen <= FLT_EPSILON)   return;

    const osgUtil::Radial3 rayMouse(ptRayBegin, vecRayDir);

    osg::ref_ptr<osgUtil::LineSegmentIntersector> pPicker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, rayMouse.getOrigin(), rayMouse.getPoint(1e9));
    osgUtil::IntersectionVisitor iv(pPicker);
    pTargetNode->accept(iv);
    if(!pPicker->containsIntersections())
    {
        return;
    }

    const osgUtil::LineSegmentIntersector::Intersections &intersections     = pPicker->getIntersections();
    osgUtil::LineSegmentIntersector::Intersections::const_iterator itorPath = intersections.begin();
    for(; itorPath != intersections.end(); ++itorPath)
    {
        const osg::NodePath &np = itorPath->nodePath;
        osg::NodePath::const_reverse_iterator ritor = np.rbegin();

        for(; np.rend() != ritor; ++ritor)
        {
            const ID &id_Temp = (*ritor)->getID();
            if(!id_Temp.isValid())  continue;
            
            if(id_Temp.ObjectID.m_nType == TERRAIN_TILE)
            {
                m_vecResults.push_back(itorPath->getWorldIntersectPoint());
            }
        }
    }
}




