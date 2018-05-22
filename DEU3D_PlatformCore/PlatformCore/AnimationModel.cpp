#include "AnimationModel.h"
#include <osgDB\readFile>
#include <osg/Quat>
#include <osg/Material>
#include <osg/Switch>

#include <osg/CoordinateSystemNode>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/CullFace>

osg::Node *createPlaneModel(void)
{
    osg::ref_ptr<osg::Box>              pBodyShape = new osg::Box(osg::Vec3d(0.0, 0.0, 0.0), 10.0, 2.0, 2.0);
    osg::ref_ptr<osg::ShapeDrawable>    pBody = new osg::ShapeDrawable(pBodyShape);

    osg::ref_ptr<osg::Cone>             pHeadShape = new osg::Cone(osg::Vec3d(5.0, 0.0, 0.0), 1.0, 5.0);
    pHeadShape->setRotation(osg::Quat(osg::PI_2, osg::Vec3d(0.0, 1.0, 0.0)));
    osg::ref_ptr<osg::ShapeDrawable>    pHead = new osg::ShapeDrawable(pHeadShape);

    osg::ref_ptr<osg::Box>              pTailShape = new osg::Box(osg::Vec3d(-3.0, 0.0, 2.0), 1.0, 1.0, 3.0);
    osg::ref_ptr<osg::ShapeDrawable>    pTail = new osg::ShapeDrawable(pTailShape);

    osg::ref_ptr<osg::Box>              pWingShape = new osg::Box(osg::Vec3d(0.0, 0.0, 0.0), 3.0, 6.0, 1.0);
    osg::ref_ptr<osg::ShapeDrawable>    pWing = new osg::ShapeDrawable(pWingShape);

    osg::ref_ptr<osg::Geode>            pGeode = new osg::Geode;
    pGeode->addDrawable(pBody);
    pGeode->addDrawable(pHead);
    pGeode->addDrawable(pTail);
    pGeode->addDrawable(pWing);

    return pGeode.release();
}


AnimationModel::AnimationModel(void)
    : m_ptObserverPos(0.0, 0.0, 0.0)
{
	m_pModelRoot = new osg::Group;
    m_pModelNode = new osg::MatrixTransform;
	m_pModelRoot->addChild(m_pModelNode);

    //m_pModelNode->addChild(createPlaneModel());

    m_bFirstPerson = false;

    m_bForcePrivatePitch    = true;
    m_dblPrivatePitch       = osg::PI_2;
    m_bForcePrivateAzimuth  = true;
    m_dblPrivateAzimuth     = osg::PI_2;

	createFrustum();
}


bool AnimationModel::loadFrom(const ID &idModel)
{
	osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(idModel);
    if(!pNode.valid())  return false;
    m_pModelNode->addChild(pNode);

    return true;
}

bool AnimationModel::loadFrom(const std::string &strLocalPath)
{
    osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(strLocalPath);
    if(!pNode.valid())  return false;
    m_pModelNode->addChild(pNode);

    return true;
}


CameraPose AnimationModel::getObserverPose(double dblPitchOffset, double dblAzimuthOffset) const
{

    if(m_ptObserverPos.length2() < 0.1)
    {
        return m_poseCurrent;
    }

    double dblAzimuthAngle    = m_poseCurrent.m_dblAzimuthAngle - dblAzimuthOffset;
    double dblPitchAngle      = m_poseCurrent.m_dblPitchAngle - dblPitchOffset;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptModelPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX, m_poseCurrent.m_dblHeight, ptModelPos.x(), ptModelPos.y(), ptModelPos.z());
    const osg::Vec3d vecModelPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX);
    const osg::Vec3d vecModelEastern   = pEllipsoidModel->computeLocalEastern(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX);

    const osg::Quat  qtModelAzimuth(-dblAzimuthAngle, vecModelPlumbLine);
    const osg::Vec3d vecModelForward = qtModelAzimuth * vecModelEastern;

    osg::Vec3d ptObserverPos = ptModelPos;
