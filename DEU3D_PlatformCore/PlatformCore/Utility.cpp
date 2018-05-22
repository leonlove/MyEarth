#include "Utility.h"
#include <assert.h>
#include <osg/MatrixTransform>
#include <osg/ProxyNode>
#include <osg/ShapeDrawable>
#include <osg/Point>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/Switch>
#include <osgUtil\FindNodeVisitor.h>
#include <osgDB\ReadFile>
#include <osg/NodeVisitor>
#include <osg/PagedLOD>
#include <osg/Node>
#include <osgText/Text>
#include <DEUDBProxy\IDEUDBProxy.h>
#include <osg/Texture2D>
#include <osgDB/FileNameUtils>
#include <strstream>
#include <osgUtil/CommonOSG.h>

#include <ParameterSys/Parameter.h>

#include <common/IDEUImage.h>
#include "AnimationModel.h"
#include "NavigationPath.h"
#include "DEUPlatformCore.h"

int FindInterSection(osg::Vec3d &P0, osg::Vec3d &D0, osg::Vec3d &P1, osg::Vec3d &D1, osg::Vec3d &I)
{
    double sqrEpsilon = sin(osg::PI / 1800.0);
    D0.normalize();
    D1.normalize();
    osg::Vec3d E = P1 - P0;
    double kross = D0.x() * D1.y() - D0.y() * D1.x();
    double sqrKross = kross * kross;
    double sqrLne0 = D0.length2();
    double sqrLne1 = D1.length2();
    if(sqrKross >= sqrEpsilon *sqrLne0 * sqrLne1)
    {
        float s = (E.x() * D1.y() - E.y() * D1.x()) / kross;
        I = P0 + D0 * s;
        return 1;
    }

    double sqrlenE = E.length2();
    kross = E.x() * D0.y() - E.y() * D0.x();
    sqrKross = kross * kross;
    if(sqrKross > sqrEpsilon * sqrLne0 * sqrlenE)
    {
        return 0;
    }

    return 2;
}

Utility::Utility(DEUPlatformCore *pPlatformCore)
{
    m_pPlatformCore = pPlatformCore;
}


Utility::~Utility()
{
    m_pPlatformCore = NULL;
}


IAnimationModel *Utility::createAnimationModel(void)
{
    osg::ref_ptr<AnimationModel>    pAnimationModel = new AnimationModel;
    return pAnimationModel.release();
}


INavigationKeyframe *Utility::createNavigationKeyframe()
{
    return new NavigationKeyframe;
}


INavigationPath *Utility::createNavigationPath()
{
    return new NavigationPath;
}


INavigationPathContainer *Utility::createNavigationPathContainer()
{
    return new NavigationPathContainer;
}


INavigationPath *Utility::createNavigationPathByCoords(const std::vector<cmm::math::Point3d> &vecCoords, bool bFixedAzimuth, double dblAzimuth, bool bFixedPitch, double dblPitch)
{
    if(vecCoords.size() < 2u)
    {
        return NULL;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    OpenSP::sp<NavigationPath>  pNavigationPath = new NavigationPath;
    std::vector<cmm::math::Point3d>::const_iterator itor0 = vecCoords.begin();
    std::vector<cmm::math::Point3d>::const_iterator itor1 = itor0 + 1;
    for( ; itor1 != vecCoords.end(); ++itor1, ++itor0)
    {
        const cmm::math::Point3d &coord0 = *itor0;
        const cmm::math::Point3d &coord1 = *itor1;

        CameraPose pose;
        pose.m_dblPositionX    = coord0.x();
        pose.m_dblPositionY    = coord0.y();
        pose.m_dblHeight       = coord0.z();
        pose.m_dblAzimuthAngle = dblAzimuth;
        pose.m_dblPitchAngle   = dblPitch;
        if(!bFixedAzimuth || !bFixedPitch)
        {
            osg::Vec3d point0, point1;
            pEllipsoidModel->convertLatLongHeightToXYZ(coord0.y(), coord0.x(), coord0.z(), point0.x(), point0.y(), point0.z());
            pEllipsoidModel->convertLatLongHeightToXYZ(coord1.y(), coord1.x(), coord1.z(), point1.x(), point1.y(), point1.z());

            osg::Vec3d vecDirection = point1 - point0;
            vecDirection.normalize();

            const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(coord0.y(), coord0.x());
            const osg::Vec3d vecEastern   = pEllipsoidModel->computeLocalEastern(coord0.y(), coord0.x());

            const double dblCosPitch = vecDirection * vecPlumbLine;
            if(!bFixedPitch)
            {
                pose.m_dblPitchAngle = acos(dblCosPitch);
            }

            if(!bFixedAzimuth)
            {
                osg::Vec3d vecProjection = vecDirection - vecPlumbLine * dblCosPitch;
                const double dblProjLen = vecProjection.normalize();
                if(cmm::math::floatEqual(dblProjLen, 0.0, 0.00001))
                {
                    pose.m_dblAzimuthAngle = osg::PI_2;
                    if(itor0 != vecCoords.begin())
                    {
                        const unsigned nCount = pNavigationPath->getItemCount();
                        INavigationKeyframe *pLastKeyframe = pNavigationPath->getItem(nCount - 1u);
                        pose.m_dblAzimuthAngle = pLastKeyframe->getCameraPose().m_dblAzimuthAngle;
                    }
                }
                else
                {
                    const double dblCosAzimuth = vecProjection * vecEastern;
                    const osg::Vec3d vecNorthern = pEllipsoidModel->computeLocalNorthern(coord0.y(), coord0.x());
                    const double dbl = vecProjection * vecNorthern;
                    pose.m_dblAzimuthAngle = acos(dblCosAzimuth);
                    if(dbl < 0.0)
                    {
                        pose.m_dblAzimuthAngle = osg::PI + osg::PI - pose.m_dblAzimuthAngle;
                    }
                }
            }
        }

        OpenSP::sp<NavigationKeyframe> pKeyframe = new NavigationKeyframe;
        pKeyframe->setCameraPose(pose);
        pKeyframe->setArgForTime(true);
        pKeyframe->setRotate_TimeOrSpeed(1.0);
        pKeyframe->setTrans_TimeOrSpeed(1.0);
        pNavigationPath->appendItem(pKeyframe.get());
    }

    CameraPose pose;
    pose.m_dblPositionX    = vecCoords.back().x();
    pose.m_dblPositionY    = vecCoords.back().y();
    pose.m_dblHeight       = vecCoords.back().z();
    pose.m_dblAzimuthAngle = dblAzimuth;
    pose.m_dblPitchAngle   = dblPitch;
    if(!bFixedAzimuth || !bFixedPitch)
    {
        const unsigned nCount = pNavigationPath->getItemCount();
        INavigationKeyframe *pLastKeyframe = pNavigationPath->getItem(nCount - 1u);
        if(!bFixedPitch)
        {
            pose.m_dblPitchAngle = pLastKeyframe->getCameraPose().m_dblPitchAngle;
        }
        if(!bFixedAzimuth)
        {
            pose.m_dblAzimuthAngle = pLastKeyframe->getCameraPose().m_dblAzimuthAngle;
        }
    }

    OpenSP::sp<NavigationKeyframe> pKeyframe = new NavigationKeyframe;
    pKeyframe->setCameraPose(pose);
    pKeyframe->setArgForTime(true);
    pKeyframe->setRotate_TimeOrSpeed(1.0);
    pKeyframe->setTrans_TimeOrSpeed(1.0);
    pNavigationPath->appendItem(pKeyframe.get());

    return pNavigationPath.release();
}


CameraPose Utility::createCameraPoseByRect(const cmm::math::Box2d &box)
{
    CameraPose pose;
    pose.m_dblPitchAngle   = 0.0;
    pose.m_dblAzimuthAngle = osg::PI_2;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    const cmm::math::Point2d ptLeftBottom = box.corner(cmm::math::Box2d::LeftBottom);
    const cmm::math::Point2d ptRightBottom = box.corner(cmm::math::Box2d::RightBottom);
    const cmm::math::Point2d ptRightTop   = box.corner(cmm::math::Box2d::RightTop);
    const cmm::math::Point2d ptLeftTop   = box.corner(cmm::math::Box2d::LeftTop);

    osg::Vec3d plb, prb, prt, plt;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptLeftBottom.y(),   ptLeftBottom.x(),   0.0, plb.x(), plb.y(), plb.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptRightBottom.y(),   ptRightBottom.x(),   0.0, prb.x(), prb.y(), prb.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptRightTop.y(),   ptRightTop.x(),   0.0, prt.x(), prt.y(), prt.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptLeftTop.y(),   ptLeftTop.x(),   0.0, plt.x(), plt.y(), plt.z());

    osg::Vec3d pc = (plb + prb + prt +plt) / 4.0;
    double dblLat, dblLon, dblHei;
    pEllipsoidModel->convertXYZToLatLongHeight(pc.x(), pc.y(), pc.z(), dblLat, dblLon, dblHei);
    pose.m_dblPositionX = dblLon;
    pose.m_dblPositionY = dblLat;

    //double bottom_half_len1 = (plb - prb).length() * 0.5;
    //double bottom_half_len2 = (prt - plt).length() * 0.5;

    //printf("%f=====%f\n", bottom_half_len1, bottom_half_len2);

    //pose.m_dblHeight = std::min(bottom_half_len1, bottom_half_len2) / tan(osg::DegreesToRadians(22.5));
    pEllipsoidModel->convertXYZToLatLongHeight(pc.x(), pc.y(), pc.z() + (plb - prt).length() * 0.5 / tan(osg::DegreesToRadians(22.5)), dblLat, dblLon, dblHei);
    pose.m_dblHeight = dblHei;
    return pose;
}


