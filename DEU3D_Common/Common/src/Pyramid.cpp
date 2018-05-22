#include <Pyramid.h>
#include <memory.h>
#include <math.h>

namespace cmm
{

class UniqueCreator
{
public:
    UniqueCreator(void)
    {
        Pyramid::instance();
        Pyramid3::instance();
    }
}__xxCreator;


const Pyramid *Pyramid::instance(void)
{
    static Pyramid s_Pyramid;
    return &s_Pyramid;
}


Pyramid::Pyramid(void)
{
    m_ptOrigin.set(-cmm::math::PI, -cmm::math::PI_2);
    m_vecTopTileSize.set(cmm::math::PI + cmm::math::PI, cmm::math::PI + cmm::math::PI);
    init();
}


Pyramid::Pyramid(double OX, double OY, double TopTileSizeX, double TopTileSizeY)
{
    m_ptOrigin.set(OX, OY);
    m_vecTopTileSize.set(TopTileSizeX, TopTileSizeY);
    init();
}


Pyramid::Pyramid(const Pyramid &Obj)
{
    *this = Obj;
}

const Pyramid &Pyramid::operator = (const Pyramid &Obj)
{
    if(this == &Obj)    return *this;
    *this = Obj;
    return *this;
}

const math::Point2d &Pyramid::getOrigin(void) const
{
    return m_ptOrigin;
}


const math::Vector2d &Pyramid::getTopTileSize(void) const
{
    return m_vecTopTileSize;
}


const math::Vector2d &Pyramid::getLevelTileSize(unsigned nLevel) const
{
    if (nLevel >= MAXLEVEL)
    {
        static const math::Vector2d vec(0.0, 0.0);
        return vec;
    }
    return m_LevelTileSizes[nLevel];
}


unsigned Pyramid::getMaxCol(unsigned nLevel) const
{
    return 1u << nLevel;
}


unsigned Pyramid::getMaxRow(unsigned nLevel) const
{
    return 1u << nLevel;
}


bool Pyramid::getTilePos(unsigned nLevel, unsigned nRow, unsigned nCol, double &xMin, double &yMin, double &xMax, double &yMax) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    xMin = m_ptOrigin.x() + (nCol * m_LevelTileSizes[nLevel].x());
    yMin = m_ptOrigin.y() + (nRow * m_LevelTileSizes[nLevel].y());
    xMax = xMin + m_LevelTileSizes[nLevel].x();
    yMax = yMin + m_LevelTileSizes[nLevel].y();
    if (xMin > xMax)
    {
        double tmp = xMin;
        xMin = xMax;
        xMax = tmp;
    }
    if (yMin > yMax)
    {
        double tmp = yMin;
        yMin = yMax;
        yMax = tmp;
    }
    return true;
}


bool Pyramid::getTile(unsigned nLevel, double x, double y, unsigned &nRow, unsigned &nCol) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    nCol = (unsigned)floor((x - m_ptOrigin.x()) / m_LevelTileSizes[nLevel].x());
    nRow = (unsigned)floor((y - m_ptOrigin.y()) / m_LevelTileSizes[nLevel].y());
    return true;
}


bool Pyramid::getTile(unsigned nLevel, double xMin, double yMin, double xMax, double yMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nRowMax, unsigned &nColMax) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    getTile(nLevel, xMin, yMin, nRowMin, nColMin);
    getTile(nLevel, xMax, yMax, nRowMax, nColMax);
    if (nRowMin > nRowMax)
    {
        const unsigned tmp = nRowMin;
        nRowMin = nRowMax;
        nRowMax = tmp;
    }
    if (nColMin > nColMax)
    {
        const unsigned tmp = nColMin;
        nColMin = nColMax;
        nColMax = tmp;
    }
    return true;
}


bool Pyramid::getTile(double xMin, double yMin, double xMax, double yMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    unsigned tmplevel = 0u;
    unsigned nRowMin, nRowMax, nColMin, nColMax;
    for (unsigned i = 0u; i < MAXLEVEL; i++)
    {
        getTile(i, xMin, yMin, xMax, yMax, nRowMin, nColMin, nRowMax, nColMax);
        if ((nRowMin == nRowMax) && (nColMin == nColMax))
        {
            nRow = nRowMin;
            nCol = nColMin;
            nLevel = i;
        }
        else break;
    }

    return true;
}