#if 0
    const osg::Vec3d vecModelSouthern = vecModelPlumbLine ^ vecModelEastern;
    ptObserverPos += vecModelEastern   *  m_ptObserverPos.x();
    ptObserverPos += vecModelSouthern  *  m_ptObserverPos.y();
    ptObserverPos += vecModelPlumbLine * -m_ptObserverPos.z();

#else
    osg::Vec3d vecModelRight  = vecModelPlumbLine ^ vecModelEastern;
    vecModelRight = qtModelAzimuth * vecModelRight;

    const osg::Quat  qtModelPitch(-(osg::PI_2 - dblPitchAngle), vecModelRight);

    const osg::Vec3d vecModelDir     = qtModelPitch   * vecModelForward;
    const osg::Vec3d vecModelUp      = vecModelRight  ^ vecModelDir;

    ptObserverPos += vecModelDir   * m_ptObserverPos.x();
    ptObserverPos += vecModelRight * m_ptObserverPos.y();
    ptObserverPos += vecModelUp    * m_ptObserverPos.z();

#endif

    osg::Vec3d vecObserverDir = ptModelPos - ptObserverPos;
    vecObserverDir.normalize();

    double dblLatitude, dblLongitude, dblHeight;
    pEllipsoidModel->convertXYZToLatLongHeight(ptObserverPos.x(), ptObserverPos.y(), ptObserverPos.z(), dblLatitude, dblLongitude, dblHeight);

    CameraPose poseObserver;
    poseObserver.m_dblPositionX     = dblLongitude;
    poseObserver.m_dblPositionY     = dblLatitude;
    poseObserver.m_dblHeight        = dblHeight;

    const osg::Vec3d vecObserverPlumbLine = pEllipsoidModel->computeLocalPlumbLine(dblLatitude, dblLongitude);
    const osg::Vec3d vecObserverEastern = pEllipsoidModel->computeLocalEastern(dblLatitude, dblLongitude);

    const double dblCosPitch        = vecObserverDir * vecObserverPlumbLine;
    poseObserver.m_dblPitchAngle    = acos(osg::clampBetween(dblCosPitch, -1.0, 1.0));

    osg::Vec3d vecObserverForward   = vecObserverDir - vecObserverPlumbLine * (vecObserverDir * vecObserverPlumbLine);
    const double dblLen = vecObserverForward.normalize();
    if(dblLen < 0.01)
    {
        // 若当前相机实线正朝下，则由于缺少必要的参量，无法计算出相机的Forward方向，则此时直接将飞机的Forward当作相机的Forward
        vecObserverForward = vecModelForward;
    }

    const double dblCosAzimuth      = vecObserverForward * vecObserverEastern;
    const double dblAzimuth         = acos(osg::clampBetween(dblCosAzimuth, -1.0, 1.0));

    const osg::Vec3d vecNorthern = vecObserverEastern ^ vecObserverPlumbLine;
    const double dbl = vecObserverForward * vecNorthern;
    if(dbl < 0.0)
    {
        poseObserver.m_dblAzimuthAngle = -dblAzimuth;
    }
    else
    {
        poseObserver.m_dblAzimuthAngle = dblAzimuth;
    }

    return poseObserver;
}