bool Utility::getViewArea(unsigned nIndex, cmm::math::Box2d &box)
{
#if 0
    OpenSP::Ref *pViewerObject = m_pSceneViewer.get();
    osgViewer::CompositeViewer *pViewer = dynamic_cast<osgViewer::CompositeViewer *>(pViewerObject);
    if(nIndex >= pViewer->getNumViews())
    {
        return false;
    }
    osgViewer::View *pView = pViewer->getView(nIndex);
    osg::Camera *pCamera = pView->getCamera();
    const osg::Viewport *pViewport = pCamera->getViewport();
    const double dblRatio = pViewport->width() / pViewport->height();

    const NavigatorManager *pNaviMgr = dynamic_cast<const NavigatorManager *>(m_pSceneViewer->getNavigatorManager(nIndex));

    CameraPose pose;
    pNaviMgr->getCameraPose(pose);
    if(pose.m_dblPitchAngle > osg::DegreesToRadians(85.0))
    {
        // 视线基本平视，无法看到
        return false;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d ptCamera;
    pEllipsoidModel->convertLatLongHeightToXYZ(pose.m_dblPositionY, pose.m_dblPositionX, pose.m_dblHeight, ptCamera.x(), ptCamera.y(), ptCamera.z());
    const osg::Vec3d vecLocalUp = -pEllipsoidModel->computeLocalPlumbLine(pose.m_dblPositionY, pose.m_dblPositionX);

    osg::Quat qt0, qt1;
    qt0.makeRotate(vecLocalUp, osg::Vec3d(0.0, 0.0, 1.0));
    qt1.makeRotate(osg::Vec3d(0.0, 0.0, 1.0), vecLocalUp);

    ptCamera = qt0 * ptCamera;

    const double dblDelta  = pose.m_dblHeight * tan(pose.m_dblPitchAngle);
    const double dblDeltaX = dblDelta * cos(pose.m_dblAzimuthAngle);
    const double dblDeltaY = dblDelta * sin(pose.m_dblAzimuthAngle);

    const osg::Vec3d ptCenter(ptCamera.x() + dblDeltaX, ptCamera.y() + dblDeltaY, ptCamera.z());
    const double dblExpandY = pose.m_dblHeight * sin(osg::DegreesToRadians(45.0));
    const double dblExpandX = dblExpandY * dblRatio;

    osg::Vec3d ptLeftBottom(ptCenter.x() - dblExpandX, ptCenter.y() - dblExpandY, ptCenter.z());
    osg::Vec3d ptRightTop(ptCenter.x() + dblExpandX, ptCenter.y() + dblExpandY, ptCenter.z());

    ptLeftBottom = qt1 * ptLeftBottom;
    ptRightTop   = qt1 * ptRightTop;

    cmm::math::Point3d point0, point1;
    pEllipsoidModel->convertXYZToLatLongHeight(ptLeftBottom.x(), ptLeftBottom.y(), ptLeftBottom.z(), point0.y(), point0.x(), point0.z());
    pEllipsoidModel->convertXYZToLatLongHeight(ptRightTop.x(), ptRightTop.y(), ptRightTop.z(), point1.y(), point1.x(), point1.z());
    box.set(cmm::math::Point2d(point0.x(), point0.y()), cmm::math::Point2d(point1.x(), point1.y()));
#else
    //double dblTime1 = osg::Timer::instance()->time_u();

    OpenSP::Ref *pViewerObject = m_pPlatformCore->getSceneViewer();
    osgViewer::CompositeViewer *pViewer = dynamic_cast<osgViewer::CompositeViewer *>(pViewerObject);
    if(nIndex >= pViewer->getNumViews())
    {
        return false;
    }
    osgViewer::View *pView = pViewer->getView(nIndex);
    osg::Camera *pCamera = pView->getCamera();
    const osg::Viewport *pViewport = pCamera->getViewport();

    ID id;
    osg::Vec3d point;
    //LEFTBOTTOM
    cmm::math::Point2i ptlb(pViewport->x(), pViewport->y());
    //RIGHTUP
    cmm::math::Point2i ptrt(pViewport->x() + pViewport->width() - 1, pViewport->y() + pViewport->height() - 1);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    double dblLat1, dblLon1, dblHeight1, dblLat2, dblLon2, dblHeight2;

    if(!m_pPlatformCore->selectByScreenPoint(ptlb, point))
    {
        return false;
    }
    pEllipsoidModel->convertXYZToLatLongHeight(point.x(), point.y(),  point.z(), dblLat1, dblLon1, dblHeight1);

    if(!m_pPlatformCore->selectByScreenPoint(ptrt, point))
    {
        return false;
    }

    pEllipsoidModel->convertXYZToLatLongHeight(point.x(), point.y(),  point.z(), dblLat2, dblLon2, dblHeight2);

    box.set(cmm::math::Point2d(osg::minimum(dblLon1, dblLon2), osg::minimum(dblLat1, dblLat2)), cmm::math::Point2d(osg::maximum(dblLon1, dblLon2), osg::maximum(dblLat1, dblLat2)));

    //double dblTime2 = osg::Timer::instance()->time_u();

    //printf("Time is %fms\n", (dblTime2 - dblTime1) / 1000.0);
#endif
    return true;
}

cmm::math::Polygon2 Utility::enlargePolygon(cmm::math::Polygon2 &polygon, double dblDist)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::ref_ptr<osg::Vec3dArray> pPoints = new osg::Vec3dArray;
    cmm::math::Point2d ptCenter(0.0, 0.0);

    unsigned int nCount = polygon.getVerticesCount();
    for(unsigned int i = 0; i < nCount; i++)
    {
        const cmm::math::Point2d &pt = polygon.getSafeVertex(i);
        ptCenter += pt;
        osg::Vec3d vPt;
        pEllipsoidModel->convertLatLongHeightToXYZ(pt.y(), pt.x(), 0.0, vPt[0], vPt[1], vPt[2]);
        pPoints->push_back(vPt);
    }

    ptCenter /= (double)nCount;
    osg::Matrixd mtx, mtxI;
    pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight(ptCenter.y(), ptCenter.x(), 0.0, mtx);
    mtxI = osg::Matrixd::inverse(mtx);

    std::vector<osg::Vec3> vecPts;
    cmm::math::Point2d ptMin, ptMax;
    ptMin.set(DBL_MAX, DBL_MAX);
    ptMax.set(-DBL_MAX, -DBL_MAX);
    for(unsigned int i = 0; i < nCount; i++)
    {
        osg::Vec3d v1, v2;
        v2 = (*pPoints)[i];
        v1 = mtxI.preMult(v2);
        vecPts.push_back(v1);
        ptMin._v[0] = std::min(ptMin._v[0], v1[0]);
        ptMin._v[1] = std::min(ptMin._v[1], v1[1]);
        ptMax._v[0] = std::max(ptMax._v[0], v1[0]);
        ptMax._v[1] = std::max(ptMax._v[1], v1[1]);
    }

    ptMax -= ptMin;
    double dblScale = (ptMax.length() + dblDist) / ptMax.length();

    osg::Matrix mtxScale = osg::Matrix::scale(osg::Vec3(dblScale, dblScale, 0.0));

    cmm::math::Polygon2 poly;
    for(unsigned int i = 0; i < nCount; i++)
    {
        osg::Vec3d vPt = vecPts[i];
        vPt = mtxScale.postMult(vPt);
        vPt = mtx.preMult(vPt);
        (*pPoints)[i] = vPt;
        pEllipsoidModel->convertXYZToLatLongHeight((*pPoints)[i][0], (*pPoints)[i][1], (*pPoints)[i][2], vPt[1], vPt[0], vPt[2]);
        cmm::math::Point2d pt(vPt[0], vPt[1]);
        poly.addVertex(pt);
    }

    return poly;
}

