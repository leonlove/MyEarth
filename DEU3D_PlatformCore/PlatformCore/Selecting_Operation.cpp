#include "Selecting_Operation.h"
#include <osgUtil/Radial.h>
#include <IDProvider/Definer.h>


void Selecting_Operation::waitForFinishing(IDList &listIDs, osg::Vec3d &minIntersectPoint)
{
    m_blockFinished.block();
    listIDs.assign(m_listResult.begin(), m_listResult.end());
	minIntersectPoint = m_minIntersectPoint;
}


bool Selecting_Operation::doAction(SceneGraphOperator *pOperator)
{
    bool bRetVal = true;

    m_listResult.clear();
    switch(m_eSelectingMode)
    {
    case SM_Point:
        doScreenPointSelecting(getCultureRootNode(pOperator));
        break;
    case SM_Rect:
        doScreenRectSelecting(getCultureRootNode(pOperator));
        break;
    default:
        bRetVal = false;
    }
    m_blockFinished.release();
    return bRetVal;
}


void Selecting_Operation::selectByScreenPoint(osgViewer::View *pView, const osg::Vec2s &point)
{
    m_eSelectingMode = SM_Point;
    m_pTargetView = pView;
    m_vecScreenSelectingCoord.clear();
    m_vecScreenSelectingCoord.push_back(point);
    m_blockFinished.reset();
}


void Selecting_Operation::selectByScreenRect(osgViewer::View *pView, const osg::Vec2s &ptLeftTop, const osg::Vec2s &ptRightBottom)
{
    m_eSelectingMode = SM_Rect;
    m_pTargetView = pView;
    m_vecScreenSelectingCoord.clear();
    m_vecScreenSelectingCoord.push_back(ptLeftTop);
    m_vecScreenSelectingCoord.push_back(ptRightBottom);
    m_blockFinished.reset();
}


void Selecting_Operation::doScreenPointSelecting(osg::Node *pTargetNode)
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

            if(id_Temp.ObjectID.m_nType == PARAM_POINT_ID ||
                    id_Temp.ObjectID.m_nType == PARAM_LINE_ID ||
                    id_Temp.ObjectID.m_nType == PARAM_FACE_ID)
            {
                m_listResult.push_back(id_Temp);
            }
        }
    }
}


static void makeSelectRect(const osg::Viewport *pViewport, const osg::Vec2 &point1, const osg::Vec2 &point2, osg::Vec2 &ptLeftTop, osg::Vec2 &ptRightBottom)
{
    ptLeftTop.x() = point1.x();
    ptRightBottom.x() = point2.x();
    if(point1.x() > point2.x())
    {
        ptLeftTop.x() = point2.x();
        ptRightBottom.x() = point1.x();
    }
    ptLeftTop.y() = point1.y();
    ptRightBottom.y() = point2.y();
    if(point1.y() > point2.y())
    {
        ptLeftTop.y() = point2.y();
        ptRightBottom.y() = point1.y();
    }

    const osg::Vec2 ptViewCenter(pViewport->width() * 0.5, pViewport->height() * 0.5);

    ptLeftTop.x() -= ptViewCenter.x();
    ptLeftTop.x() /= ptViewCenter.x();
    ptLeftTop.x() -= 0.000001;

    ptRightBottom.x() -= ptViewCenter.x();
    ptRightBottom.x() /= ptViewCenter.x();
    ptRightBottom.x() += 0.000001;

    ptLeftTop.y() -= ptViewCenter.y();
    ptLeftTop.y() /= ptViewCenter.y();
    ptLeftTop.y() -= 0.000001;

    ptRightBottom.y() -= ptViewCenter.y();
    ptRightBottom.y() /= ptViewCenter.y();
    ptRightBottom.y() += 0.0000001;
}


void Selecting_Operation::doScreenRectSelecting(osg::Node *pTargetNode)
{
    osg::Camera *pCurrentCamera = m_pTargetView->getCamera();

    const osg::Vec2s &ptLeftTop = m_vecScreenSelectingCoord[0];
    const osg::Vec2s &ptRightBottom = m_vecScreenSelectingCoord[1];
    const osg::Vec2 tmp1(ptLeftTop.x(), ptLeftTop.y()), tmp2(ptRightBottom.x(), ptRightBottom.y());
    osg::Vec2 left_top, bottom_right;
    makeSelectRect(pCurrentCamera->getViewport(), tmp1, tmp2, left_top, bottom_right);

    osg::ref_ptr<osgUtil::PolytopeIntersector>    pPicker = new osgUtil::PolytopeIntersector
    (
        osgUtil::Intersector::PROJECTION,
        left_top.x(),
        left_top.y(),
        bottom_right.x(),
        bottom_right.y()
    );

    osgUtil::IntersectionVisitor iv(pPicker);
    pCurrentCamera->accept(iv);
    if(!pPicker->containsIntersections())
    {
        return;
    }

	double dblMin = FLT_MAX;
	osg::Vec3d ptCameraPos, ptCenter, vecUp;
	pCurrentCamera->getViewMatrixAsLookAt(ptCameraPos, ptCenter, vecUp);

    osgUtil::PolytopeIntersector::Intersections &Inters = pPicker->getIntersections();
    osgUtil::PolytopeIntersector::Intersections::iterator itorPath = Inters.begin();
    for(; itorPath != Inters.end(); ++itorPath)
    {
        const osg::NodePath &np = (*itorPath).nodePath;
        osg::NodePath::const_reverse_iterator ritor = np.rbegin();
        for(; ritor != np.rend(); ++ritor)
        {
            const ID &id_Temp = (*ritor)->getID();
            if(!id_Temp.isValid())
            {
                continue;
            }

            if(id_Temp.ObjectID.m_nType == PARAM_POINT_ID ||
                id_Temp.ObjectID.m_nType == PARAM_LINE_ID ||
                id_Temp.ObjectID.m_nType == PARAM_FACE_ID)
            {
				osg::Vec3d point = (*itorPath).matrix.valid() ? (*itorPath).localIntersectionPoint * (*((*itorPath).matrix)) : (*itorPath).localIntersectionPoint;
				const double dbl = (ptCameraPos - point).length();
				if(dbl < dblMin)
				{
					dblMin = dbl;
					m_minIntersectPoint = point;
				}

                m_listResult.push_back(id_Temp);

                break;
            }
			else if (id_Temp.ObjectID.m_nType == DETAIL_DYN_LINE_ID)
			{
				osg::Vec3d point = (*itorPath).matrix.valid() ? (*itorPath).localIntersectionPoint * (*((*itorPath).matrix)) : (*itorPath).localIntersectionPoint;
				const double dbl = (ptCameraPos - point).length();
				if(dbl < dblMin)
				{
					dblMin = dbl;
					m_minIntersectPoint = point;
				}
			}
        }
    }

    std::sort(m_listResult.begin(), m_listResult.end());
    IDList::iterator itorNewEnd = std::unique(m_listResult.begin(), m_listResult.end());
    m_listResult.erase(itorNewEnd, m_listResult.end());
}