void AnimationModel::setPose(const CameraPose &pose)
{
    m_poseCurrent = pose;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d  ptPosition;
    pEllipsoidModel->convertLatLongHeightToXYZ(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX, m_poseCurrent.m_dblHeight, ptPosition.x(), ptPosition.y(), ptPosition.z());

    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX);
    const osg::Vec3d vecEastern   = pEllipsoidModel->computeLocalEastern(m_poseCurrent.m_dblPositionY, m_poseCurrent.m_dblPositionX);

    osg::Quat   qtRotation;

    osg::Quat   qtToEastern;
    qtToEastern.makeRotate(osg::Vec3d(1.0, 0.0, 0.0), vecEastern);
    qtRotation *= qtToEastern;

    osg::Quat   qtToPlumbLine;
    qtToPlumbLine.makeRotate(osg::Vec3d(0.0, 0.0, -1.0), vecPlumbLine);
    qtRotation *= qtToPlumbLine;

    osg::Quat qtAzimuth;
    if(m_bForcePrivateAzimuth)
    {
        qtAzimuth.makeRotate(m_dblPrivateAzimuth, -vecPlumbLine);
    }
    else
    {
        qtAzimuth.makeRotate(pose.m_dblAzimuthAngle, -vecPlumbLine);
    }
    qtRotation *= qtAzimuth;

    const osg::Quat qtHorzAxis(-osg::PI_2 + pose.m_dblAzimuthAngle, -vecPlumbLine);
    const osg::Vec3d vecHorzAxis = qtHorzAxis * vecEastern;
    osg::Quat qtPitch;
    if(m_bForcePrivatePitch)
    {
        qtPitch.makeRotate(-(osg::PI_2 - m_dblPrivatePitch), vecHorzAxis);
    }
    else
    {
        qtPitch.makeRotate(-(osg::PI_2 - pose.m_dblPitchAngle), vecHorzAxis);
    }
    qtRotation *= qtPitch;

    osg::Matrixd    mtx;
    mtx.setRotate(qtRotation);
    mtx.postMultTranslate(ptPosition);
    m_pModelNode->setMatrix(mtx);

	updateFrustum();
}

void AnimationModel::setFirstPerson(bool bFirst)
{
    m_bFirstPerson = bFirst;
}

bool AnimationModel::isFirstPerson(void) const
{
    return m_bFirstPerson;
}

void AnimationModel::setVisible(bool visible)
{
    m_pModelNode->setNodeMask(visible ? 0xffffffff : 0);
}

bool AnimationModel::getVisible(void) const
{
    return m_pModelNode->getNodeMask() == 0xffffffff;
}

void AnimationModel::createFrustum(void)
{
	std::vector<osg::Vec3d> vecVertices(4);

	osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);

	m_pFrustum = new osg::MatrixTransform;
	m_pModelRoot->addChild(m_pFrustum);
	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
	m_pFrustum->addChild(pGeode);

	osg::ref_ptr<osg::StateSet> pState = m_pFrustum->getOrCreateStateSet();
    pState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
    pState->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
    pState->setMode(GL_BLEND, osg::StateAttribute::ON);
    pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    {
        osg::ref_ptr<osg::Geometry>     pSideFace = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array>    pSideVtx = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array>    pSideNormal = new osg::Vec3Array;

		for(size_t i = 0; i < vecVertices.size(); i++)
        {
            const osg::Vec3d vtx0(0.0, 0.0, 0.0);
            const osg::Vec3d &vtx1 = vecVertices[i];
			const osg::Vec3d &vtx2 = vecVertices[i + 1 == vecVertices.size() ? 0 : i + 1];
            pSideVtx->push_back(vtx0);
            pSideVtx->push_back(vtx1);
            pSideVtx->push_back(vtx2);

            const osg::Vec3d vec1 = vtx1 - vtx0;
            const osg::Vec3d vec2 = vtx2 - vtx0;
            osg::Vec3d vecNormal = vec1 ^ vec2;
            vecNormal.normalize();
            pSideNormal->push_back(vecNormal);
        }

        pSideFace->setVertexArray(pSideVtx.get());
        pSideFace->setNormalArray(pSideNormal.get());
        pSideFace->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

		const osg::Vec4 vColor(0.0, 0.0, 1.0, 0.2);
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(vColor);
		pSideFace->setColorArray(color.get());
		pSideFace->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, pSideVtx->size());
        pSideFace->addPrimitiveSet(pDrawArrays.get());
        pGeode->addDrawable(pSideFace.get());
    }

    showFrustum(false);
}

void AnimationModel::showFrustum(bool bShow)		
{
	m_pFrustum->setNodeMask(bShow ? 0xffffffff : 0);
}

bool AnimationModel::isFrustumShown(void) const
{
	return m_pFrustum->getNodeMask() == 0xffffffff;
}

void AnimationModel::setFrustumColor(const cmm::FloatColor &clr)
{
	osg::Geode *pGeode = dynamic_cast<osg::Geode*>(m_pFrustum->getChild(0));
	osg::Geometry *pGeom = dynamic_cast<osg::Geometry*>(pGeode->getDrawable(0));
	osg::Vec4Array *pColorArray = dynamic_cast<osg::Vec4Array*>(pGeom->getColorArray());
	(*pColorArray)[0].set(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);
}

