#include <osgUtil/CommonModelCreater.h>

#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Quat>
#include <osg/MatrixTransform>

#include <osgUtil/Tessellator>
#include <osg/Point>
#include <osg/LineWidth>
#include <xutility>
#include <Common\deuMath.h>
#include <osgUtil/ShareModelVisitor>

namespace osgUtil
{

class Creator
{
public:
    Creator(void)
    {
        CommonModelCreater::instance();
    }
}__creator;

CommonModelCreater::CommonModelCreater()
{
    osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);

    m_pNormalState = new osg::StateSet;
    m_pNormalState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
    m_pNormalState->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    m_pNoNormalState = new osg::StateSet;
    m_pNoNormalState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);

    m_pNormalAndAlphaState = new osg::StateSet;
    m_pNormalAndAlphaState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
    m_pNormalState->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
    m_pNormalAndAlphaState->setMode(GL_BLEND, osg::StateAttribute::ON);
    m_pNormalAndAlphaState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    m_pNoNormalAndAlphaState = new osg::StateSet;
    m_pNoNormalAndAlphaState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
    m_pNoNormalAndAlphaState->setMode(GL_BLEND, osg::StateAttribute::ON);
    m_pNoNormalAndAlphaState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

CommonModelCreater::~CommonModelCreater()
{
}

CommonModelCreater *CommonModelCreater::instance(void)
{
    static osg::ref_ptr<CommonModelCreater>  spCommonModelCreater = new CommonModelCreater();
    return spCommonModelCreater.get();
}

void CommonModelCreater::getColorArray(const osg::Vec4 &clr, osg::ref_ptr<osg::Vec4Array> &pColorArray)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxColorArray);

    if(m_mapColorArray.size() > 1000u)
    {
        std::map<osg::Vec4, osg::observer_ptr<osg::Vec4Array> >::iterator itor = m_mapColorArray.begin();
        while(itor != m_mapColorArray.end())
        {
            if(itor->second.valid())
            {
                ++itor;
            }
            else
            {
                itor = m_mapColorArray.erase(itor);
            }
        }
    }

    std::map<osg::Vec4, osg::observer_ptr<osg::Vec4Array> >::iterator itorFind = m_mapColorArray.find(clr);
    if(itorFind != m_mapColorArray.end())
    {
        itorFind->second.lock(pColorArray);
    }


    if(pColorArray.valid())
    {
        return;
    }

    pColorArray = new osg::Vec4Array;
    pColorArray->push_back(clr);

    m_mapColorArray[clr] = pColorArray.get();
}


void CommonModelCreater::createStandardModel(ModleType eModleType, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor, osg::ref_ptr<osg::Node> &pReturnNode)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxStandardModel);

    if(m_mapStandardModel.size() > 1000)
    {
        std::map<StandardModelTag, osg::observer_ptr<osg::Node> >::iterator itor = m_mapStandardModel.begin();
        while(itor != m_mapStandardModel.end())
        {
            if(itor->second.valid())
            {
                ++itor;
            }
            else
            {
                itor = m_mapStandardModel.erase(itor);
            }
        }
    }


    const unsigned int nTag = ((unsigned int)eModleType << 24u) + ((unsigned int)eCoverType << 16u) + ((unsigned int)bTexStretched << 8u);
    const StandardModelTag stag(vColor, nTag);

    std::map<StandardModelTag, osg::observer_ptr<osg::Node> >::iterator itorFind = m_mapStandardModel.find(stag);
    if(itorFind != m_mapStandardModel.end())
    {
        itorFind->second.lock(pReturnNode);
    }

    if(pReturnNode.valid())
    {
        return;
    }


    switch(eModleType)
    {
    case CYLINDER:
        {
            pReturnNode = createCylinder(eCoverType, bTexStretched, vColor);
            break;
        }
    case CUBE:
        {
            pReturnNode = createCube(eCoverType, bTexStretched, vColor);
            break;
        }
    case CONE:
        {
            pReturnNode = createCone(eCoverType, bTexStretched, vColor);
            break;
        }
    case SPHERE:
        {
            pReturnNode = createSphere(bTexStretched, vColor);
            break;
        }
    default:
        {
            break;
        }
    }

    if(pReturnNode.valid())
    {
        //SetDrawableAsShared visitor(true);
        //pReturnNode->accept(visitor);

        if(vColor[3] < 0.98f)
        {
            pReturnNode->setStateSet(m_pNormalAndAlphaState.get());
        }
        else
        {
            pReturnNode->setStateSet(m_pNormalState.get());
        }
        m_mapStandardModel[stag] = pReturnNode.get();
    }
}

//创建标准圆柱
osg::Node *CommonModelCreater::createCylinder(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor)
{
    const float fRadius = 1.0f;
    const unsigned int nSegments = 40u;
    const osg::Vec3 vCenter(0.0f, 0.0f, 0.0f);
    osg::ref_ptr<osg::Vec3Array> pVertexArray = createCycleVertex(vCenter, fRadius, nSegments);

    return createPrism(pVertexArray.get(), 1.0f, eCoverType, bTexStretched, vColor);
}

osg::Node *CommonModelCreater::createCylinderWithHole(float fRadius, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor)
{
    const unsigned int nSegments = 40u;
    const osg::Vec3 vCenter(0.0f, 0.0f, 0.0f);
    osg::ref_ptr<osg::Vec3Array> pVertexArray = createCycleVertex(vCenter, fRadius, nSegments);

    return createPrismWithHole(pVertexArray.get(), fHeight, eCoverType, vecTopHoleArray, vecBottomHoleArray, bTexStretched, vColor);
}

//创建标准长方体
osg::Node *CommonModelCreater::createCube(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;

    pVertexArray->push_back(osg::Vec3( 0.5f, -0.5f, 0.0f));
    pVertexArray->push_back(osg::Vec3(-0.5f, -0.5f, 0.0f));
    pVertexArray->push_back(osg::Vec3(-0.5f,  0.5f, 0.0f));
    pVertexArray->push_back(osg::Vec3( 0.5f,  0.5f, 0.0f));
    pVertexArray->push_back(osg::Vec3( 0.5f, -0.5f, 0.0f));

    return createPrism(pVertexArray.get(), 1.0f, eCoverType, bTexStretched, vColor);
}

