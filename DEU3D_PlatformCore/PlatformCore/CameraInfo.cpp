#include "CameraInfo.h"
#include <OpenSP/sp.h>
#include "Utility.h"

class UniqueCameraInfoInstance
{
public:
    UniqueCameraInfoInstance(void)
    {
        CameraInfo::instance();
    }
}__UniqueCameraInfoInstance;


CameraInfo *CameraInfo::instance(void)
{
    static OpenSP::sp<CameraInfo>  spCameraInfo = new CameraInfo;
    return spCameraInfo.get();
}


CameraInfo::CameraInfo(void)
{
}


CameraInfo::~CameraInfo(void)
{
}


void CameraInfo::setCameraInfo(unsigned nViewIndex,
                            unsigned nFrameNumber,
                            const CameraPose &pose,
                            bool bUnderGround,
                            const osg::Vec3d &ptCamGroundPos)
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        m_vecCameraInfo.resize(nViewIndex + 1u);
    }

    Info &info = m_vecCameraInfo[nViewIndex];
    if(info.m_CameraPose == pose)
    {
        return;
    }

    info.m_CameraPose   = pose;
    info.m_nFrameNumber = nFrameNumber;
    info.m_bUnderGround = bUnderGround;
    info.m_ptCameraGroundPosInWorld = ptCamGroundPos;
    info.m_vecCameraDirection = calcCamDirByCameraPose(pose);
    info.m_ptCameraPos = calcWorldCoordByCameraPose(pose);
}


const CameraPose &CameraInfo::getCameraPose(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        static const CameraPose pose = {0.0, 0.0, 0.0, 0.0, 0.0};
        return pose;
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_CameraPose;
}


osg::Vec3d CameraInfo::getCameraPosInWorld(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        return osg::Vec3d(0.0, 0.0, 0.0);
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_ptCameraPos;
}


osg::Vec3d CameraInfo::getCameraDirection(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        return osg::Vec3d(0.0, 0.0, 0.0);
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_vecCameraDirection;
}


const osg::Vec3d &CameraInfo::getCameraGroundPosInWorld(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        const static osg::Vec3d point(0.0, 0.0, 0.0);
        return point;
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_ptCameraGroundPosInWorld;
}


bool CameraInfo::isCameraUnderGround(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        return false;
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_bUnderGround;
}


unsigned CameraInfo::getFrameNumber(unsigned nViewIndex) const
{
    if(nViewIndex >= m_vecCameraInfo.size())
    {
        return 0u;
    }

    const Info &info = m_vecCameraInfo[nViewIndex];
    return info.m_nFrameNumber;
}

