#ifndef    __C_PYRAMIDS_H__
#define    __C_PYRAMIDS_H__

#include "Export.h"
#include "deuMath.h"

namespace cmm
{

class CM_EXPORT Pyramid
{
    const static unsigned MAXLEVEL = 32u;
public:
    Pyramid(void);
    //构造金字塔对象，传入参数
    //OX,OY: 原点坐标
    //XAxis, YAxis: X轴方向和Y轴方向，true表示正方向，false表示负方向
    //TopTileSizeX,TopTileSizeY: 顶层一个瓦片的大小，实际大小
    //TilePixelX,TilePixelY: 瓦片像素大小
    //LevelScale: 每层见的比例关系
    Pyramid(double OX, double OY, double TopTileSizeX, double TopTileSizeY);
    Pyramid(const Pyramid &Obj);

    static const Pyramid *instance(void);

public:
    const Pyramid &operator = (const Pyramid &Obj);

public:
    const math::Point2d &getOrigin(void) const;
    const math::Vector2d &getTopTileSize(void) const;
    const math::Vector2d &getLevelTileSize(unsigned nLevel) const;

    //通过层号和行列号得到瓦片的范围
    bool getTilePos(unsigned nLevel, unsigned nRow, unsigned nCol, double &xMin, double &yMin, double &xMax, double &yMax) const;

    //通过层号和点坐标得到行列号
    bool getTile(unsigned nLevel, double x, double y, unsigned &nRow, unsigned &nCol) const;

    //通过层号和数据范围得到行列号范围
    bool getTile(unsigned nLevel, double xMin, double yMin, double xMax, double yMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nRowMax, unsigned &nColMax) const;

    //通过数据范围，判断哪一层，哪一个瓦片，正好包含数据
    bool getTile(double xMin, double yMin, double xMax, double yMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol) const;

    //通过层号行号列号，得到父亲节点的层号行号列号
    bool getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent) const;

    //通过层号行号列号和指定的父亲层号，父亲层的节点的行号列号，要求父亲层号小宇当前层号
    bool getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent) const;

    // 获取某一层的最大行列号
    unsigned getMaxCol(unsigned nLevel) const;
    unsigned getMaxRow(unsigned nLevel) const;

protected:
    void init();

protected:
    math::Point2d   m_ptOrigin;
    math::Vector2d  m_vecTopTileSize;
    math::Vector2d  m_LevelTileSizes[MAXLEVEL];
};


class CM_EXPORT Pyramid3
{
    const static unsigned MAXLEVEL = 32u;
public:
    Pyramid3(void);
    Pyramid3(const math::Point3d &ptOrigin, const math::Vector3d &vecTopCubeSize);
    Pyramid3(const Pyramid3 &Obj);

public:
    const Pyramid3 &operator = (const Pyramid3 &Obj);

    static const Pyramid3 *instance(void);

    const math::Point3d &getOrigin(void) const;
    const math::Vector3d &getTopCubeSize(void) const;
    const math::Vector3d &getLevelCubeSize(unsigned level) const;

    //通过层号和行列号得到瓦片的范围
    bool getCubePos(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, cmm::math::Point3d &ptMin, cmm::math::Point3d &ptMax) const;

    //通过层号和点坐标得到行列号
    bool getCube(unsigned nLevel, const cmm::math::Point3d &point, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const;

    //通过层号和数据范围得到行列号范围
    bool getCube(unsigned nLevel, const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nHeightMin, unsigned &nRowMax, unsigned &nColMax, unsigned &nHeightMax) const;

    //通过数据范围，判断哪一层，哪一个瓦片，正好包含数据
    bool getCube(const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const;

    //通过层号行号列号，得到父亲节点的层号行号列号
    bool getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const;

    //通过层号行号列号和指定的父亲层号，父亲层的节点的行号列号，要求父亲层号小宇当前层号
    bool getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const;

    // 获取某一层的最大行列号
    unsigned getMaxCol(unsigned nLevel) const;
    unsigned getMaxRow(unsigned nLevel) const;
    unsigned getMaxHeight(unsigned nLevel) const;

protected:
    void init();

protected:
    math::Point3d   m_ptOrigin;
    math::Vector3d  m_vecTopCubeSize;
    math::Vector3d  m_LevelCubeSizes[MAXLEVEL];
};


}

#endif