osg::Node *CommonModelCreater::createCubeWithHole(float fLength, float fWidth, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;

    pVertexArray->push_back(osg::Vec3( 0.5f * fLength, -0.5f * fWidth, 0.0f));
    pVertexArray->push_back(osg::Vec3(-0.5f * fLength, -0.5f * fWidth, 0.0f));
    pVertexArray->push_back(osg::Vec3(-0.5f * fLength,  0.5f * fWidth, 0.0f));
    pVertexArray->push_back(osg::Vec3( 0.5f * fLength,  0.5f * fWidth, 0.0f));
    pVertexArray->push_back(osg::Vec3( 0.5f * fLength, -0.5f * fWidth, 0.0f));

    return createPrismWithHole(pVertexArray.get(), fHeight, eCoverType, vecTopHoleArray, vecBottomHoleArray, bTexStretched, vColor);
}

//创建标准圆锥
osg::Node *CommonModelCreater::createCone(CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const float fRadius = 1.0f;
    const float fHeight = 1.0f;

    const unsigned int nSegments = 40u;
    const unsigned int nRow = 5u;

    //创建椎体
    {
        const float fDeltaHeight = fHeight / (float)nRow;
        const float fDeltaRadius = fRadius / (float)nRow;

        const osg::Vec3 vCenter(0.0f, 0.0f, -fRadius * 0.5f);
        const float fTopAngle = atanf(fRadius / fHeight);

        float fTexDeltaX;
        float fTexDeltaY;

        if(bTexStretched)
        {
            fTexDeltaX = 1.0f / (float)nSegments;
            fTexDeltaY = 1.0f / (float)nRow;
        }
        else
        {
            fTexDeltaX = 2 * osg::PI * fRadius / (float)nSegments;
            fTexDeltaY = fHeight / (nRow * cosf(fTopAngle));;
        }

        osg::ref_ptr<osg::Vec3Array> pCycleNormal = createCycleNormal(fTopAngle, nSegments);

        for(unsigned int i = 0; i < nRow; i++)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

            osg::Vec3 vTempCenter1(vCenter[0], vCenter[1], vCenter[2] + i * fDeltaHeight);
            float fTempRadius1 = fRadius - i * fDeltaRadius;
            osg::Vec3 vTempCenter2(vCenter[0], vCenter[1], vCenter[2] + (i + 1) * fDeltaHeight);
            float fTempRadius2 = fRadius - (i + 1) * fDeltaRadius;

            //顶点
            osg::ref_ptr<osg::Vec3Array> pCycleVertex1 = createCycleVertex(vTempCenter1, fTempRadius1, nSegments);
            osg::ref_ptr<osg::Vec3Array> pCycleVertex2 = createCycleVertex(vTempCenter2, fTempRadius2, nSegments);

            for(unsigned int j = 0; j <= nSegments; j++)
            {
                pVertexArray->push_back((*pCycleVertex1)[j]);
                pVertexArray->push_back((*pCycleVertex2)[j]);

                pNormalArray->push_back((*pCycleNormal)[j]);
                pNormalArray->push_back((*pCycleNormal)[j]);

                pTexCoordArray->push_back(osg::Vec2(i * fTexDeltaY, j * fTexDeltaX));
                pTexCoordArray->push_back(osg::Vec2((i + 1) * fTexDeltaY, j * fTexDeltaX));
            }

            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, pVertexArray->size()));

            pGeode->addDrawable(pGeometry.get()); 
        }
    }

    //生成底面
    if(eCoverType == ALL || eCoverType == BOTTOM)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

        //顶点索引
        pVertexArray->push_back(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5f));
        osg::ref_ptr<osg::Vec3Array> pCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5f), fRadius, nSegments);
        pVertexArray->insert(pVertexArray->end(), pCycleVertex->begin(), pCycleVertex->end());

        //法向量
        pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));

        //纹理坐标f
        if(bTexStretched)
        {
            pTexCoordArray->push_back(osg::Vec2(0.5f, 0.5f));

            const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
            float fAngle = 0;
            for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
            {
                pTexCoordArray->push_back(osg::Vec2(0.5f * cosf(fAngle) + 0.5f, 0.5f * sinf(fAngle) + 0.5f));
            }
            pTexCoordArray->push_back(osg::Vec2(1.0f, 0.5f));
        }
        else
        {
            pTexCoordArray->push_back(osg::Vec2(fRadius, fRadius));

            const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
            float fAngle = 0;
            for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
            {
                pTexCoordArray->push_back(osg::Vec2(fRadius * cosf(fAngle) + fRadius, fRadius * sinf(fAngle) + fRadius));
            }
            pTexCoordArray->push_back(osg::Vec2(2.0f * fRadius, fRadius));
        }

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertexArray->size()));
        pGeode->addDrawable(pGeometry.get());
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNormalAndAlphaState);
    }
    else
    {
        pGeode->setStateSet(m_pNormalState);
    }

    return pGeode.release();
}

//创建标准球
osg::Node *CommonModelCreater::createSphere(bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const unsigned int nRows = 20u;
    const unsigned int nSegments = 40u;

    const float fRadius = 1.0f;

    const float fDeltaVAngle  = osg::PI / (float)nRows;

    float fTexDeltaX;
    float fTexDeltaY;

    if(bTexStretched)
    {
        fTexDeltaX = 1.0f / (float)nSegments;
        fTexDeltaY = 1.0f / (float)nRows;
    }
    else
    {
        fTexDeltaX = 2.0f * osg::PI * fRadius / (float)nSegments;
        fTexDeltaY = osg::PI * fRadius / (float)nRows;
    }

    for(unsigned int i = 0u; i < nRows; i++)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormorArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

        const float fRowRadius1 = fRadius * fabsf(sinf(i * fDeltaVAngle));
        const float fRowHeight1 = -fRadius * cosf(i * fDeltaVAngle);

        const float fRowRadius2 = fRadius * fabsf(sinf((i + 1u) * fDeltaVAngle));
        const float fRowHeight2 = -fRadius * cosf((i + 1u) * fDeltaVAngle);

        const float fDeltaHAngle = osg::PI * 2.0f / (float)nSegments;

        for(unsigned int j = 0u; j <= nSegments; j++)
        {
            float fTempAngle = j * fDeltaHAngle;
            //顶点
            osg::Vec3 v1(cosf(fTempAngle) * fRowRadius1, sinf(fTempAngle) * fRowRadius1, fRowHeight1);
            osg::Vec3 v2(cosf(fTempAngle) * fRowRadius2, sinf(fTempAngle) * fRowRadius2, fRowHeight2);
            pVertexArray->push_back(v1);
            pVertexArray->push_back(v2);

            //法向量
            v1.normalize();
            pNormorArray->push_back(v1);
            v2.normalize();
            pNormorArray->push_back(v2);

            //纹理坐标
            pTexCoordArray->push_back(osg::Vec2(i * fTexDeltaY, j * fTexDeltaX));
            pTexCoordArray->push_back(osg::Vec2((i + 1u) * fTexDeltaY, j * fTexDeltaX));
        }

        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, pVertexArray->size()));
        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->setNormalArray(pNormorArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeode->addDrawable(pGeometry.get());
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNormalAndAlphaState);
    }
    else
    {
        pGeode->setStateSet(m_pNormalState);
    }

    return pGeode.release();
}

