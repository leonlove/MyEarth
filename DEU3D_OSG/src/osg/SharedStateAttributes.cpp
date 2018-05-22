#include <osg/SharedStateAttributes>
#include <common/Common.h>

namespace osg {

class Creator
{
public:
    explicit Creator(void)
    {
        SharedStateAttributes::instance();
    }
}__Creator;


SharedStateAttributes *SharedStateAttributes::instance(void)
{
    static ref_ptr<SharedStateAttributes>  pSharedStateAttributes = new SharedStateAttributes;
    return pSharedStateAttributes.get();
}


Material *SharedStateAttributes::getMaterialByColor(const Vec4 &clr)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxColors);

    unsigned nClrIndex = 0u;
    std::map<Vec4, unsigned>::const_iterator itorIndex = m_mapColor2Index.find(clr);
    if(itorIndex == m_mapColor2Index.end())
    {
        cmm::genUniqueValue32(nClrIndex);
        m_mapColor2Index[clr] = nClrIndex;

        if(m_mapColor2Index.size() > 100u)
        {
            unsigned nScrapIndex = m_mapColor2Index.begin()->second;
            m_mapColor2Index.erase(m_mapColor2Index.begin());
            m_mapMaterials.erase(nScrapIndex);
        }
    }
    else
    {
        nClrIndex = itorIndex->second;
    }

    std::map<unsigned, ref_ptr<Material> >::iterator itor = m_mapMaterials.find(nClrIndex);
    if(itor == m_mapMaterials.end())
    {
        ref_ptr<Material> pMaterial = new Material;
        pMaterial->setDiffuse(Material::FRONT_AND_BACK, clr);
        pMaterial->setAmbient(Material::FRONT_AND_BACK, clr * 0.6f);
        pMaterial->setSpecular(Material::FRONT_AND_BACK, clr);
        pMaterial->setShininess(Material::FRONT_AND_BACK, 128.0f);
        //pMaterial->setColorMode(Material::AMBIENT_AND_DIFFUSE);
        if(clr[3] <= 0.95f)
        {
            pMaterial->setTransparency(Material::FRONT_AND_BACK, clr[3]);
        }
        m_mapMaterials[nClrIndex] = pMaterial;
        return pMaterial.get();
    }

    return itor->second.get();
}


Vec4Array *SharedStateAttributes::getColorArrayByColor(const Vec4 &clr)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxColors);
    unsigned nClrIndex = 0u;
    std::map<Vec4, unsigned>::const_iterator itorIndex = m_mapColor2Index.find(clr);
    if(itorIndex == m_mapColor2Index.end())
    {
        cmm::genUniqueValue32(nClrIndex);
        m_mapColor2Index[clr] = nClrIndex;

        if(m_mapColor2Index.size() > 100u)
        {
            unsigned nScrapIndex = m_mapColor2Index.begin()->second;
            m_mapColor2Index.erase(m_mapColor2Index.begin());
            m_mapColorArrays.erase(nScrapIndex);
        }
    }
    else
    {
        nClrIndex = itorIndex->second;
    }

    std::map<unsigned, ref_ptr<Vec4Array> >::iterator itor = m_mapColorArrays.find(nClrIndex);
    if(itor == m_mapColorArrays.end())
    {
        ref_ptr<Vec4Array> pColor = new Vec4Array;
        pColor->push_back(clr);
        m_mapColorArrays[nClrIndex] = pColor;
        return pColor.get();
    }

    return itor->second.get();
}


Point *SharedStateAttributes::getPoint(float fSize)
{
    fSize = clampAbove(fSize, 1.0f);
    const unsigned nSize = unsigned(fSize + 0.5f);

    ref_ptr<Point>    pPoint;
    if(nSize > 10u)
    {
        pPoint = new Point(float(nSize));
    }
    else
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(m_mtxPoints);

        std::map<unsigned, ref_ptr<Point> >::iterator itor = m_mapPoints.find(nSize);
        if(itor == m_mapPoints.end())
        {
            pPoint = new Point(float(nSize));
            m_mapPoints.insert(std::make_pair(nSize, pPoint));
        }
        else
        {
            pPoint = itor->second;
        }
    }

    return pPoint.release();
}


LineWidth *SharedStateAttributes::getLineWidth(float fLineWidth)
{
    fLineWidth = clampAbove(fLineWidth, 1.0f);
    const unsigned nLineWidth = unsigned(fLineWidth + 0.5f);
    ref_ptr<LineWidth> pLineWidth;
    if(nLineWidth > 10u)
    {
        pLineWidth = new LineWidth(float(nLineWidth));
    }
    else
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(m_mtxLineWidths);
        std::map<unsigned, ref_ptr<LineWidth> >::iterator itor = m_mapLineWidths.find(nLineWidth);
        if(itor == m_mapLineWidths.end())
        {
            pLineWidth = new LineWidth(float(nLineWidth));
            m_mapLineWidths.insert(std::make_pair(nLineWidth, pLineWidth));
        }
        else
        {
            pLineWidth = itor->second;
        }
    }

    return pLineWidth.release();
}

}
