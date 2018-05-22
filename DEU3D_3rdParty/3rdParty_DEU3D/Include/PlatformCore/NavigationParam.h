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
    double      m_dblKeyboardTranslationSpeed;  // ���̵�ƽ���ٶ�
    double      m_dblKeyboardRotationSpeed;     // ���̵���ת�ٶ�
    double      m_dblShiftLMag;                 // ������Shift֮���ٶȷŴ�ı���
    double      m_dblShiftRMag;                 // ������Shift֮���ٶȷŴ�ı���

    double      m_dblFrictionalFactor;          // ����Ħ��ϵ��
    bool        m_bMouseInertia;                // �Ƿ�֧������������
    bool        m_bKeyboardInertia;             // �Ƿ�֧�ּ��̹�������

    double      m_dblKeyboardSpeedBindHeight;   // ���̵��ٶ���߶ȵ���ر���

    bool        m_bLockWithTerrain;             // ����ʱ�Ƿ������θ߳�
    double      m_dblLockTerrainHeight;         // ����ʱ��������θ̣߳�ָ���������С�߶�

	bool		m_bUnderGroundViewMode;			// �Ƿ�֧�ֵ������ģʽ

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

    std::map<NavKeyboard, int>        m_NavKeyboardConfig;    // ��������

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
