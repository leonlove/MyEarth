#ifndef I_EVENT_ADAPTER_H_31FED420_B7C9_417E_88EB_6D91F3CE683B_INCLUDE
#define I_EVENT_ADAPTER_H_31FED420_B7C9_417E_88EB_6D91F3CE683B_INCLUDE

#include <OpenSP/Ref.h>
#include "Export.h"

namespace ea
{
class IEventObject;
class IEventReceiver;
class IEventFilter;

//事件注册器
class IEventAdapter : public OpenSP::Ref
{
public:
    virtual void sendBroadcast(IEventObject *pEvent)                                          = 0;
    //注册一个事件接收器
    virtual bool registerReceiver(IEventReceiver *pEventReceiver, IEventFilter *pEventFilter) = 0;
    //反注册一个事件接收器
    virtual void unregisterReceiver(IEventReceiver *pEventReceiver)                           = 0;
};

EA_DLL_SPECIAL IEventAdapter *createEventAdapter(void);

extern EA_DLL_SPECIAL const std::string ACTION_POINT_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_LINE_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_POLYLINE_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_RECT_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_ELLIPSE_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_POLYGON_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_EDITING_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_NAVPATH_KEYFRAME;
extern EA_DLL_SPECIAL const std::string ACTION_CAMERA_POSE_CHANGED;

/*临时工具消息***********************************************************/
extern EA_DLL_SPECIAL const std::string ACTION_MEASURE_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_AREA_TOOL;
extern EA_DLL_SPECIAL const std::string ACTION_DISTANCE_TOOL;
/************************************************************************/

extern EA_DLL_SPECIAL const std::string ACTION_SUBMIT_DATA;
extern EA_DLL_SPECIAL const std::string ACTION_START_SERVICE;
extern EA_DLL_SPECIAL const std::string ACTION_STOP_SERVICE;
extern EA_DLL_SPECIAL const std::string ACTION_SUBMIT_BREAK;
extern EA_DLL_SPECIAL const std::string ACTION_TILE_BUILDER;
extern EA_DLL_SPECIAL const std::string ACTION_MODEL_BUILDER;

//extern EA_DLL_SPECIAL const std::string ACTION_;
	

}
#endif