cmm::math::Polygon2 Utility::generalCircle(cmm::math::Point2d &center, double dblRadius, double dblHits)
{
    if(dblHits < 0.1)
    {
        dblHits = 0.1;
    }
    else if(dblHits > 1.0)
    {
        dblHits = 1.0;
    }

    cmm::math::Polygon2 poly_circle;

    osg::Matrixd mtx;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight(center.y(), center.x(), 0.0, mtx);

    std::vector<osg::Vec3d> vecCircle;
    unsigned int nFragment = 360 * dblHits;
    double dblSetp = 2 * osg::PI / double(nFragment);
    for(unsigned int i = 0; i < nFragment; i++)
    {
        double dblAngle = i * dblSetp;
        vecCircle.push_back(osg::Vec3d(dblRadius * cos(dblAngle), dblRadius * sin(dblAngle), 0.0));
    }
    vecCircle.push_back(osg::Vec3d(dblRadius * cos(2 * osg::PI), dblRadius * sin(2 * osg::PI), 0.0));

    std::vector<osg::Vec3d>::iterator itor = vecCircle.begin();
    for(; itor != vecCircle.end(); ++itor)
    {
        osg::Vec3d vTemp = mtx.preMult(*itor);
        double dblLon, dblLat, dblHeight;
        pEllipsoidModel->convertXYZToLatLongHeight(vTemp.x(), vTemp.y(), vTemp.z(), dblLat, dblLon, dblHeight);
        poly_circle.addVertex(cmm::math::Point2d(dblLon, dblLat));
    }

    return poly_circle;
}