//创建棱柱
osg::Node *CommonModelCreater::createPrism(const osg::Vec3Array *pVertex, float fHeight, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const float fHalfHeight = fHeight * 0.5f;

    std::vector<osg::Vec3> vecVertex;

    //创建侧面
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

        unsigned int nSize = pVertex->size();
        for(unsigned int i = 0u; i < nSize; i++)
        {
            vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], -fHalfHeight));
        }
        for(unsigned int i = 0u; i < nSize; i++)
        {
            vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], fHalfHeight));
        }

        for(unsigned int i = 0u; i < nSize - 1u; i++)
        {
            osg::Vec3 v1 = vecVertex[i + 1u] - vecVertex[i];
            osg::Vec3 v2 = vecVertex[i + nSize] - vecVertex[i];
            osg::Vec3 vN = v2 ^ v1;
            vN.normalize();
            pNormalArray->push_back(vN);
        }

        nSize--;
        float fTex = 0.0f;
        for(unsigned int i = 0; i < nSize; i++)
        {
            osg::Vec3 vTemp = vecVertex[i + 1] - vecVertex[i];
            const float fTemp = vTemp.length();

            pVertexArray->push_back(vecVertex[i]);
            pTexCoordArray->push_back(osg::Vec2(fHeight, fTex));

            pVertexArray->push_back(vecVertex[i + 1]);
            pTexCoordArray->push_back(osg::Vec2(fHeight, fTex + fTemp));

            pVertexArray->push_back(vecVertex[nSize + 2 + i]);
            pTexCoordArray->push_back(osg::Vec2(0.0f, fTex + fTemp));

            pVertexArray->push_back(vecVertex[nSize + 1 + i]);
            pTexCoordArray->push_back(osg::Vec2(0.0f, fTex));

            fTex += fTemp;
        }

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, pVertexArray->size()));

        pGeode->addDrawable(pGeometry.get());
    }

    bool bHasTop = true;
    bool bHasBottom = true;

    switch(eCoverType)
    {
    case NONE:
        {
            bHasTop = false;
            bHasBottom = false;
            break;
        }
    case TOP:
        {
            bHasBottom = false;
            break;
        }
    case BOTTOM:
        {
            bHasTop = false;
            break;
        }
    }

    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = createTexCoordByVertex(pVertex, bTexStretched);

    osg::ref_ptr<osgUtil::Tessellator>  pTessellator = new osgUtil::Tessellator;

    if(bHasBottom)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

        pVertexArray->insert(pVertexArray->end(), vecVertex.begin(), vecVertex.begin() + pVertex->size());

        pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        if(!pTexCoordArray->empty())
        {
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        }
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));
        pTessellator->retessellatePolygons(*pGeometry);
        pGeode->addDrawable(pGeometry.get());
    }

    if(bHasTop)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

        pVertexArray->insert(pVertexArray->end(), vecVertex.begin() + pVertex->size(), vecVertex.end());
        pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        if(!pTexCoordArray->empty())
        {
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        }
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));
        pTessellator->retessellatePolygons(*pGeometry);
        pGeode->addDrawable(pGeometry.get());
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNoNormalAndAlphaState.get());
    }
    else
    {
        pGeode->setStateSet(m_pNoNormalState.get());
    }

    return pGeode.release();
}

