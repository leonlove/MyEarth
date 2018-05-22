#include "FetchingElevation_Operation.h"
#include <osgUtil/FindNodeVisitor.h>


double FetchingElevation_Operation::waitForFinishing(void)
{
    m_blockFinished.block();
    return m_dblElevation;
}


void FetchingElevation_Operation::fetchElevation(osgViewer::View *pView, const cmm::math::Point2d &position)
{
    m_ptCoordinate = position;
    m_pTargetView = pView;
    m_blockFinished.reset();
}


bool FetchingElevation_Operation::doAction(SceneGraphOperator *pOperator)
{
    osg::ref_ptr<osg::Node> pRootNode = m_pTargetView->getSceneData();
    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder = new osgUtil::FindNodeByNameVisitor("TerrainRootNode");
    pRootNode->accept(*pFinder);

    osg::Node *pTerrainNode = pFinder->getTargetNode();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptPosition;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptCoordinate.y(), m_ptCoordinate.x(), -1000.0, ptPosition.x(), ptPosition.y(), ptPosition.z());

    osg::Vec3d p1;
    pEllipsoidModel->convertXYZToLatLongHeight(ptPosition.x(), ptPosition.y(), ptPosition.z(), p1.y(), p1.x(), p1.z());

    osg::Vec3d p2;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_ptCoordinate.y(), m_ptCoordinate.x(), 0.0, p2.x(), p2.y(), p2.z());

    osg::Vec3d vecUpLine = ptPosition;
    vecUpLine.normalize();
    const osgUtil::Radial3  ray(ptPosition, vecUpLine);

    m_dblElevation = 0.0;
    osg::Vec3d ptHit;
    const bool bHit = hitScene(ray, pTerrainNode, ptHit);
    if(bHit)
    {
        osg::Vec3d ptHitCoord;
        pEllipsoidModel->convertXYZToLatLongHeight(ptHit.x(), ptHit.y(), ptHit.z(), ptHitCoord.y(), ptHitCoord.x(), ptHitCoord.z());
        m_dblElevation = ptHitCoord.z();
    }
    m_blockFinished.release();
    return true;
}


bool FetchingElevation_Operation::hitScene(const osgUtil::Radial3 &ray, osg::Node *pTerrainNode, osg::Vec3d &ptHitTest) const
{
    const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
    osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e8));
    osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
    pTerrainNode->accept(*pVisitor);

    if(!pPicker->containsIntersections())
    {
        return false;
    }

    const osgUtil::LineSegmentIntersector::Intersections &listInters = pPicker->getIntersections();
    if(listInters.empty())
    {
        return false;
    }

    const osgUtil::LineSegmentIntersector::Intersection &inter = *listInters.begin();
    ptHitTest = inter.getWorldIntersectPoint();
    return true;
}