cmm::math::Polygon2 Utility::generalBufferArea(cmm::math::Point2d &point1, cmm::math::Point2d &point2, double dblDistance, double dblHits)
{
    osg::Vec3d vDir(point2.x() - point1.x(), point2.y() - point1.y(), 0.0);
    vDir.normalize();

    osg::Vec3d vX(1.0, 0.0, 0.0);

    double dblBegin1, dblEnd1;
    osg::Vec3d vDot1 = -vDir ^ vX;
    if(vDot1._v[2] > 0)
    {
        dblBegin1 = osg::PI + osg::PI_2 - acos(-vDir * vX);
        dblEnd1 = dblBegin1 + osg::PI;
    }
    else
    {
        dblBegin1 = osg::PI_2 + acos(-vDir * vX) - osg::PI;
        dblEnd1 = dblBegin1 + osg::PI;
    }

    double dblBegin2, dblEnd2;
    osg::Vec3d vDot2 = vDir ^ vX;
    if(vDot2._v[2] > 0)
    {
        dblBegin2 = osg::PI + osg::PI_2 - acos(vDir * vX);
        dblEnd2 = dblBegin2 + osg::PI;
    }
    else
    {
        dblBegin2 = osg::PI_2 + acos(vDir * vX) - osg::PI;
        dblEnd2 = dblBegin2 + osg::PI;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d vPt1, vPt2;
    pEllipsoidModel->convertLatLongHeightToXYZ(point1.y(), point1.x(), 0.0, vPt1.x(), vPt1.y(), vPt1.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(point2.y(), point2.x(), 0.0, vPt2.x(), vPt2.y(), vPt2.z());

    osg::Vec3d vLen = vPt2 - vPt1;
    dblDistance = osg::minimum(dblDistance, vLen.length());

    if(dblHits > 1.0)
    {
        dblHits = 1.0;
    }
    else if(dblHits < 0.1)
    {
        dblHits = 0.1;
    }

    unsigned int nStep = dblHits * 10;
    dblHits = nStep / 10.0;
    nStep = 180 * dblHits;
    double dblStepAngle = osg::PI / nStep;

    osg::ref_ptr<osg::Vec3dArray> pVertexArray = new osg::Vec3dArray;

    {
        osg::Matrixd matrix;
        const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(point1.x(), point1.y()), 0.0, 0.0);
        matrix.postMultRotate(qtRotation);

        matrix.postMultTranslate(vPt1);

        double dblAngle = dblBegin1;
        osg::Vec3d vtx;
        for(unsigned i = 0u; i <= nStep; i++)
        {
            vtx.x() = cos(dblAngle) * dblDistance;
            vtx.y() = sin(dblAngle) * dblDistance;
            vtx.z() = 0.0;
            pVertexArray->push_back(matrix.preMult(vtx));

            dblAngle += dblStepAngle;
        }
    }

    {
        osg::Matrixd matrix;
        const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(point2.x(), point2.y()), 0.0, 0.0);
        matrix.postMultRotate(qtRotation);

        matrix.postMultTranslate(vPt2);

        double dblAngle = dblBegin2;
        osg::Vec3d vtx;
        for(unsigned i = 0u; i <= nStep; i++)
        {
            vtx.x() = cos(dblAngle) * dblDistance;
            vtx.y() = sin(dblAngle) * dblDistance;
            vtx.z() = 0.0;
            pVertexArray->push_back(matrix.preMult(vtx));

            dblAngle += dblStepAngle;
        }
    }

    pVertexArray->push_back((*pVertexArray)[0]);

    unsigned int nSize = pVertexArray->size();
    double dblLat, dblLon, dblHeight;
    cmm::math::Polygon2 poly;
    for(unsigned int i = 0; i < nSize; i++)
    {
        osg::Vec3d &vtx = (*pVertexArray)[i];
        pEllipsoidModel->convertXYZToLatLongHeight(vtx.x(), vtx.y(), vtx.z(), dblLat, dblLon, dblHeight);
        poly.addVertex(cmm::math::Point2d(dblLon, dblLat));
    }

    return poly;
}