osg::Node *CommonModelCreater::createPrismWithHole(const osg::Vec3Array *pVertex, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const float fHalfHeight = fHeight * 0.5f;

    std::vector<osg::Vec3> vecVertex;

    //创建侧面
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

        unsigned int nSize = pVertex->size();
        for(unsigned int i = 0u; i < nSize; i++)
        {
            vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], -fHalfHeight));
        }
        for(unsigned int i = 0u; i < nSize; i++)
        {
            vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], fHalfHeight));
        }

        for(unsigned int i = 0u; i < nSize - 1u; i++)
        {
            osg::Vec3 v1 = vecVertex[i + 1u] - vecVertex[i];
            osg::Vec3 v2 = vecVertex[i + nSize] - vecVertex[i];
            osg::Vec3 vN = v2 ^ v1;
            vN.normalize();
            pNormalArray->push_back(vN);
        }

        nSize--;
        float fTex = 0.0f;
        for(unsigned int i = 0u; i < nSize; i++)
        {
            osg::Vec3 vTemp = vecVertex[i + 1u] - vecVertex[i];
            const float fTemp = vTemp.length();

            pVertexArray->push_back(vecVertex[i]);
            pTexCoordArray->push_back(osg::Vec2(0.0f, fTex));

            pVertexArray->push_back(vecVertex[i + 1]);
            pTexCoordArray->push_back(osg::Vec2(0.0f, fTex + fTemp));

            pVertexArray->push_back(vecVertex[nSize + 2 + i]);
            pTexCoordArray->push_back(osg::Vec2(fHeight, fTex + fTemp));

            pVertexArray->push_back(vecVertex[nSize + 1 + i]);
            pTexCoordArray->push_back(osg::Vec2(fHeight, fTex));

            fTex += fTemp;
        }

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, pVertexArray->size()));

        pGeode->addDrawable(pGeometry.get());
    }

    bool bHasTop = true;
    bool bHasBottom = true;

    switch(eCoverType)
    {
    case NONE:
        {
            bHasTop = false;
            bHasBottom = false;
            break;
        }
    case TOP:
        {
            bHasBottom = false;
            break;
        }
    case BOTTOM:
        {
            bHasTop = false;
            break;
        }
    }

    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = createTexCoordByVertex(pVertex, bTexStretched);

    osg::ref_ptr<osgUtil::Tessellator>  pTessellator = new osgUtil::Tessellator;

    if(bHasTop)
    {
        //上底有洞
        if(!vecTopHoleArray.empty())
        {
            std::vector<osg::Vec3> vecVertex;
            const unsigned int nSize = pVertex->size();
            for(unsigned int i = 0u; i < nSize; i++)
            {
                vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], fHalfHeight));
            }

            osg::ref_ptr<osg::Geometry> pTopGeo = createSheetWithHoles(vecVertex, vecTopHoleArray, bTexStretched, vColor);
            pGeode->addDrawable(pTopGeo.get());
        }
        else
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

            pVertexArray->insert(pVertexArray->end(), vecVertex.begin() + pVertex->size(), vecVertex.end());
            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            if(!pTexCoordArray->empty())
            {
                pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            }
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));
            pTessellator->retessellatePolygons(*pGeometry);
            pGeode->addDrawable(pGeometry.get());
        }
    }

    if(bHasBottom)
    {
        //下底有洞
        if(!vecBottomHoleArray.empty())
        {
            std::vector<osg::Vec3> vecVertex;
            for(unsigned int i = 0u; i < pVertex->size(); i++)
            {
                vecVertex.push_back(osg::Vec3((*pVertex)[i][0], (*pVertex)[i][1], -fHalfHeight));
            }

            osg::ref_ptr<osg::Geometry> pBottomGeo = createSheetWithHoles(vecVertex, vecTopHoleArray, bTexStretched, vColor);
            pGeode->addDrawable(pBottomGeo.get());
        }
        else
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

            pVertexArray->insert(pVertexArray->end(), vecVertex.begin(), vecVertex.begin() + pVertex->size());

            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));

            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            if(!pTexCoordArray->empty())
            {
                pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            }
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));
            pTessellator->retessellatePolygons(*pGeometry);
            pGeode->addDrawable(pGeometry.get());

        }
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNoNormalAndAlphaState.get());
    }
    else
    {
        pGeode->setStateSet(m_pNoNormalState.get());
    }

    return pGeode.release();
}

//创建圆台
osg::Node *CommonModelCreater::createRoundTable(float fTopRadius, float fBottomRadius, float fHeight, CoverType eCoverType, bool bTexStretched, const osg::Vec4 &vColor) 
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const unsigned int nSegments = 40u;
    const unsigned int nRow = 3u;

    //创建侧面
    {
        const float fDeltaHeight = fHeight / (float)nRow;
        const float fDeltaRadius = (fBottomRadius - fTopRadius) / (float)nRow;

        const osg::Vec3 vCenter(0.0f, 0.0f, -fHeight * 0.5f);
        const float fTopAngle = atan2f((fBottomRadius - fTopRadius), fHeight);

        float fTexDeltaX;
        float fTexDeltaY;

        if(bTexStretched)
        {
            fTexDeltaX = 1.0f / (float)nSegments;
            fTexDeltaY = 1.0f / (float)nRow;
        }
        else
        {
            fTexDeltaX = 2 * osg::PI * std::max(fTopRadius, fBottomRadius) / (float)nSegments;
            fTexDeltaY = fHeight / (nRow * cosf(fTopAngle));
        }

        osg::ref_ptr<osg::Vec3Array> pCycleNormal = createCycleNormal(fTopAngle, nSegments);

        for(unsigned int i = 0u; i < nRow; i++)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

            osg::Vec3 vTempCenter1(vCenter[0], vCenter[1], vCenter[2] + i * fDeltaHeight);
            float fTempRadius1 = fBottomRadius - i * fDeltaRadius;
            osg::Vec3 vTempCenter2(vCenter[0], vCenter[1], vCenter[2] + (i + 1) * fDeltaHeight);
            float fTempRadius2 = fBottomRadius - (i + 1) * fDeltaRadius;

            //顶点
            osg::ref_ptr<osg::Vec3Array> pCycleVertex1 = createCycleVertex(vTempCenter1, fTempRadius1, nSegments);
            osg::ref_ptr<osg::Vec3Array> pCycleVertex2 = createCycleVertex(vTempCenter2, fTempRadius2, nSegments);
            for(unsigned int j = 0u; j <= nSegments; j++)
            {
                pVertexArray->push_back((*pCycleVertex1)[j]);
                pVertexArray->push_back((*pCycleVertex2)[j]);

                pNormalArray->push_back((*pCycleNormal)[j]);
                pNormalArray->push_back((*pCycleNormal)[j]);

                pTexCoordArray->push_back(osg::Vec2(i * fTexDeltaY, j * fTexDeltaX));
                pTexCoordArray->push_back(osg::Vec2((i + 1) * fTexDeltaY, j * fTexDeltaX));
            }

            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, pVertexArray->size()));
            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeode->addDrawable(pGeometry.get()); 
        }
    }

    bool bHasTop = true;
    bool bHasBottom = true;

    switch(eCoverType)
    {
    case NONE:
        {
            bHasTop = false;
            bHasBottom = false;
            break;
        }
    case TOP:
        {
            bHasBottom = false;
            break;
        }
    case BOTTOM:
        {
            bHasTop = false;
            break;
        }
    }

    //上底下底的纹理坐标
    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;
    if(bTexStretched)
    {
        pTexCoordArray->push_back(osg::Vec2(0.5f, 0.5f));

        float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
        float fAngle = 0.0f;
        for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
        {
            pTexCoordArray->push_back(osg::Vec2(0.5f * cosf(fAngle) + 0.5f, 0.5f * sinf(fAngle) + 0.5f));
        }
        pTexCoordArray->push_back(osg::Vec2(1.0f, 0.5f));
    }

    //生成底面
    if(bHasBottom)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

        //顶点索引
        pVertexArray->push_back(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5));
        osg::ref_ptr<osg::Vec3Array> pCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5), fBottomRadius, nSegments);
        pVertexArray->insert(pVertexArray->end(), pCycleVertex->begin(), pCycleVertex->end());

        //法向量
        pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));

        //纹理坐标
        if(!bTexStretched)
        {
            pTexCoordArray = new osg::Vec2Array;
            pTexCoordArray->push_back(osg::Vec2(fBottomRadius, fBottomRadius));

            const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
            float fAngle = 0.0f;
            for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
            {
                pTexCoordArray->push_back(osg::Vec2(fBottomRadius * cosf(fAngle) + fBottomRadius, fBottomRadius * sinf(fAngle) + fBottomRadius));
            }
            pTexCoordArray->push_back(osg::Vec2(2.0f * fBottomRadius, fBottomRadius));
        }

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertexArray->size()));

        pGeode->addDrawable(pGeometry.get());
    }

    //生成顶面
    if(bHasTop)
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

        //顶点索引
        pVertexArray->push_back(osg::Vec3(0.0f, 0.0f, fHeight * 0.5));
        osg::ref_ptr<osg::Vec3Array> pCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, fHeight * 0.5), fTopRadius, nSegments);
        pVertexArray->insert(pVertexArray->end(), pCycleVertex->begin(), pCycleVertex->end());

        //法向量
        pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

        //纹理坐标
        if(!bTexStretched)
        {
            pTexCoordArray = new osg::Vec2Array;
            pTexCoordArray->push_back(osg::Vec2(fTopRadius, fTopRadius));

            const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
            float fAngle = 0.0f;
            for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
            {
                pTexCoordArray->push_back(osg::Vec2(fTopRadius * cosf(fAngle) + fTopRadius, fTopRadius * sinf(fAngle) + fTopRadius));
            }
            pTexCoordArray->push_back(osg::Vec2(2.0f * fTopRadius, fTopRadius));
        }

        pGeometry->setVertexArray(pVertexArray.get());
        pGeometry->setNormalArray(pNormalArray.get());
        pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->setTexCoordArray(0, pTexCoordArray.get());
        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertexArray->size()));
        pGeode->addDrawable(pGeometry.get());
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNoNormalAndAlphaState);
    }
    else
    {
        pGeode->setStateSet(m_pNoNormalState);
    }

    return pGeode.release();
}

