#ifndef _WFS_DEFINE_H_4169C33E_6951_4A98_9317_A52747C07D8B_
#define _WFS_DEFINE_H_4169C33E_6951_4A98_9317_A52747C07D8B_

namespace wfsb
{
    struct BBoxInfo
    {
        double dxmin;
        double dymin;
        double dxmax;
        double dymax;
    };

    enum GMType
    {
        GM_POINT = 0,
        GM_LINE  = 1,
        GM_FACE  = 2
    };

    struct TileRange
    {
        unsigned     m_fromTile;
        unsigned     m_toTile;
    };
}

#endif //_WFS_DEFINE_H_4169C33E_6951_4A98_9317_A52747C07D8B_