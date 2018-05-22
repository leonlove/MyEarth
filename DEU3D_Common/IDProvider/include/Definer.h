#ifndef DEFINER_H_DE02727E_D067_412B_A41F_6ADBDF5785F2_INCLUDE
#define DEFINER_H_DE02727E_D067_412B_A41F_6ADBDF5785F2_INCLUDE

enum DeuObjectIDType
{
    VIRTUAL_TILE                      = 0,                       // 虚拟瓦片
    VIRTUAL_CUBE                      = 0,                       // 虚拟块片
    TERRAIN_TILE                      = 1,                       // 混合的地表瓦片
    TERRAIN_TILE_HEIGHT_FIELD         = 2,                       // 地形高程瓦片
    TERRAIN_TILE_IMAGE                = 3,                       // 地形影像瓦片

    V_TILE_PROXY_GROUP_ID             = 20,
    V_TILE_FRAGMENT_ID                = 21,                      // 虚拟瓦片片段的ID
    V_TILE_PROXY_NODE_ID              = 22,

    V_CUBE_FRAGMENT_GROUP_ID          = 23,                      // 虚拟方块片段组的ID
    V_CUBE_FRAGMENT_NODE_ID           = 24,                      // 虚拟方块片段的ID

    MODEL_ID                          = 32,                      // 普通模型
    IMAGE_ID                          = 33,                      // 图像
    SHARE_IMAGE_ID                    = 34,                      // 共享图片
    SHARE_MODEL_ID                    = 35,                      // 共享模型

    CULTURE_LAYER_ID                  = 40,                      // 逻辑图层
    TERRAIN_DEM_LAYER_ID              = 41,                      // 高程图层
    TERRAIN_DOM_LAYER_ID              = 42,                      // 影像图层
    SYMBOL_CATEGORY_ID                = 43,

    TERRAIN_DEM_ID                    = 50,                      // 高程更新
    TERRAIN_DOM_ID                    = 51,                      // 影像更新
    SYMBOL_ID                         = 52,                      // 图像符号类型
    PARAM_POINT_ID                    = 53,                      // 点标注
    PARAM_LINE_ID                     = 54,                      // 线标注
    PARAM_FACE_ID                     = 55,                      // 面标注

    TERRAIN_COLOR_MODIFICATION        = 70,
    TERRAIN_ELEVATION_MODIFICATION    = 71,
    TERRAIN_DOM_MODIFICATION          = 72,
    TERRAIN_DEM_MODIFICATION          = 73,

    DETAIL_PIPE_CONNECTOR_ID          = 110,
    DETAIL_CUBE_ID                    = 111,
    DETAIL_CYLINDER_ID                = 112,
    DETAIL_PRISM_ID                   = 113,
    DETAIL_PYRAMID_ID                 = 114,
    DETAIL_SPHERE_ID                  = 115,
    DETAIL_SECTOR_ID                  = 116,
    DETAIL_STATIC_MODEL_ID            = 117,
    DETAIL_DYN_POINT_ID               = 118,
    DETAIL_DYN_LINE_ID                = 119,
    DETAIL_DYN_FACE_ID                = 120,
    DETAIL_DYN_IMAGE_ID               = 121,
    DETAIL_BUBBLE_TEXT_ID             = 122,
    DETAIL_POLYGON_ID                 = 123,
    DETAIL_ROUND_TABLE_ID             = 124,
    DETAIL_RECT_PIPE_CONNECTOR_ID     = 125,
    DETAIL_POLYGON_PIPE_CONNECTOR_ID  = 126,
	DETAIL_DYN_POINT_CLOUD_ID         = 127,

    INST_ID_2_PARENT_ID               = 200,                       // 通过对象ID，查找其父图层的ID列表
    CULTURE_LAYER_ID_2_PARENT_ID      = 201,
    DEM_LAYER_ID_2_PARENT_ID          = 202,
    DOM_LAYER_ID_2_PARENT_ID          = 203,

    UNKNOWN_TPYE                      = 0xFF              // 无效类型
};

#endif