osg::Node *CommonModelCreater::createRoundTableWithHole(float fTopRadius, float fBottomRadius, float fHeight, CoverType eCoverType, const std::vector<std::vector<osg::Vec3> > &vecTopHoleArray, const std::vector<std::vector<osg::Vec3> > &vecBottomHoleArray, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    const unsigned int nSegments = 40u;
    const unsigned int nRow = 3u;

    //创建侧面
    {
        const float fDeltaHeight = fHeight / (float)nRow;
        const float fDeltaRadius = (fBottomRadius - fTopRadius) / (float)nRow;

        const osg::Vec3 vCenter(0.0f, 0.0f, -fHeight * 0.5f);
        const float fTopAngle = atan2f((fBottomRadius - fTopRadius), fHeight);

        float fTexDeltaX;
        float fTexDeltaY;

        if(bTexStretched)
        {
            fTexDeltaX = 1.0f / (float)nSegments;
            fTexDeltaY = 1.0f / (float)nRow;
        }
        else
        {
            fTexDeltaX = 2.0f * osg::PI * std::max(fTopRadius, fBottomRadius) / (float)nSegments;
            fTexDeltaY = fHeight / (nRow * cosf(fTopAngle));
        }

        osg::ref_ptr<osg::Vec3Array> pCycleNormal = createCycleNormal(fTopAngle, nSegments);

        for(unsigned int i = 0; i < nRow; i++)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;

            osg::Vec3 vTempCenter1(vCenter[0], vCenter[1], vCenter[2] + i * fDeltaHeight);
            float fTempRadius1 = fBottomRadius - i * fDeltaRadius;
            osg::Vec3 vTempCenter2(vCenter[0], vCenter[1], vCenter[2] + (i + 1) * fDeltaHeight);
            float fTempRadius2 = fBottomRadius - (i + 1) * fDeltaRadius;

            //顶点
            osg::ref_ptr<osg::Vec3Array> pCycleVertex1 = createCycleVertex(vTempCenter1, fTempRadius1, nSegments);
            osg::ref_ptr<osg::Vec3Array> pCycleVertex2 = createCycleVertex(vTempCenter2, fTempRadius2, nSegments);
            for(unsigned int j = 0; j <= nSegments; j++)
            {
                pVertexArray->push_back((*pCycleVertex1)[j]);
                pVertexArray->push_back((*pCycleVertex2)[j]);

                pNormalArray->push_back((*pCycleNormal)[j]);
                pNormalArray->push_back((*pCycleNormal)[j]);

                pTexCoordArray->push_back(osg::Vec2(i * fTexDeltaY, j * fTexDeltaX));
                pTexCoordArray->push_back(osg::Vec2((i + 1) * fTexDeltaY, j * fTexDeltaX));
            }

            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, pVertexArray->size()));
            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeode->addDrawable(pGeometry.get()); 
        }
    }

    bool bHasTop = true;
    bool bHasBottom = true;

    switch(eCoverType)
    {
    case NONE:
        {
            bHasTop = false;
            bHasBottom = false;
            break;
        }
    case TOP:
        {
            bHasBottom = false;
            break;
        }
    case BOTTOM:
        {
            bHasTop = false;
            break;
        }
    }

    //上底下底的纹理坐标
    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;
    if(bTexStretched)
    {
        pTexCoordArray->push_back(osg::Vec2(0.5f, 0.5f));

        const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
        float fAngle = 0.0f;
        for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
        {
            pTexCoordArray->push_back(osg::Vec2(0.5f * cosf(fAngle) + 0.5f, 0.5f * sinf(fAngle) + 0.5f));
        }
        pTexCoordArray->push_back(osg::Vec2(1.0f, 0.5f));
    }

    if(bHasTop)
    {
        //上底有洞
        if(!vecTopHoleArray.empty())
        {
            std::vector<osg::Vec3> vecCircle;
            osg::ref_ptr<osg::Vec3Array> pTopCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, fHeight * 0.5f), fTopRadius, nSegments);
            vecCircle.insert(vecCircle.end(), pTopCycleVertex->begin(), pTopCycleVertex->begin() + pTopCycleVertex->size() - 1);

            osg::ref_ptr<osg::Geometry> pTopGeo = createSheetWithHoles(vecCircle, vecTopHoleArray, bTexStretched, vColor);
            pGeode->addDrawable(pTopGeo.get());
        }
        else
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

            //顶点索引
            pVertexArray->push_back(osg::Vec3(0.0f, 0.0f, fHeight * 0.5));
            osg::ref_ptr<osg::Vec3Array> pCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, fHeight * 0.5), fTopRadius, nSegments);
            pVertexArray->insert(pVertexArray->end(), pCycleVertex->begin(), pCycleVertex->end());

            //法向量
            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

            //纹理坐标
            if(!bTexStretched)
            {
                pTexCoordArray = new osg::Vec2Array;
                pTexCoordArray->push_back(osg::Vec2(fTopRadius, fTopRadius));

                const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
                float fAngle = 0.0f;
                for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
                {
                    pTexCoordArray->push_back(osg::Vec2(fTopRadius * cosf(fAngle) + fTopRadius, fTopRadius * sinf(fAngle) + fTopRadius));
                }
                pTexCoordArray->push_back(osg::Vec2(2.0f * fTopRadius, fTopRadius));
            }

            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertexArray->size()));
            pGeode->addDrawable(pGeometry.get());
        }
    }

    if(bHasBottom)
    {
        //下底有洞
        if(!vecBottomHoleArray.empty())
        {
            std::vector<osg::Vec3> vecCircle;
            osg::ref_ptr<osg::Vec3Array> pTopCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5f), fBottomRadius, nSegments);
            vecCircle.insert(vecCircle.end(), pTopCycleVertex->begin(), pTopCycleVertex->begin() + pTopCycleVertex->size() - 1);

            osg::ref_ptr<osg::Geometry> pBottomGeo = createSheetWithHoles(vecCircle, vecTopHoleArray, bTexStretched, vColor);
            pGeode->addDrawable(pBottomGeo.get());
        }
        else
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;

            //顶点索引
            pVertexArray->push_back(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5));
            osg::ref_ptr<osg::Vec3Array> pCycleVertex = createCycleVertex(osg::Vec3(0.0f, 0.0f, -fHeight * 0.5), fBottomRadius, nSegments);
            pVertexArray->insert(pVertexArray->end(), pCycleVertex->begin(), pCycleVertex->end());

            //法向量
            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));

            //纹理坐标
            if(!bTexStretched)
            {
                pTexCoordArray = new osg::Vec2Array;
                pTexCoordArray->push_back(osg::Vec2(fBottomRadius, fBottomRadius));

                const float fAngleDelta = 2.0f * osg::PI / (float)nSegments;
                float fAngle = 0.0f;
                for(unsigned int i = 0; i < nSegments; i++, fAngle += fAngleDelta)
                {
                    pTexCoordArray->push_back(osg::Vec2(fBottomRadius * cosf(fAngle) + fBottomRadius, fBottomRadius * sinf(fAngle) + fBottomRadius));
                }
                pTexCoordArray->push_back(osg::Vec2(2.0f * fBottomRadius, fBottomRadius));
            }

            pGeometry->setVertexArray(pVertexArray.get());
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->setTexCoordArray(0, pTexCoordArray.get());
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertexArray->size()));

            pGeode->addDrawable(pGeometry.get());

        }
    }

    if(vColor[3] < 0.98f)
    {
        pGeode->setStateSet(m_pNoNormalAndAlphaState.get());
    }
    else
    {
        pGeode->setStateSet(m_pNoNormalState.get());
    }

    return pGeode.release();
}

