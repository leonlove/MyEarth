#ifndef I_NAVIGATION_PARAM_H_4F278D24_78D7_4B5D_8088_A1184D171417_INCLUDE
#define I_NAVIGATION_PARAM_H_4F278D24_78D7_4B5D_8088_A1184D171417_INCLUDE

#include <OpenSP/Ref.h>
#include <map>
#include <vector>
#include "Export.h"
#include <Common\DEUBson.h>
#include <OpenSP/sp.h>
#include <common/IDEUImage.h>

class PLATFORM_EXPORT NavigationParam : public OpenSP::Ref
{
public:
    double      m_dblKeyboardTranslationSpeed;  // 键盘的平移速度
    double      m_dblKeyboardRotationSpeed;     // 键盘的旋转速度
    double      m_dblShiftLMag;                 // 按下左Shift之后，速度放大的倍率
    double      m_dblShiftRMag;                 // 按下右Shift之后，速度放大的倍率

    double      m_dblFrictionalFactor;          // 惯性摩擦系数
    bool        m_bMouseInertia;                // 是否支持鼠标惯性漫游
    bool        m_bKeyboardInertia;             // 是否支持键盘惯性漫游

    double      m_dblKeyboardSpeedBindHeight;   // 键盘的速度与高度的相关比率

    bool        m_bLockWithTerrain;             // 漫游时是否跟随地形高程
    double      m_dblLockTerrainHeight;         // 漫游时若跟随地形高程，指定跟随的最小高度

	bool		m_bUnderGroundViewMode;			// 是否支持地下浏览模式

    enum NavKeyboard
    {
        NK_Shift_L,
        NK_Shift_R,

        NK_Control_L,
        NK_Control_R,
        NK_ResetCamera,
        NK_Forward,
        NK_Backward,
        NK_Left,
        NK_Right,
        NK_Up,
        NK_Down,
        NK_LookUp,
        NK_LookDown,
        NK_RotateRight,
        NK_RotateLeft,
        NK_RotateRight_V,
        NK_RotateLeft_V,
        NK_StopInertia,
        NK_NavPath_Pause,
        NK_NavPath_StepForward,
        NK_NavPath_StepBackward
    };

    std::map<NavKeyboard, int>        m_NavKeyboardConfig;    // 键盘配置

    explicit NavigationParam(void);
             NavigationParam(const NavigationParam &param);

    void     Default(void);
    void     setParam(const NavigationParam &param);
    const    NavigationParam &operator=(const NavigationParam &param);
    bool     isEqual(const NavigationParam &param) const;
    bool     operator==(const NavigationParam &param) const;
    bool     operator!=(const NavigationParam &param) const;
};


struct CameraPose
{
    double        m_dblPositionX;            // the x-position of camera:
                                        // if current scene is a planet scene, it represents the longitude of camera position, degree unit

    double        m_dblPositionY;            // the y-position of camera:
                                        // if current scene is a planet scene, it represents the latitude of camera position, degree unit

    double        m_dblHeight;            // the height of camera, or we can say it altitude

    double        m_dblPitchAngle;        // the angle between sight line and plumbline, it must be in [0, PI]

    double        m_dblAzimuthAngle;        // the angle between 'PROJECTION SIGHT LINE' and 'EASTERN DIRECTION', it must be in[-PI, PI]
                                        // PROJECTION SIGHT LINE: project the sight line on a horizontal plane, the shadow is named PROJECTION SIGHT LINE
                                        // EASTERN DIRECTION: the eastern direction of current camera position

    inline bool operator==(const CameraPose &param) const
    {
        if(!cmm::math::floatEqual(m_dblPositionX, param.m_dblPositionX, 1E-8))
        {
            return false;
        }
        if(!cmm::math::floatEqual(m_dblPositionY, param.m_dblPositionY, 1E-8))
        {
            return false;
        }
        if(!cmm::math::floatEqual(m_dblHeight, param.m_dblHeight, 1E-3))
        {
            return false;
        }
        if(!cmm::math::floatEqual(m_dblPitchAngle, param.m_dblPitchAngle, 1E-6))
        {
            return false;
        }
        if(!cmm::math::floatEqual(m_dblAzimuthAngle, param.m_dblAzimuthAngle, 1E-6))
        {
            return false;
        }
        return true;
    }

    inline bool operator!=(const CameraPose &param) const
    {
        return !operator==(param);
    }
};
typedef std::vector<CameraPose>        CameraPoseList;


#endif