cmm::math::Polygon2 Utility::generalCubeConnector(cmm::math::Point2d &point1, cmm::math::Point2d &point2, cmm::math::Point2d &point3, cmm::math::Point2d &point4, double dblDistance, double dblHits)
{
    osg::Vec3d v1, v2, v3, v4, v22;
    v1.set(point1.x(), point1.y(), 0.0);
    v2.set(point2.x(), point2.y(), 0.0);
    v3.set(point3.x(), point3.y(), 0.0);
    v4.set(point4.x(), point4.y(), 0.0);

    double dblX, dblY, dblZ;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    pEllipsoidModel->convertLatLongHeightToXYZ(v2.y(), v2.x(), 0.0, dblX, dblY, dblZ);
    v22.set(dblX, dblY, dblZ);

    osg::Vec3d vX(1.0, 0.0, 0.0);
    osg::Vec3d vZ(0.0, 0.0, 1.0);

    osg::Vec3d vC1 = (v2 + v3) * 0.5;
    osg::Vec3d vDir = v3 - v2;
    vDir.normalize();
    osg::Vec3d vCui = vDir ^ vZ;
    vCui.normalize();

    osg::Vec3d vDir1 = v2 - v1;
    vDir1.normalize();
    osg::Vec3d vCui1 = vDir1 ^ vZ;
    vCui1.normalize();

    osg::Vec3d vInter1;
    FindInterSection(vC1, vCui, v2, vCui1, vInter1);

    osg::Vec3d vDir2 = v4 - v3;
    vDir2.normalize();
    osg::Vec3d vCui2 = vDir2 ^ osg::Vec3(0.0, 0.0, 1.0);
    vCui2.normalize();

    osg::Vec3d vInter2;
    FindInterSection(vC1, vCui, v3, vCui2, vInter2);

    osg::Vec3d vC2 = (vInter1 + vInter2) * 0.5;

    double dblRadius = (v2 - vC2).length();

    osg::Vec3d vDir3 = v2 - vC2;
    vDir3.normalize();
    osg::Vec3d vDir4 = v3 - vC2;
    vDir4.normalize();

    double dblAngle = acos(vDir3 * vDir4);

    double dblBegin1, dblBegin2;
    osg::Vec3d vDot1 = vDir3 ^ vX;
    if(vDot1._v[2] > 0)
    {
        dblBegin1 = 2 * osg::PI - acos(vDir3 * vX);
    }
    else
    {
        dblBegin1 = acos(vDir3 * vX);
    }

    osg::Vec3d vDot2 = vDir4 ^ vX;
    if(vDot2._v[2] > 0)
    {
        dblBegin2 =  2 * osg::PI - acos(vDir4 * vX);
    }
    else
    {
        dblBegin2 = acos(vDir4 * vX);
    }

    double dblBegin = osg::minimum(dblBegin1, dblBegin2);
    double dblEnd = dblBegin + dblAngle;

    unsigned int nStep = dblHits * 10;
    dblHits = nStep / 10.0;
    nStep = 180 * dblHits;
    double dblStepAngle = osg::PI / nStep;

    std::vector<osg::Vec3d> vecPoint1, vecPoint2;

    pEllipsoidModel->convertLatLongHeightToXYZ(vC2.y(), vC2.x(), 0.0, dblX, dblY, dblZ);
    osg::Vec3d vCenter(dblX, dblY, dblZ);

    osg::Matrixd matrix;
    const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vC2.x(), vC2.y()), 0.0, 0.0);
    matrix.postMultRotate(qtRotation);

    matrix.postMultTranslate(vCenter);

    double vRadius1 = (vCenter - v22).length() - 1.0;
    double vRadius2 = (vCenter - v22).length() + 1.0;

    osg::Vec3d vtx1, vtx2;
    for(double dblTempAngle = dblBegin; dblTempAngle < dblEnd; dblTempAngle += dblStepAngle)
    {
        vtx1.x() = cos(dblTempAngle) * vRadius1;
        vtx1.y() = sin(dblTempAngle) * vRadius1;
        vtx1.z() = 0.0;
        vecPoint1.push_back(vtx1);

        vtx2.x() = cos(dblTempAngle) * vRadius2;
        vtx2.y() = sin(dblTempAngle) * vRadius2;
        vtx2.z() = 0.0;
        vecPoint2.push_back(vtx2);
    }

    vtx1.x() = cos(dblEnd) * vRadius1;
    vtx1.y() = sin(dblEnd) * vRadius1;
    vtx1.z() = 0.0;
    vecPoint1.push_back(vtx1);

    vtx2.x() = cos(dblEnd) * vRadius2;
    vtx2.y() = sin(dblEnd) * vRadius2;
    vtx2.z() = 0.0;
    vecPoint2.push_back(vtx2);

    cmm::math::Polygon2 poly;
    double dblLon, dblLat, dblHeight;

    for(std::vector<osg::Vec3d>::iterator itor = vecPoint1.begin(); itor != vecPoint1.end(); ++itor)
    {
        osg::Vec3d vTemp = matrix.preMult(*itor);
        pEllipsoidModel->convertXYZToLatLongHeight(vTemp.x(), vTemp.y(), vTemp.z(), dblLat, dblLon, dblHeight);
        poly.addVertex(cmm::math::Point2d(dblLon, dblLat));
    }
    for(std::vector<osg::Vec3d>::reverse_iterator ritor = vecPoint2.rbegin(); ritor != vecPoint2.rend(); ++ritor)
    {
        osg::Vec3d vTemp = matrix.preMult(*ritor);
        pEllipsoidModel->convertXYZToLatLongHeight(vTemp.x(), vTemp.y(), vTemp.z(), dblLat, dblLon, dblHeight);
        poly.addVertex(cmm::math::Point2d(dblLon, dblLat));
    }

    poly.addVertex(poly.getSafeVertex(0));

    return poly;
}


bool Utility::shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin)
{
    osg::Vec3d ptBegin, ptEnd;
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(begin.x(), begin.y(), begin.z(), ptBegin.x(), ptBegin.y(), ptBegin.z());
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(end.x(), end.y(), end.z(), ptEnd.x(), ptEnd.y(), ptEnd.z());

    osg::Vec3d  vDir = ptEnd - ptBegin;
    double      len  = vDir.normalize() - dblShortenLength;
    if (len <= 0.000001) return false;

    osg::Vec3d  dest;

    if (bShortenFromBegin)
    {
        dest = ptEnd - vDir * len;
        osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(dest.x(), dest.y(), dest.z(), begin.x(), begin.y(), begin.z());
    }
    else
    {
        dest = ptBegin + vDir * len;
        osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(dest.x(), dest.y(), dest.z(), end.x(), end.y(), end.z());
    }

    return true;
}


bool Utility::moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptBegin, ptEnd;
    pEllipsoidModel->convertLatLongHeightToXYZ(begin.y(), begin.x(), begin.z(), ptBegin.x(), ptBegin.y(), ptBegin.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(end.y(), end.x(), end.z(), ptEnd.x(), ptEnd.y(), ptEnd.z());

    osg::Vec3d  vecDir = ptEnd - ptBegin;
    const double dblLenDir = vecDir.normalize();
    if(cmm::math::floatEqual(dblLenDir, 0.0))
    {
        return false;
    }

    const cmm::math::Point3d center = (end + begin) * 0.5;
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(center.y(), center.x());
    osg::Vec3d vecRightDir = vecPlumbLine ^ vecDir;
    const double dblLenRight = vecRightDir.normalize();
    if(cmm::math::floatEqual(dblLenRight, 0.0))
    {
        return false;
    }

    vecRightDir *= dblMovingDist;
    ptBegin += vecRightDir;
    ptEnd   += vecRightDir;

    pEllipsoidModel->convertXYZToLatLongHeight(ptBegin.x(), ptBegin.y(), ptBegin.z(), begin.y(), begin.x(), begin.z());
    pEllipsoidModel->convertXYZToLatLongHeight(ptEnd.x(),   ptEnd.y(),   ptEnd.z(),   end.y(),   end.x(),   end.z());
    return true;
}