//创建多边形
osg::Geometry *CommonModelCreater::createPolygon(const osg::Vec3Array *pVertex, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(const_cast<osg::Vec3Array *>(pVertex));

    osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
    pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));

    if(vColor[3] < 0.98f)
    {
        pGeometry->setStateSet(m_pNoNormalAndAlphaState);
    }
    else
    {
        pGeometry->setStateSet(m_pNoNormalState);
    }

    return pGeometry.release();
}

osg::Geometry *CommonModelCreater::createPoint(const osg::Vec3 &point, float fPointSize, const osg::Vec4 &vColor)
{
    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
    pVertexArray->push_back(point);

    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
    pGeometry->setVertexArray(pVertexArray.get());
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    if(fPointSize > 1.0f)
    {
        osg::ref_ptr<osg::Point> pPoint = new osg::Point;
        pPoint->setSize(fPointSize);
        pGeometry->getOrCreateStateSet()->setAttribute(pPoint.get());
    }
    if(vColor[3] < 0.98f)
    {
        pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
        pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    return pGeometry.release();
}

osg::Geometry *CommonModelCreater::createLine(const osg::Vec3Array *pVertex, osg::PrimitiveSet::Mode eMode, float fLineWidth, bool bTexStretched, const osg::Vec4 &vColor)
{
    //颜色
    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    
    if(eMode != osg::PrimitiveSet::LINE_STRIP && eMode != osg::PrimitiveSet::LINE_LOOP && eMode != osg::PrimitiveSet::LINES)
    {
        return NULL;
    }

    const unsigned int nSize = pVertex->size();
    std::vector<float> vecLen;
    vecLen.push_back(0.0f);
    for(unsigned int i = 1; i < nSize; i++)
    {
        const osg::Vec3 vLen = (*pVertex)[i] - (*pVertex)[i - 1];
        vecLen.push_back(vLen.length());
    }

    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;
    //若使用纹理拉伸
    if(bTexStretched)
    {
        for(unsigned int i = 0; i < nSize; i++)
        {
            pTexCoordArray->push_back(osg::Vec2(vecLen[i] / vecLen[nSize - 1], 1.0f));
        }
    }
    else
    {
        for(unsigned int i = 0; i < nSize; i++)
        {
            pTexCoordArray->push_back(osg::Vec2(vecLen[i], 1.0f));
        }
    }

    pGeometry->addPrimitiveSet(new osg::DrawArrays(eMode, 0, pVertex->size()));
    pGeometry->setVertexArray(const_cast<osg::Vec3Array *>(pVertex));
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->setTexCoordArray(0, pTexCoordArray.get());

    if(fLineWidth > 1.0f)
    {
        osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
        pLineWidth->setWidth(fLineWidth);
        pGeometry->getOrCreateStateSet()->setAttribute(pLineWidth.get());
    }
    if(vColor[3] < 0.98f)
    {
        pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
        pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    return pGeometry.release();
}

osg::Vec3Array *CommonModelCreater::createCycleVertex(const osg::Vec3 &vCenter, float fRadius, unsigned int nSegments)
{
    osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;

    const float fDeltaAngle = 2.0f * osg::PI / (float)nSegments;
    float fAngle = 0.0f;
    for(unsigned int i = 0; i < nSegments; i++, fAngle += fDeltaAngle)
    {
        pVertex->push_back(osg::Vec3(vCenter[0] + cosf(fAngle) * fRadius, vCenter[1] + sinf(fAngle) * fRadius, vCenter[2]));
    }

    pVertex->push_back((*pVertex)[0]);

    return pVertex.release();
}

osg::Vec3Array *CommonModelCreater::createCycleNormal(float fTopAngle, unsigned int nSegments)
{
    osg::ref_ptr<osg::Vec3Array> pNormal = new osg::Vec3Array;

    const float fDeltaAngle = 2.0f * osg::PI / (float)nSegments;
    float fAngle = 0.0f;
    osg::Quat qt;
    for(unsigned int i = 0; i < nSegments; i++, fAngle += fDeltaAngle)
    {
        osg::Vec3 vVec1(cosf(fAngle), sinf(fAngle), 0.0f);
        osg::Vec3 vVec2 = vVec1 ^ osg::Vec3(0.0f, 0.0f, 1.0f);
        qt.makeRotate(fTopAngle, vVec2);
        vVec1 = qt * vVec1;
        vVec1.normalize();
        pNormal->push_back(vVec1);
    }

    pNormal->push_back((*pNormal)[0]);

    return pNormal.release();
}

osg::Vec2Array *CommonModelCreater::createTexCoordByVertex(const osg::Vec3Array *pVertexArray, bool bTexStretched)
{
    if(pVertexArray == NULL || pVertexArray->size() == 0)
    {
        return NULL;
    }

    osg::BoundingBox bb;
    const unsigned int nSize = pVertexArray->size();
    for(unsigned int i = 0; i < nSize; i++)
    {
        bb.expandBy((*pVertexArray)[i]);
    }

    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = new osg::Vec2Array;
    const float w = bb.xMax() - bb.xMin();
    const float h = bb.yMax() - bb.yMin();
    for(unsigned int i = 0; i < nSize; i++)
    {
        if(bTexStretched)
        {
            pTexCoordArray->push_back(osg::Vec2(((*pVertexArray)[i]._v[0] - bb.xMin()) / w, ((*pVertexArray)[i]._v[1] - bb.yMin()) / h));
        }
        else
        {
            pTexCoordArray->push_back(osg::Vec2((*pVertexArray)[i]._v[0] - bb.xMin(), (*pVertexArray)[i]._v[1] - bb.yMin()));
        }
    }

    return pTexCoordArray.release();
}

osg::Geometry *CommonModelCreater::createSheetWithHoles(const std::vector<osg::Vec3> &vecVertices, const std::vector<std::vector<osg::Vec3> > &vecHoleVertices, bool bTexStretched, const osg::Vec4 &vColor)
{
    osg::ref_ptr<osg::Geometry>     pTessGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array>    pTessVtxArray = new osg::Vec3Array;
    pTessGeometry->setVertexArray(pTessVtxArray.get());

    pTessVtxArray->assign(vecVertices.begin(), vecVertices.end());
    unsigned nLength = pTessVtxArray->size();

    pTessGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, nLength));

    for(unsigned int i = 0u; i < vecHoleVertices.size(); i++)
    {
        pTessVtxArray->insert(pTessVtxArray->end(), vecHoleVertices[i].begin(), vecHoleVertices[i].end());

        pTessGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, nLength, vecHoleVertices[i].size()));

        nLength += vecHoleVertices[i].size();
    }

    osg::ref_ptr<osgUtil::Tessellator> pTessellator = new osgUtil::Tessellator;
    pTessellator->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    pTessellator->setBoundaryOnly(false);
    pTessellator->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
    pTessellator->retessellatePolygons(*pTessGeometry);

    osg::Vec3Array *pVertexArray = dynamic_cast<osg::Vec3Array *>(pTessGeometry->getVertexArray());
    osg::ref_ptr<osg::Vec2Array> pTexCoordArray = createTexCoordByVertex(pVertexArray, bTexStretched);
    pTessGeometry->setTexCoordArray(0, pTexCoordArray.get());

    osg::ref_ptr<osg::Vec4Array> pColorArray;
    getColorArray(vColor, pColorArray);
    pTessGeometry->setColorArray(pColorArray.get());
    pTessGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    return pTessGeometry.release();
}

