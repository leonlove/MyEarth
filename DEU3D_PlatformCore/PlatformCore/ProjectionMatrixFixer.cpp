#include "ProjectionMatrixFixer.h"
#include <assert.h>
#include "Utility.h"
#include "CameraInfo.h"

ProjectionMatrixFixer::ProjectionMatrixFixer(void)
{
    m_dblNearAbove  = -1.0;
    m_dblClampRatio = 0.05;
}


ProjectionMatrixFixer::~ProjectionMatrixFixer(void)
{
}


void ProjectionMatrixFixer::setPlanetNode(const osg::Node *pNode)
{
    m_pPlanetNode = pNode;
}


bool ProjectionMatrixFixer::clampProjectionMatrixImplementation(const osg::Camera *pCamera, osg::Matrixf &mtxProjection, double &dblNearZ, double &dblFarZ) const
{
    return _clampProjectionMatrix(pCamera, mtxProjection, dblNearZ, dblFarZ);
}


bool ProjectionMatrixFixer::clampProjectionMatrixImplementation(const osg::Camera *pCamera, osg::Matrixd &mtxProjection, double &dblNearZ, double &dblFarZ) const
{
    return _clampProjectionMatrix(pCamera, mtxProjection, dblNearZ, dblFarZ);
}


template<class matrix_type>
double ProjectionMatrixFixer::computePerspectiveFovy(matrix_type &mtxProjection) const
{
    const double temp_near = mtxProjection(3, 2) / (mtxProjection(2, 2) - 1.0);

    const double top = temp_near * (1.0 + mtxProjection(2, 1)) / mtxProjection(1, 1);
    const double bottom = temp_near * (mtxProjection(2, 1) - 1.0) / mtxProjection(1, 1);
    const double fovy = atan2(top, temp_near) - atan2(bottom, temp_near);
    const double dblCosFovy = cos(fovy);
    return dblCosFovy;
}


double ProjectionMatrixFixer::computeCurrentCameraHeight(const osg::Camera *pCamera) const
{
    const CameraInfo *pCamInfo = CameraInfo::instance();
    const CameraPose &camPose = pCamInfo->getCameraPose(0u);
    if(!m_pPlanetNode.valid())
    {
        return camPose.m_dblHeight;
    }

    static const osg::Vec3d ptPlanetCenter(0.0, 0.0, 0.0);

    const osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptCameraPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(camPose.m_dblPositionY, camPose.m_dblPositionX, camPose.m_dblHeight, ptCameraPos.x(), ptCameraPos.y(), ptCameraPos.z());
    const osgUtil::Radial3    ray(ptPlanetCenter, ptCameraPos - ptPlanetCenter);

    double dblCameraHeight = camPose.m_dblHeight;
    osg::Vec3d ptHitTest;
    if(hitScene(m_pPlanetNode.get(), pCamera, ray, ptHitTest))
    {
        dblCameraHeight = (ptHitTest - ptCameraPos).length();
    }
    return dblCameraHeight;
}