bool Pyramid::getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent) const
{
    if(nLevel == 0u)
    {
        nLevelParent = ~0u;
        nRowParent = ~0u;
        nColParent = ~0u;
        return false;
    }

    nLevelParent = --nLevel;
    nRowParent = nRow >> 1u;
    nColParent = nCol >> 1u;
    return true;
}


//通过层号行号列号，得到父亲节点的层号行号列号
bool Pyramid::getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent) const
{
    if(nLevelParent >= nLevel || nLevel == 0u)
    {
        nRowParent = ~0u;
        nColParent = ~0u;
        return false;
    }

    const unsigned nLevelDetal = nLevel - nLevelParent;
    nRowParent = nRow >> nLevelDetal;
    nColParent = nCol >> nLevelDetal;
    return true;
}


void Pyramid::init()
{
    m_LevelTileSizes[0u] = m_vecTopTileSize;
    for(unsigned i = 1u; i < MAXLEVEL; i++)
    {
        m_LevelTileSizes[i] = m_LevelTileSizes[i - 1u] * 0.5;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
Pyramid3::Pyramid3(void)
{
    const double dblEquatorRadius = 6378137.0;
    const double dblCircumference = dblEquatorRadius * cmm::math::PI;

    m_ptOrigin.set(-cmm::math::PI, -cmm::math::PI_2, -dblCircumference);

    const double dblPI2 = cmm::math::PI + cmm::math::PI;
    m_vecTopCubeSize.set(dblPI2, dblPI2, dblCircumference + dblCircumference);

    init();
}


Pyramid3::Pyramid3(const math::Point3d &ptOrigin, const math::Vector3d &vecTopCubeSize)
{
    m_ptOrigin = ptOrigin;
    m_vecTopCubeSize = vecTopCubeSize;
    init();
}


Pyramid3::Pyramid3(const Pyramid3 &Obj)
{
    *this = Obj;
}


const Pyramid3 &Pyramid3::operator = (const Pyramid3 &Obj)
{
    if(this == &Obj)    return *this;
    *this = Obj;
    return *this;
}


const Pyramid3 *Pyramid3::instance(void)
{
    static Pyramid3 s_Pyramid3;
    return &s_Pyramid3;
}


void Pyramid3::init(void)
{
    m_LevelCubeSizes[0u] = m_vecTopCubeSize;
    for(unsigned i = 1u; i < MAXLEVEL; i++)
    {
        m_LevelCubeSizes[i] = m_LevelCubeSizes[i - 1u] * 0.5;
    }
}


const math::Point3d &Pyramid3::getOrigin(void) const
{
    return m_ptOrigin;
}


const math::Vector3d &Pyramid3::getTopCubeSize(void) const
{
    return m_vecTopCubeSize;
}


const math::Vector3d &Pyramid3::getLevelCubeSize(unsigned level) const
{
    if(level >= MAXLEVEL)
    {
        static const math::Vector3d vec(0.0, 0.0, 0.0);
        return vec;
    }
    return m_LevelCubeSizes[level];
}


//通过层号和行列号得到瓦片的范围
bool Pyramid3::getCubePos(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, cmm::math::Point3d &ptMin, cmm::math::Point3d &ptMax) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    const cmm::math::Vector3d &vecTileSize = m_LevelCubeSizes[nLevel];
    ptMin.x() = m_ptOrigin.x() + (nCol * vecTileSize.x());
    ptMin.y() = m_ptOrigin.y() + (nRow * vecTileSize.y());
    ptMin.z() = m_ptOrigin.z() + (nHeight * vecTileSize.z());
    ptMax.x() = ptMin.x() + vecTileSize.x();
    ptMax.y() = ptMin.y() + vecTileSize.y();
    ptMax.z() = ptMin.z() + vecTileSize.z();

    if(ptMin.x() > ptMax.x())
    {
        double tmp = ptMin.x();
        ptMin.x() = ptMax.x();
        ptMax.x() = tmp;
    }
    if(ptMin.y() > ptMax.y())
    {
        double tmp = ptMin.y();
        ptMin.y() = ptMax.y();
        ptMax.y() = tmp;
    }
    if(ptMin.z() > ptMax.z())
    {
        double tmp = ptMin.z();
        ptMin.z() = ptMax.z();
        ptMax.z() = tmp;
    }

    return true;
}


//通过层号和点坐标得到行列号
bool Pyramid3::getCube(unsigned nLevel, const cmm::math::Point3d &point, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    const cmm::math::Vector3d &vecCubeSize = m_LevelCubeSizes[nLevel];
    nCol = (unsigned)floor((point.x() - m_ptOrigin.x()) / vecCubeSize.x());
    nRow = (unsigned)floor((point.y() - m_ptOrigin.y()) / vecCubeSize.y());
    nHeight = (unsigned)(floor((point.z() - m_ptOrigin.z()) / vecCubeSize.z()));

    return true;
}


//通过层号和数据范围得到行列号范围
bool Pyramid3::getCube(unsigned nLevel, const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nRowMin, unsigned &nColMin, unsigned &nHeightMin, unsigned &nRowMax, unsigned &nColMax, unsigned &nHeightMax) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    getCube(nLevel, ptMin, nRowMin, nColMin, nHeightMin);
    getCube(nLevel, ptMax, nRowMax, nColMax, nHeightMax);
    if (nRowMin > nRowMax)
    {
        const unsigned tmp = nRowMin;
        nRowMin = nRowMax;
        nRowMax = tmp;
    }
    if (nColMin > nColMax)
    {
        const unsigned tmp = nColMin;
        nColMin = nColMax;
        nColMax = tmp;
    }
    if(nHeightMin > nHeightMax)
    {
        const int tmp = nHeightMin;
        nHeightMin = nHeightMax;
        nHeightMax = tmp;
    }
    return true;
}


