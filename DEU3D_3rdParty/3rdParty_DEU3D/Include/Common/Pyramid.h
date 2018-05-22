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
    //������������󣬴������
    //OX,OY: ԭ������
    //XAxis, YAxis: X�᷽���Y�᷽��true��ʾ������false��ʾ������
    //TopTileSizeX,TopTileSizeY: ����һ����Ƭ�Ĵ�С��ʵ�ʴ�С
    //TilePixelX,TilePixelY: ��Ƭ���ش�С
    //LevelScale: ÿ����ı�����ϵ
    Pyramid(double OX, double OY, double TopTileSizeX, double TopTileSizeY);
    Pyramid(const Pyramid &Obj);

    static const Pyramid *instance(void);

public:
    const Pyramid &operator = (const Pyramid &Obj);

public:
    const math::Point2d &getOrigin(void) const;
    const math::Vector2d &getTopTileSize(void) const;
    const math::Vector2d &getLevelTileSize(unsigned nLevel) const;

    //ͨ����ź����кŵõ���Ƭ�ķ�Χ
    bool getTilePos(unsigned nLevel, unsigned nRow, unsigned nCol, double &xMin, double &yMin, double &xMax, double &yMax) const;

    //ͨ����ź͵�����õ����к�
    bool getTile(unsigned nLevel, double x, double y, unsigned &nRow, unsigned &nCol) const;

    //ͨ����ź����ݷ�Χ�õ����кŷ�Χ
    bool getTile(unsigned nLevel, double xMin, double yMin, double xMax, double yMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nRowMax, unsigned &nColMax) const;

    //ͨ�����ݷ�Χ���ж���һ�㣬��һ����Ƭ�����ð�������
    bool getTile(double xMin, double yMin, double xMax, double yMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol) const;

    //ͨ������к��кţ��õ����׽ڵ�Ĳ���к��к�
    bool getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent) const;

    //ͨ������к��кź�ָ���ĸ��ײ�ţ����ײ�Ľڵ���к��кţ�Ҫ���ײ��С�ǰ���
    bool getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent) const;

    // ��ȡĳһ���������к�
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

    //ͨ����ź����кŵõ���Ƭ�ķ�Χ
    bool getCubePos(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, cmm::math::Point3d &ptMin, cmm::math::Point3d &ptMax) const;

    //ͨ����ź͵�����õ����к�
    bool getCube(unsigned nLevel, const cmm::math::Point3d &point, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const;

    //ͨ����ź����ݷ�Χ�õ����кŷ�Χ
    bool getCube(unsigned nLevel, const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nHeightMin, unsigned &nRowMax, unsigned &nColMax, unsigned &nHeightMax) const;

    //ͨ�����ݷ�Χ���ж���һ�㣬��һ����Ƭ�����ð�������
    bool getCube(const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const;

    //ͨ������к��кţ��õ����׽ڵ�Ĳ���к��к�
    bool getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const;

    //ͨ������к��кź�ָ���ĸ��ײ�ţ����ײ�Ľڵ���к��кţ�Ҫ���ײ��С�ǰ���
    bool getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const;

    // ��ȡĳһ���������к�
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