osg::Node *CommonModelCreater::createSector(float fBeginAngle, float fEndAngle, float fRadius1, float fRadius2, bool bTexStretched, const osg::Vec4 &vFaceColor, const osg::Vec4 &vBorderColor, float fBorderWidth)
{
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    const float fltAngleDelta = fEndAngle - fBeginAngle;
    const unsigned nSegCount  = 60u;
    const float fltAngleBias  = fltAngleDelta / (float)nSegCount;

    const float fltOuterRadius = std::max(fRadius1, fRadius2);
    const float fltInnerRadius = std::min(fRadius1, fRadius2);

    osg::ref_ptr<osg::Vec3Array>    pVerticesArray = new osg::Vec3Array;

    // 标准扇形
    if(cmm::math::floatEqual(fltInnerRadius, 0.0f))
    {
        pVerticesArray->resize(nSegCount + 2u);
        pVerticesArray->at(0u).set(0.0f, 0.0f, 0.0f);

        float dblAngle = fBeginAngle;
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            osg::Vec3 &vtx = pVerticesArray->at(n + 1u);
            vtx.x() = cos(dblAngle) * fltOuterRadius;
            vtx.y() = sin(dblAngle) * fltOuterRadius;
            vtx.z() = 0.0f;

            dblAngle += fltAngleBias;
        }

        //面
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            pGeometry->setVertexArray(pVerticesArray);
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVerticesArray->size()));

            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

            osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
            pColorArray->push_back(vFaceColor);
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            if(vFaceColor[3] < 0.98f)
            {
                pGeometry->setStateSet(m_pNoNormalAndAlphaState);
            }
            else
            {
                pGeometry->setStateSet(m_pNoNormalState);
            }

            pGeode->addDrawable(pGeometry.get());
        }

        //边框
        if(fBorderWidth > 0.98f)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            pGeometry->setVertexArray(pVerticesArray);

            if(fltAngleDelta < osg::PI * 2.0f - FLT_EPSILON)
            {
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, pVerticesArray->size()));
            }
            else
            {
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 1, pVerticesArray->size() - 1));
            }

            osg::Vec4Array *pBorderColor = new osg::Vec4Array;
            pBorderColor->push_back(vBorderColor);
            pGeometry->setColorArray(pBorderColor);
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            if(fBorderWidth > 1.1f)
            {
                osg::LineWidth *pLineWidth = new osg::LineWidth(fBorderWidth);
                osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();
                pStateSet->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            }

            pGeode->addDrawable(pGeometry.get());
        }

    }
    else if(cmm::math::floatEqual(fltInnerRadius, fltOuterRadius))
    {
        pVerticesArray->resize(nSegCount + 1u);
        float fltAngle = fBeginAngle;
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            osg::Vec3 &vtx = pVerticesArray->at(n);
            vtx.x() = cos(fltAngle) * fRadius1;
            vtx.y() = sin(fltAngle) * fRadius1;
            vtx.z() = 0.0f;

            fltAngle += fltAngleBias;
        }

        //边框
        if(fBorderWidth > 0.98f)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            pGeometry->setVertexArray(pVerticesArray);

            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pVerticesArray->size()));

            osg::Vec4Array *pBorderColor = new osg::Vec4Array;
            pBorderColor->push_back(vBorderColor);
            pGeometry->setColorArray(pBorderColor);
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            if(fBorderWidth > 1.1f)
            {
                osg::LineWidth *pLineWidth = new osg::LineWidth(fBorderWidth);
                osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();
                pStateSet->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            }

            pGeode->addDrawable(pGeometry.get());
        }
    }
    else
    {
        // 扇环
        pVerticesArray->resize(nSegCount + nSegCount + 2u);

        float fltAngle = fBeginAngle;
        for(unsigned n = 0u; n <= nSegCount; n++)
        {
            const float fltCos = cos(fltAngle);
            const float fltSin = sin(fltAngle);

            osg::Vec3 &vtx0 = pVerticesArray->at(n + n);
            osg::Vec3 &vtx1 = pVerticesArray->at(n + n + 1u);

            vtx0.set(fltCos * fltOuterRadius, fltSin * fltOuterRadius, 0.0f);
            vtx1.set(fltCos * fltInnerRadius, fltSin * fltInnerRadius, 0.0f);

            fltAngle += fltAngleBias;
        }

        //面
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            pGeometry->setVertexArray(pVerticesArray);
            pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, pVerticesArray->size()));

            osg::ref_ptr<osg::Vec3Array> pNormalArray = new osg::Vec3Array;
            pNormalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
            pGeometry->setNormalArray(pNormalArray.get());
            pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

            osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
            pColorArray->push_back(vFaceColor);
            pGeometry->setColorArray(pColorArray.get());
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            if(vFaceColor[3u] < 0.98f)
            {
                pGeometry->setStateSet(m_pNoNormalAndAlphaState);
            }
            else
            {
                pGeometry->setStateSet(m_pNoNormalState);
            }

            pGeode->addDrawable(pGeometry.get());
        }

        //边框
        if(fBorderWidth > 0.98f)
        {
            osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
            pGeometry->setVertexArray(pVerticesArray);

            if(fltAngleDelta < osg::PI * 2.0f - FLT_EPSILON)
            {
                osg::ref_ptr<osg::UShortArray> pVertexIndexArray = new osg::UShortArray;
                pVertexIndexArray->reserve(pVerticesArray->size());
                for(unsigned int i = 0u; i < pVerticesArray->size(); i += 2u)
                {
                    pVertexIndexArray->push_back(i);
                }
                for(unsigned int i = 1u; i < pVerticesArray->size(); i += 2u)
                {
                    pVertexIndexArray->push_back(pVerticesArray->size() - i);
                }

                pGeometry->setVertexIndices(pVertexIndexArray.get());
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0u, pVertexIndexArray->size()));
            }
            //整圆，边框为两个圆环
            else
            {
                osg::ref_ptr<osg::UShortArray> pVertexIndexArray1 = new osg::UShortArray;
                osg::ref_ptr<osg::UShortArray> pVertexIndexArray2 = new osg::UShortArray;

                pVertexIndexArray1->reserve(pVerticesArray->size() / 2u);
                for(unsigned int i = 0u; i < pVerticesArray->size(); i += 2u)
                {
                    pVertexIndexArray1->push_back(i);
                }

                pVertexIndexArray2->reserve(pVerticesArray->size() / 2u);
                for(unsigned int i = 1u; i < pVerticesArray->size(); i += 2u)
                {
                    pVertexIndexArray2->push_back(i);
                }
                pGeometry->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::LINE_LOOP, pVertexIndexArray1->begin(), pVertexIndexArray1->end()));
                pGeometry->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::LINE_LOOP, pVertexIndexArray2->begin(), pVertexIndexArray2->end()));
            }

            osg::Vec4Array *pBorderColor = new osg::Vec4Array;
            pBorderColor->push_back(vBorderColor);
            pGeometry->setColorArray(pBorderColor);
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            if(fBorderWidth > 1.1f)
            {
                osg::LineWidth *pLineWidth = new osg::LineWidth(fBorderWidth);
                osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();
                pStateSet->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            }

            pGeode->addDrawable(pGeometry.get());
        }
    }

    return pGeode.release();
}

}