const cmm::FloatColor AnimationModel::getFrustumColor(void) const
{
	osg::Geode *pGeode = dynamic_cast<osg::Geode*>(m_pFrustum->getChild(0));
	osg::Geometry *pGeom = dynamic_cast<osg::Geometry*>(pGeode);
	osg::Vec4Array *pColorArray = dynamic_cast<osg::Vec4Array*>(pGeom->getColorArray());

	cmm::FloatColor clr;
	clr.m_fltR = (*pColorArray)[0].x();
	clr.m_fltG = (*pColorArray)[0].y();
	clr.m_fltB = (*pColorArray)[0].z();
	clr.m_fltA = (*pColorArray)[0].w();
	return clr;
}

void AnimationModel::updateFrustum()
{
    if (!isFrustumShown()) return;

	osg::Matrixd tmp;
	tmp.setTrans(m_pModelNode->getMatrix().getTrans());
	m_pFrustum->setMatrix(tmp);

	osg::Geode *pGeode		   = dynamic_cast<osg::Geode*>(m_pFrustum->getChild(0));
	osg::Geometry *pGeom	   = dynamic_cast<osg::Geometry*>(pGeode->getDrawable(0));
	osg::Vec3Array *pVerticies = dynamic_cast<osg::Vec3Array*>(pGeom->getVertexArray());
	osg::Vec3Array *pNormals   = dynamic_cast<osg::Vec3Array*>(pGeom->getNormalArray());
	std::vector<osg::Vec3d> vTransed;

	for(size_t i = 0; i < 4; i++)
	{
		osg::Vec3d xyz;
		osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(m_vecFrustum[i].x(), m_vecFrustum[i].y(), m_vecFrustum[i].z(), 
																   xyz.x(),xyz.y(),xyz.z());
		vTransed.push_back(xyz - tmp.getTrans());
	}

	for(size_t i = 0, j = 0; i < 4; i++)
    {
		const osg::Vec3d &vtx0 = osg::Vec3d(0,0,0);
        const osg::Vec3d &vtx1 = vTransed[i];
		const osg::Vec3d &vtx2 = vTransed[i + 1 == vTransed.size() ? 0 : i + 1];

		j++;
        (*pVerticies)[j++] = vtx1;
        (*pVerticies)[j++] = vtx2;

        const osg::Vec3d vec1 = vtx1 - vtx0;
        const osg::Vec3d vec2 = vtx2 - vtx0;
        osg::Vec3d vecNormal = vec1 ^ vec2;
        vecNormal.normalize();
        (*pNormals)[i] = vecNormal;
    }

	pGeom->dirtyDisplayList();
}

void AnimationModel::setFrustum(double x0, double y0, double z0, 
				   double x1, double y1, double z1, 
				   double x2, double y2, double z2, 
				   double x3, double y3, double z3)
{
	m_vecFrustum.clear();
	m_vecFrustum.push_back(osg::Vec3d(x0, y0, z0));
	m_vecFrustum.push_back(osg::Vec3d(x1, y1, z1));
	m_vecFrustum.push_back(osg::Vec3d(x2, y2, z2));
	m_vecFrustum.push_back(osg::Vec3d(x3, y3, z3));

	updateFrustum();
}

void AnimationModel::getFrustum(double &x0, double &y0, double &z0, 
					double &x1, double &y1, double &z1, 
					double &x2, double &y2, double &z2, 
					double &x3, double &y3, double &z3)const
{
	x0 = m_vecFrustum[0].x(); y0 = m_vecFrustum[0].y(); z0 = m_vecFrustum[0].z();
	x1 = m_vecFrustum[1].x(); y1 = m_vecFrustum[1].y(); z1 = m_vecFrustum[1].z();
	x2 = m_vecFrustum[2].x(); y2 = m_vecFrustum[2].y(); z2 = m_vecFrustum[2].z();
	x3 = m_vecFrustum[3].x(); y3 = m_vecFrustum[3].y(); z3 = m_vecFrustum[3].z();
}