void Utility::convRect2Polygon(const cmm::math::Point2d &center, double dblWidth, double dblHeight, double dblAzimuth, cmm::math::Polygon2 &polygon)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d ptCenter;
    pEllipsoidModel->convertLatLongHeightToXYZ(center.y(), center.x(), 0.0, ptCenter.x(), ptCenter.y(), ptCenter.z());

    const osg::Vec3d vecEastern   = pEllipsoidModel->computeLocalEastern(center.y(), center.x());
    const osg::Vec3d vecNorthern  = pEllipsoidModel->computeLocalNorthern(center.y(), center.x());
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(center.y(), center.x());

    const osg::Quat  qtAzimuth(-dblAzimuth, vecPlumbLine);
    const osg::Vec3d vecWidthDir  = qtAzimuth * vecEastern;
    const osg::Vec3d vecHeightDir = qtAzimuth * vecNorthern;

    dblWidth  *= 0.5;
    dblHeight *= 0.5;

    const osg::Vec3d ptLeftBottom  = ptCenter - vecWidthDir * dblWidth - vecHeightDir * dblHeight;
    const osg::Vec3d ptRightBottom = ptCenter + vecWidthDir * dblWidth - vecHeightDir * dblHeight;
    const osg::Vec3d ptRightTop    = ptCenter + vecWidthDir * dblWidth + vecHeightDir * dblHeight;
    const osg::Vec3d ptLeftTop     = ptCenter - vecWidthDir * dblWidth + vecHeightDir * dblHeight;

    cmm::math::Point2d left_bottom, right_bottom, right_top, left_top;
    double             height0,     height1,      height2,   height3;
    pEllipsoidModel->convertXYZToLatLongHeight(ptLeftBottom.x(),  ptLeftBottom.y(),  ptLeftBottom.z(),  left_bottom.y(),  left_bottom.x(),  height0);
    pEllipsoidModel->convertXYZToLatLongHeight(ptRightBottom.x(), ptRightBottom.y(), ptRightBottom.z(), right_bottom.y(), right_bottom.x(), height1);
    pEllipsoidModel->convertXYZToLatLongHeight(ptRightTop.x(),    ptRightTop.y(),    ptRightTop.z(),    right_top.y(),    right_top.x(),    height2);
    pEllipsoidModel->convertXYZToLatLongHeight(ptLeftTop.x(),     ptLeftTop.y(),     ptLeftTop.z(),     left_top.y(),     left_top.x(),     height3);

    polygon.clear();
    polygon.addVertex(left_bottom);
    polygon.addVertex(right_bottom);
    polygon.addVertex(right_top);
    polygon.addVertex(left_top);
}


std::string Utility::getVersion(void) const
{
    const std::string strVersion = "9999";
    return strVersion;
}


OpenThreads::Mutex EarthLightModel::ms_Mutex;
const std::string EarthLightModel::ms_strTexture[8] = {"gu_tex0", "gu_tex1", "gu_tex2", "gu_tex3", "gu_tex4", "gu_tex5", "gu_tex6", "gu_tex7"};

const std::string EarthLightModel::ms_strHasTexture[8]  = {"gu_bHasTex0", "gu_bHasTex1", "gu_bHasTex2", "gu_bHasTex3", "gu_bHasTex4", "gu_bHasTex5", "gu_bHasTex6", "gu_bHasTex7"};

const std::string EarthLightModel::ms_strVertexShader =
    "uniform bool gu_bHasTex0;\n"
    "uniform bool gu_bHasTex1;\n"
    "uniform bool gu_bHasTex2;\n"
    "uniform bool gu_bHasTex3;\n"
    "uniform bool gu_bHasTex4;\n"
    "uniform bool gu_bHasTex5;\n"
    "uniform bool gu_bHasTex6;\n"
    "uniform bool gu_bHasTex7;\n"

    "varying vec2 gv_ptTexcoord0;\n"
    "varying vec2 gv_ptTexcoord1;\n"
    "varying vec2 gv_ptTexcoord2;\n"
    "varying vec2 gv_ptTexcoord3;\n"
    "varying vec2 gv_ptTexcoord4;\n"
    "varying vec2 gv_ptTexcoord5;\n"
    "varying vec2 gv_ptTexcoord6;\n"
    "varying vec2 gv_ptTexcoord7;\n"

    "varying vec4 gv_vecLightIndensity;\n"

    "void main(void)\n"
    "{\n"
    "    if(gu_bHasTex0)\n"
    "    {\n"
    "        gv_ptTexcoord0   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;\n"
    "    if(gu_bHasTex1)\n"
    "        {\n"
    "            gv_ptTexcoord1   = (gl_TextureMatrix[1] * gl_MultiTexCoord1).xy;\n"
    "            if(gu_bHasTex2)\n"
    "            {\n"
    "                gv_ptTexcoord2   = (gl_TextureMatrix[2] * gl_MultiTexCoord2).xy;\n"
    "                if(gu_bHasTex3)\n"
    "                {\n"
    "                    gv_ptTexcoord3   = (gl_TextureMatrix[3] * gl_MultiTexCoord3).xy;\n"
    "                    if(gu_bHasTex4)\n"
    "                    {\n"
    "                        gv_ptTexcoord4   = (gl_TextureMatrix[4] * gl_MultiTexCoord4).xy;\n"
    "                        if(gu_bHasTex5)\n"
    "                        {\n"
    "                            gv_ptTexcoord5   = (gl_TextureMatrix[5] * gl_MultiTexCoord5).xy;\n"
    "                            if(gu_bHasTex6)\n"
    "                            {\n"
    "                                gv_ptTexcoord6   = (gl_TextureMatrix[6] * gl_MultiTexCoord6).xy;\n"
    "                            }\n"
    "                        }\n"
    "                    }\n"
    "                }\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    if(gu_bHasTex7)\n"
    "    {\n"
    "        gv_ptTexcoord7   = (gl_TextureMatrix[7] * gl_MultiTexCoord7).xy;\n"
    "    }\n"

    "    vec3 vecNormal   = normalize(gl_NormalMatrix * gl_Normal).xyz;\n"
    "    vec3 vecLightDir = normalize(gl_LightSource[0].position.xyz);\n"
    "    float fltDot     = dot(vecLightDir, vecNormal);\n"
    "    float fltLight   = max(fltDot, 0.0);\n"
    "    gv_vecLightIndensity = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * fltLight\n"
    "                         + gl_LightSource[0].ambient * gl_FrontMaterial.ambient;\n"
    "    gl_Position = ftransform();\n"
    "}\n";