//通过数据范围，判断哪一层，哪一个瓦片，正好包含数据
bool Pyramid3::getCube(const cmm::math::Point3d &ptMin, const cmm::math::Point3d &ptMax, unsigned &nLevel, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const
{
    if (nLevel >= MAXLEVEL)
    {
        return false;
    }

    unsigned tmplevel = 0u;
    unsigned nRowMin, nRowMax, nColMin, nColMax, nHeightMin, nHeightMax;
    for (unsigned i = 0u; i < MAXLEVEL; i++)
    {
        getCube(i, ptMin, ptMax, nRowMin, nColMin, nHeightMin, nRowMax, nColMax, nHeightMax);
        if ((nRowMin == nRowMax) && (nColMin == nColMax) && (nHeightMin == nHeightMax))
        {
            nRow = nRowMin;
            nCol = nColMin;
            nHeight = nHeightMin;
            nLevel = i;
        }
        else break;
    }
    return true;
}


//通过层号行号列号，得到父亲节点的层号行号列号
bool Pyramid3::getParent(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned &nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const
{
    if(nLevel == 0u)
    {
        nLevelParent = ~0u;
        nRowParent = ~0u;
        nColParent = ~0u;
        nHeightParent = ~0u;
        return false;
    }

    nLevelParent = --nLevel;
    nRowParent = nRow >> 1u;
    nColParent = nCol >> 1u;
    nHeightParent = nHeight / 2;
    return true;
}


//通过层号行号列号和指定的父亲层号，父亲层的节点的行号列号，要求父亲层号小宇当前层号
bool Pyramid3::getParentByLevel(unsigned nLevel, unsigned nRow, unsigned nCol, unsigned nHeight, unsigned nLevelParent, unsigned &nRowParent, unsigned &nColParent, unsigned &nHeightParent) const
{
    if(nLevelParent >= nLevel || nLevel == 0u)
    {
        nRowParent = ~0u;
        nColParent = ~0u;
        nHeightParent = ~0u;
        return false;
    }

    const unsigned nLevelDetal = nLevel - nLevelParent;
    nRowParent = nRow >> nLevelDetal;
    nColParent = nCol >> nLevelDetal;
    nHeightParent = nHeight >> nLevelDetal;
    return true;
}


unsigned Pyramid3::getMaxCol(unsigned nLevel) const
{
    return 1u << nLevel;
}


unsigned Pyramid3::getMaxRow(unsigned nLevel) const
{
    return 1u << nLevel;
}


unsigned Pyramid3::getMaxHeight(unsigned nLevel) const
{
    return 1u << nLevel;
}



}

