#ifndef DEFINER_H_DE02727E_D067_412B_A41F_6ADBDF5785F2_INCLUDE
#define DEFINER_H_DE02727E_D067_412B_A41F_6ADBDF5785F2_INCLUDE

enum DeuObjectIDType
{
    VIRTUAL_TILE                      = 0,                       // ������Ƭ
    VIRTUAL_CUBE                      = 0,                       // �����Ƭ
    TERRAIN_TILE                      = 1,                       // ��ϵĵر���Ƭ
    TERRAIN_TILE_HEIGHT_FIELD         = 2,                       // ���θ߳���Ƭ
    TERRAIN_TILE_IMAGE                = 3,                       // ����Ӱ����Ƭ

    V_TILE_PROXY_GROUP_ID             = 20,
    V_TILE_FRAGMENT_ID                = 21,                      // ������ƬƬ�ε�ID
    V_TILE_PROXY_NODE_ID              = 22,

    V_CUBE_FRAGMENT_GROUP_ID          = 23,                      // ���ⷽ��Ƭ�����ID
    V_CUBE_FRAGMENT_NODE_ID           = 24,                      // ���ⷽ��Ƭ�ε�ID

    MODEL_ID                          = 32,                      // ��ͨģ��
    IMAGE_ID                          = 33,                      // ͼ��
    SHARE_IMAGE_ID                    = 34,                      // ����ͼƬ
    SHARE_MODEL_ID                    = 35,                      // ����ģ��

    CULTURE_LAYER_ID                  = 40,                      // �߼�ͼ��
    TERRAIN_DEM_LAYER_ID              = 41,                      // �߳�ͼ��
    TERRAIN_DOM_LAYER_ID              = 42,                      // Ӱ��ͼ��
    SYMBOL_CATEGORY_ID                = 43,

    TERRAIN_DEM_ID                    = 50,                      // �̸߳���
    TERRAIN_DOM_ID                    = 51,                      // Ӱ�����
    SYMBOL_ID                         = 52,                      // ͼ���������
    PARAM_POINT_ID                    = 53,                      // ���ע
    PARAM_LINE_ID                     = 54,                      // �߱�ע
    PARAM_FACE_ID                     = 55,                      // ���ע

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

    INST_ID_2_PARENT_ID               = 200,                       // ͨ������ID�������丸ͼ���ID�б�
    CULTURE_LAYER_ID_2_PARENT_ID      = 201,
    DEM_LAYER_ID_2_PARENT_ID          = 202,
    DOM_LAYER_ID_2_PARENT_ID          = 203,

    UNKNOWN_TPYE                      = 0xFF              // ��Ч����
};

#endif