const std::string EarthLightModel::ms_strFragmentShader =
    "uniform sampler2D gu_tex0;\n"
    "uniform sampler2D gu_tex1;\n"
    "uniform sampler2D gu_tex2;\n"
    "uniform sampler2D gu_tex3;\n"
    "uniform sampler2D gu_tex4;\n"
    "uniform sampler2D gu_tex5;\n"
    "uniform sampler2D gu_tex6;\n"
    "uniform sampler2D gu_tex7;\n"

    "uniform bool gu_bHasTex0;\n"
    "uniform bool gu_bHasTex1;\n"
    "uniform bool gu_bHasTex2;\n"
    "uniform bool gu_bHasTex3;\n"
    "uniform bool gu_bHasTex4;\n"
    "uniform bool gu_bHasTex5;\n"
    "uniform bool gu_bHasTex6;\n"
    "uniform bool gu_bHasTex7;\n"

    "varying vec2 gv_ptTexcoord0;\n"
    "varying vec2 gv_ptTexcoord1;\n"
    "varying vec2 gv_ptTexcoord2;\n"
    "varying vec2 gv_ptTexcoord3;\n"
    "varying vec2 gv_ptTexcoord4;\n"
    "varying vec2 gv_ptTexcoord5;\n"
    "varying vec2 gv_ptTexcoord6;\n"
    "varying vec2 gv_ptTexcoord7;\n"
    "varying vec4 gv_vecLightIndensity;\n"

    "void main(void)\n"
    "{\n"
    "    vec4 color0, color1;\n"
    "    color0 = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "    if(gu_bHasTex0)\n"
    "    {\n"
    "        color1 = texture2D(gu_tex0, gv_ptTexcoord0);\n"
    "        if(color1.w == 1.0)\n"
    "        {\n"
    "            color0 = color1;\n"
    "        }\n"

    "        if(gu_bHasTex1)\n"
    "        {\n"
    "            color1 = texture2D(gu_tex1, gv_ptTexcoord1);\n"
    "            if(color1.w == 1.0)\n"
    "            {\n"
    "                color0 = color1;\n"
    "            }\n"

    "            if(gu_bHasTex2)\n"
    "            {\n"
    "                color1 = texture2D(gu_tex2, gv_ptTexcoord2);\n"
    "                if(color1.w == 1.0)\n"
    "                {\n"
    "                    color0 = color1;\n"
    "                }\n"

    "                if(gu_bHasTex3)\n"
    "                {\n"
    "                    color1 = texture2D(gu_tex3, gv_ptTexcoord3);\n"
    "                    if(color1.w == 1.0)\n"
    "                    {\n"
    "                        color0 = color1;\n"
    "                    }\n"

    "                    if(gu_bHasTex4)\n"
    "                    {\n"
    "                        color1 = texture2D(gu_tex4, gv_ptTexcoord4);\n"
    "                        if(color1.w == 1.0)\n"
    "                        {\n"
    "                            color0 = color1;\n"
    "                        }\n"

    "                        if(gu_bHasTex5)\n"
    "                        {\n"
    "                            color1 = texture2D(gu_tex5, gv_ptTexcoord5);\n"
    "                            if(color1.w == 1.0)\n"
    "                            {\n"
    "                                color0 = color1;\n"
    "                            }\n"

    "                            if(gu_bHasTex6)\n"
    "                            {\n"
    "                                color1 = texture2D(gu_tex6, gv_ptTexcoord6);\n"
    "                                if(color1.w == 1.0)\n"
    "                                {\n"
    "                                    color0 = color1;\n"
    "                                }\n"
    "                            }\n"
    "                        }\n"
    "                    }\n"
    "                }\n"
    "            }\n"
    "        }\n"
    "    }\n"

    "    if(gu_bHasTex7)\n"
    "    {\n"
    "        color1 = texture2D(gu_tex7, gv_ptTexcoord7);\n"
    "        color0 = mix(color0, color1, color1.w);\n"
    "    }\n"

    "    gl_FragColor.x = color0.x * gv_vecLightIndensity.x;\n"
    "    gl_FragColor.y = color0.y * gv_vecLightIndensity.y;\n"
    "    gl_FragColor.z = color0.z * gv_vecLightIndensity.z;\n"
    "    gl_FragColor.w = color0.w * gv_vecLightIndensity.w;\n"
    "}\n";


osg::ref_ptr<osg::Program>  EarthLightModel::ms_pProgram = NULL;

EarthLightModel __EarthLightModel;

EarthLightModel::EarthLightModel(void)
{
    EarthLightModel::createEarthLightModel();
}


bool EarthLightModel::bindEarthLightModel(osg::StateSet *pStateSet, char T)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    if(!pStateSet)   return false;

    setSampleStatus(pStateSet, T);
    pStateSet->setAttributeAndModes(ms_pProgram.get(), osg::StateAttribute::ON);
    return true;
}


void EarthLightModel::unBindEarthLightModel(osg::StateSet *pStateSet)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    if(!pStateSet)   return;

    pStateSet->removeAttribute(osg::StateAttribute::PROGRAM);
    resetSampleStatus(pStateSet);
}


void EarthLightModel::setSampleStatus(osg::StateSet *pStateSet, char T)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    pStateSet->getOrCreateUniform(ms_strTexture[0], osg::Uniform::SAMPLER_2D)->set(0);
    pStateSet->getOrCreateUniform(ms_strTexture[1], osg::Uniform::SAMPLER_2D)->set(1);
    pStateSet->getOrCreateUniform(ms_strTexture[2], osg::Uniform::SAMPLER_2D)->set(2);
    pStateSet->getOrCreateUniform(ms_strTexture[3], osg::Uniform::SAMPLER_2D)->set(3);
    pStateSet->getOrCreateUniform(ms_strTexture[4], osg::Uniform::SAMPLER_2D)->set(4);
    pStateSet->getOrCreateUniform(ms_strTexture[5], osg::Uniform::SAMPLER_2D)->set(5);
    pStateSet->getOrCreateUniform(ms_strTexture[6], osg::Uniform::SAMPLER_2D)->set(6);
    pStateSet->getOrCreateUniform(ms_strTexture[7], osg::Uniform::SAMPLER_2D)->set(7);

    bool b;
    b = (T & 0x00000001) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[0], osg::Uniform::BOOL)->set(b);

    b = (T & 0x00000010) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[1], osg::Uniform::BOOL)->set((T & 0x00000010) == 0 ? false : true);

    b = (T & 0x00000100) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[2], osg::Uniform::BOOL)->set((T & 0x00000100) == 0 ? false : true);

    b = (T & 0x00001000) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[3], osg::Uniform::BOOL)->set((T & 0x00001000) == 0 ? false : true);

    b = (T & 0x00010000) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[4], osg::Uniform::BOOL)->set((T & 0x00010000) == 0 ? false : true);

    b = (T & 0x00100000) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[5], osg::Uniform::BOOL)->set((T & 0x00100000) == 0 ? false : true);

    b = (T & 0x01000000) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[6], osg::Uniform::BOOL)->set((T & 0x01000000) == 0 ? false : true);

    b = (T & 0x10000000) == 0 ? false : true;
    pStateSet->getOrCreateUniform(ms_strHasTexture[7], osg::Uniform::BOOL)->set((T & 0x10000000) == 0 ? false : true);
}


