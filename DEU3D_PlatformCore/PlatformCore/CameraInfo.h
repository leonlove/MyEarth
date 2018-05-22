#ifndef CAMERA_INFO_H_E34A4293_DF39_46D3_BED7_7B6CAB32556C_INCLUDE
#define CAMERA_INFO_H_E34A4293_DF39_46D3_BED7_7B6CAB32556C_INCLUDE

#include <OpenSP/Ref.h>
#include <osg/Vec3d>
#include <osg/FrameStamp>
#include <osg/ref_ptr>
#include <map>
#include "NavigationParam.h"

class CameraInfo : public OpenSP::Ref
{
public:
    explicit CameraInfo(void);
    virtual ~CameraInfo(void);

    static CameraInfo *instance(void);

public:
    void                    setCameraInfo(unsigned nViewIndex,
                                unsigned nFrameNumber,
                                const CameraPose &pose,
                                bool bUnderGround,
                                const osg::Vec3d &ptCamGroundPos);
    const CameraPose       &getCameraPose(unsigned nViewIndex) const;
    osg::Vec3d              getCameraPosInWorld(unsigned nViewIndex) const;
    osg::Vec3d              getCameraDirection(unsigned nViewIndex) const;
    bool                    isCameraUnderGround(unsigned nViewIndex) const;
    const osg::Vec3d       &getCameraGroundPosInWorld(unsigned nViewIndex) const;
    unsigned                getFrameNumber(unsigned nViewIndex) const;

protected:
    struct Info
    {
        CameraPose      m_CameraPose;
        bool            m_bUnderGround;
        osg::Vec3d      m_ptCameraGroundPosInWorld;
        osg::Vec3d      m_ptCameraPos;
        osg::Vec3d      m_vecCameraDirection;
        unsigned        m_nFrameNumber;
    };

    std::vector<Info>           m_vecCameraInfo;
};

#endif