template<class matrix_type>
bool ProjectionMatrixFixer::_clampProjectionMatrix(const osg::Camera *pCamera, matrix_type &mtxProjection, double &dblNearZ, double &dblFarZ) const
{
    const double epsilon = 1e-6;
    if (dblFarZ < dblNearZ - epsilon)
    {
        OSG_INFO << "_clampProjectionMatrix not applied, invalid depth range, dblNearZ = " << dblNearZ
                 << "  dblFarZ = "<< dblFarZ
                 << std::endl;
        return false;
    }

    if (dblFarZ < dblNearZ + epsilon)
    {
        // dblNearZ and dblFarZ are too close together and could cause divide by zero problems
        // late on in the clamping code, so move the dblNearZ and dblFarZ apart.
        const double average = (dblNearZ + dblFarZ) * 0.5;
        dblNearZ = average - epsilon;
        dblFarZ = average + epsilon;
        // OSG_INFO << "_clampProjectionMatrix widening dblNearZ and dblFarZ to "<<dblNearZ<<" "<<dblFarZ<<std::endl;
    }

    if (fabs(mtxProjection(0,3))<epsilon  && fabs(mtxProjection(1,3))<epsilon  && fabs(mtxProjection(2,3)) < epsilon )
    {
        // OSG_INFO << "Orthographic matrix before clamping"<<projection<<std::endl;

        double delta_span = (dblFarZ-dblNearZ)*0.02;
        if (delta_span<1.0) delta_span = 1.0;
        double desired_znear = dblNearZ - delta_span;
        double desired_zfar = dblFarZ + delta_span;

        // assign the clamped values back to the computed values.
        dblNearZ = desired_znear;
        dblFarZ = desired_zfar;

        mtxProjection(2,2)=-2.0f/(desired_zfar-desired_znear);
        mtxProjection(3,2)=-(desired_zfar+desired_znear)/(desired_zfar-desired_znear);

        // OSG_INFO << "Orthographic matrix after clamping "<<projection<<std::endl;
    }
    else
    {
        // OSG_INFO << "Persepective matrix before clamping"<<projection<<std::endl;

        const CameraInfo *pCamInfo = CameraInfo::instance();
        const CameraPose &camPose = pCamInfo->getCameraPose(0u);

        const double dblCosFovy = computePerspectiveFovy(mtxProjection);

        const osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        const double dblCameraHeight = computeCurrentCameraHeight(pCamera);

        //std::cout << dblCameraHeight - m_CameraPose.m_dblHeight << std::endl;

        const double dblFarPushRatio  = 1.0 - m_dblClampRatio * 0.4;
        const double dblNearPullRatio = 1.0 - m_dblClampRatio;

        double dblBoundFarZ  = dblFarZ  * dblFarPushRatio;
        double dblBoundNearZ = dblNearZ * dblNearPullRatio;

        // near plane clamping.
        const double dblPlanetRadius = pEllipsoidModel->getRadius();
        dblBoundFarZ = osg::clampBelow(dblBoundFarZ, camPose.m_dblHeight + dblPlanetRadius);
        dblBoundNearZ = osg::clampAbove(dblBoundNearZ, 0.15);

        const std::pair<double, double>   HeightGap(1000.0, 10000.0);
        const double dblMidHeightGap  = (HeightGap.first + HeightGap.second) * 0.5;
        const double dblActHeight     = dblCameraHeight * dblCosFovy;
        const double dblRatioOfHeight = (dblActHeight - dblMidHeightGap) / 1000.0;
        const double dblHeightWeight  = cmm::math::logsig(dblRatioOfHeight);
        const double dblBoundWeight   = 1.0 - dblHeightWeight;

        // assign the clamped values back to the computed values.
        dblFarZ  = osg::clampAbove(dblBoundFarZ, 10000.0);
        dblNearZ = dblBoundNearZ * dblBoundWeight + dblActHeight * dblHeightWeight;

        //测试近剪裁面距离经验值
        char *pNearZ = ::getenv("DEU_DEBUG_NEAR_EYE");
        if (pNearZ != NULL)
        {
            double dLen = atof(pNearZ);
            dblNearZ -= dLen;
        }

        dblNearZ = osg::clampAbove(dblNearZ, 0.15);

        //OSG_ALWAYS << "BoundNearZ = \t" << dblBoundNearZ << "\n"
        //           << "BoundWeight = \t" << dblBoundWeight << "\n"
        //           << "Height = \t" << m_CameraPose.m_dblHeight << "\n"
        //           << "HeightWeight = \t" << dblHeightWeight << "\n"
        //           << std::endl;

        const double trans_near_plane = (-dblNearZ * mtxProjection(2,2) + mtxProjection(3,2)) / (-dblNearZ * mtxProjection(2,3) + mtxProjection(3,3));
        const double trans_far_plane  = (-dblFarZ  * mtxProjection(2,2) + mtxProjection(3,2)) / (-dblFarZ  * mtxProjection(2,3) + mtxProjection(3,3));

        const double ratio  = fabs(2.0 / (trans_near_plane - trans_far_plane));
        const double center = -(trans_near_plane + trans_far_plane) / 2.0;

        mtxProjection.postMult(osg::Matrixd(1.0, 0.0, 0.0,            0.0,
                                            0.0, 1.0, 0.0,            0.0,
                                            0.0, 0.0, ratio,          0.0,
                                            0.0, 0.0, center * ratio, 1.0));

        //osg::ref_ptr<osgViewer::DepthPartitionSettings> pDepthPartitionSettings = m_pDepthPartitionSettings;
        //if(pDepthPartitionSettings.valid())
        //{
        //    pDepthPartitionSettings->_currentNearZ = dblNearZ;
        //    pDepthPartitionSettings->_currentFarZ  = dblFarZ;
        //}
    }

    return true;
}


template<class matrix_type>
bool ProjectionMatrixFixer::isPerspectiveMatrix(matrix_type &mtxProjection)
{
    const double dblEpsilon = 1e-6;
    const bool b1 = fabs(mtxProjection(0,3)) < dblEpsilon;
    const bool b2 = fabs(mtxProjection(1,3)) < dblEpsilon;
    const bool b3 = fabs(mtxProjection(2,3)) < dblEpsilon;
    if(b1 && b2 && b3)
    {
        return false;
    }
    return true;
}