void EarthLightModel::setSampleStatus(osg::StateSet *pStateSet, unsigned int nIndex, bool hasTexture)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    if(nIndex >= 8) return;
    pStateSet->getOrCreateUniform(ms_strHasTexture[nIndex], osg::Uniform::BOOL)->set(hasTexture);
}

std::string EarthLightModel::getHasTextrueName(unsigned int nIndex)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    if(nIndex >= 8) return "";
    return ms_strHasTexture[nIndex];
}

void EarthLightModel::resetSampleStatus(osg::StateSet *pStateSet)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ms_Mutex);
    pStateSet->removeUniform(ms_strTexture[0]);
    pStateSet->removeUniform(ms_strTexture[1]);
    pStateSet->removeUniform(ms_strTexture[2]);
    pStateSet->removeUniform(ms_strTexture[3]);
    pStateSet->removeUniform(ms_strTexture[4]);
    pStateSet->removeUniform(ms_strTexture[5]);
    pStateSet->removeUniform(ms_strTexture[6]);
    pStateSet->removeUniform(ms_strTexture[7]);
    pStateSet->removeUniform(ms_strHasTexture[0]);
    pStateSet->removeUniform(ms_strHasTexture[1]);
    pStateSet->removeUniform(ms_strHasTexture[2]);
    pStateSet->removeUniform(ms_strHasTexture[3]);
    pStateSet->removeUniform(ms_strHasTexture[4]);
    pStateSet->removeUniform(ms_strHasTexture[5]);
    pStateSet->removeUniform(ms_strHasTexture[6]);
    pStateSet->removeUniform(ms_strHasTexture[7]);
}


void EarthLightModel::createEarthLightModel(void)
{
    if(ms_pProgram.valid()) return;

    ms_pProgram = new osg::Program;

    osg::ref_ptr<osg::Shader> pVertexShader = new osg::Shader(osg::Shader::VERTEX, ms_strVertexShader);
    ms_pProgram->addShader(pVertexShader.get());

    osg::ref_ptr<osg::Shader> pFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, ms_strFragmentShader);
    ms_pProgram->addShader(pFragmentShader.get());
}


bool computeIntersection(const osg::Node *pIntersectTargetNode, const osg::Camera *pCamera, const osg::Vec2d &ptMousePos, osg::Vec3d &vIntersection, osg::Node **ppInterNode)
{
    osgUtil::Radial3 rayMouse;
    const bool bHasRadial = getScreenRadial(pCamera, ptMousePos, rayMouse);
    if(!bHasRadial)
    {
        return false;
    }

    osg::Vec3d  ptHitTest;
    if(!hitScene(pIntersectTargetNode, pCamera, rayMouse, ptHitTest, ppInterNode))
    {
        return false;
    }

    vIntersection = ptHitTest;
    return true;
}


bool getScreenRadial(const osg::Camera *pCamera, const osg::Vec2d &ptPosNormalize, osgUtil::Radial3 &ray)
{
    const osg::Matrixd &mtxProj      = pCamera->getProjectionMatrix();
    const osg::Matrixd &mtxView      = pCamera->getViewMatrix();
    const osg::Matrixd &mtxInverseVP = osg::Matrixd::inverse(mtxView * mtxProj);

    const osg::Vec3d ptNear(ptPosNormalize.x(), ptPosNormalize.y(), -1.0f);
    const osg::Vec3d ptMid(ptPosNormalize.x(), ptPosNormalize.y(), 0.0f);

    const osg::Vec3d ptRayBegin = ptNear * mtxInverseVP;
    const osg::Vec3d ptRayEnd   = ptMid  * mtxInverseVP;
    osg::Vec3d       vecRayDir  = ptRayEnd - ptRayBegin;
    const double     fltLen     = vecRayDir.normalize();
    if(!vecRayDir.valid())      return false;
    if(fltLen <= FLT_EPSILON)   return false;

    ray.setOrigin(ptRayBegin);
    ray.setDirection(vecRayDir);

    return true;
}


bool hitScene(const osg::Node *pHitTargetNode, const osg::Camera *pCamera, const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, osg::Node **ppHitNode)
{
    const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
    osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e9));
    //pPicker->set
    osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
    pVisitor->setTraversalMask(0x00FFFFFF);
    const_cast<osg::Node *>(pHitTargetNode)->accept(*pVisitor);

    if(!pPicker->containsIntersections())
    {
        return false;
    }

    osg::Vec3d ptCameraPos, ptCenter, vecUp;
    pCamera->getViewMatrixAsLookAt(ptCameraPos, ptCenter, vecUp);

    const osgUtil::LineSegmentIntersector::Intersections &inters = pPicker->getIntersections();
    osgUtil::LineSegmentIntersector::Intersections::const_iterator itor = inters.begin();
    osg::Vec3d ptHit;
    double dblMin = FLT_MAX;
    osg::Node *pHitNode = NULL;
    for( ; itor != inters.end(); ++itor)
    {
        const osgUtil::LineSegmentIntersector::Intersection &inter = *itor;
        osg::Vec3d point = inter.getWorldIntersectPoint();
        const double dbl = (ptCameraPos - point).length();
        if(dbl < dblMin)
        {
            dblMin = dbl;
            ptHit = point;
            pHitNode = inter.nodePath[inter.nodePath.size() - 1];
        }
    }
    ptHitTest = ptHit;
    if(ppHitNode != NULL)
        *ppHitNode = pHitNode;
    return true;
}


osg::Vec3d calcWorldCoordByCameraPose(const CameraPose &pose)
{
    osg::Vec3d ptCameraPos;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(pose.m_dblPositionY, pose.m_dblPositionX, pose.m_dblHeight, ptCameraPos.x(), ptCameraPos.y(), ptCameraPos.z());
    return ptCameraPos;
}


osg::Vec3d calcCamDirByCameraPose(const CameraPose &pose)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(pose.m_dblPositionY, pose.m_dblPositionX);
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(pose.m_dblPositionY, pose.m_dblPositionX);

    const osg::Quat qtAzimuth(pose.m_dblAzimuthAngle, -vecPlumbLine);
    const osg::Vec3d vecFlatDir = qtAzimuth * vecEastern;

    const osg::Vec3d vecSouthern = vecPlumbLine ^ vecEastern;
    const osg::Vec3d vecAxis = qtAzimuth * vecSouthern;
    const osg::Quat qtPitch(pose.m_dblPitchAngle - osg::PI_2, vecAxis);

    const osg::Vec3d vecCameraDir = qtPitch * vecFlatDir;
    return vecCameraDir;
}


