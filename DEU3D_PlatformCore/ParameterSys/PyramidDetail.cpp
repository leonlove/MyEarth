#include "PyramidDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/Tessellator>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geometry>
#include <osg/Geode>

namespace param
{

PyramidDetail::PyramidDetail(void)
{

}

PyramidDetail::PyramidDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_PYRAMID_ID)
{

}

PyramidDetail::~PyramidDetail(void)
{

}


bool PyramidDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonBoolEle *pBottomVisible = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("BottomVisible"));
    if(pBottomVisible == NULL)
    {
        return false;
    }
    m_bBottomVisible = pBottomVisible->BoolValue();

    bson::bsonBinaryEle *pBottomImgID = dynamic_cast<bson::bsonBinaryEle *>(bsonDoc.GetElement("BottomImgID"));
    if(pBottomImgID == NULL)
    {
        return false;
    }
    m_BottomImgID = ID::genIDfromBinary(pBottomImgID->BinData(), pBottomImgID->BinDataLen());

    bson::bsonArrayEle * arr = (bson::bsonArrayEle*)bsonDoc.GetElement("BottomVertices");
    if (!arr || arr->ChildCount() % 3 != 0) 
    {
        return false;
    }

    for(unsigned i = 0; i < arr->ChildCount(); )
    {
        bson::bsonDoubleEle *x = dynamic_cast<bson::bsonDoubleEle *>(arr->GetElement(i++));
        if (!x) return false;

        bson::bsonDoubleEle *y = dynamic_cast<bson::bsonDoubleEle *>(arr->GetElement(i++));
        if (!y) return false;

        bson::bsonDoubleEle *z = dynamic_cast<bson::bsonDoubleEle *>(arr->GetElement(i++));
        if (!z) return false;

        addBottomVertex(cmm::math::Point3d(x->DblValue(), y->DblValue(), z->DblValue()));
    }
    return !m_vecBottomVertices.empty();
}


bool PyramidDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if (m_vecBottomVertices.empty())
    {
        return false;
    }

    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBoolElement("BottomVisible", m_bBottomVisible))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("BottomImgID", (void *)&m_BottomImgID, sizeof(ID)))
    {
        return false;
    }

    bson::bsonArrayEle *arr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("BottomVertices"));
    for(size_t i = 0; i < m_vecBottomVertices.size(); ++i)
    {
        arr->AddDblElement(m_vecBottomVertices[i].x());
        arr->AddDblElement(m_vecBottomVertices[i].y());
        arr->AddDblElement(m_vecBottomVertices[i].z());
    }

    return true;
}


double PyramidDetail::getBoundingSphereRadius(void) const
{
	return 10;
    cmm::math::Sphered s;
    for(size_t i = 0; i < m_vecBottomVertices.size(); i++)
    {
        s.expandBy(m_vecBottomVertices[i]);
    }
    return s.getRadius();
}

void computeTextureCoord(osg::Geometry *pGeometry)
{
    if(pGeometry == NULL)   return;

    osg::Vec3Array *pVertex = dynamic_cast<osg::Vec3Array *>(pGeometry->getVertexArray());

    osg::BoundingBox bb;
    unsigned int nSize = pVertex->size();
    for(unsigned int i = 0; i < nSize; i++)
    {
        bb.expandBy((*pVertex)[i]);
    }

    osg::ref_ptr<osg::Vec2Array> pNormal = new osg::Vec2Array;
    float w = bb.xMax() - bb.xMin();
    float h = bb.yMax() - bb.yMin();
    for(unsigned int i = 0; i < nSize; i++)
    {
        float x = ((*pVertex)[i]._v[0] - bb.xMin()) / w;
        float y = ((*pVertex)[i]._v[1] - bb.yMin()) / h;
        pNormal->push_back(osg::Vec2(x, y));
    }

    pGeometry->setTexCoordArray(0, pNormal);

    return;
}

osg::Drawable *createPolygonSheet(const std::vector<osg::Vec3d> &vecVertices, bool bGenTexCoord)
{
	
    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;
    pVtxArray->assign(vecVertices.rbegin(), vecVertices.rend());

    osg::Vec3 vec0 = vecVertices[0] - vecVertices[1];
    vec0.normalize();
    osg::Vec3 vec1 = vecVertices[2] - vecVertices[1];
    vec1.normalize();

    osg::Vec3 vecNormal = vec1 ^ vec0;
    vecNormal.normalize();

    osg::ref_ptr<osg::Vec3Array>   pNormalArray = new osg::Vec3Array;
    pNormalArray->push_back(vecNormal);

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());
    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    if(bGenTexCoord)
    {
        osg::Vec2d  ptMin(FLT_MAX, FLT_MAX), ptMax(-FLT_MAX, -FLT_MAX);
        std::vector<osg::Vec3d>::const_iterator itorVtx = vecVertices.begin();
        for( ; itorVtx != vecVertices.end(); ++itorVtx)
        {
            const osg::Vec3d &vtx = *itorVtx;
            if(vtx.x() < ptMin.x())    ptMin.x() = vtx.x();
            if(vtx.y() < ptMin.y())    ptMin.y() = vtx.y();
            if(vtx.x() > ptMax.x())    ptMax.x() = vtx.x();
            if(vtx.y() > ptMax.y())    ptMax.y() = vtx.y();
        }

        osg::Vec2d vecTexRatio = ptMax - ptMin;
        vecTexRatio.x() = osg::clampAbove(vecTexRatio.x(), 0.0001);
        vecTexRatio.y() = osg::clampAbove(vecTexRatio.y(), 0.0001);

        osg::ref_ptr<osg::Vec2Array>   pTextureCoord = new osg::Vec2Array;
        for(itorVtx = vecVertices.begin(); itorVtx != vecVertices.end(); ++itorVtx)
        {
            const osg::Vec3d &vtx = *itorVtx;

            osg::Vec2d ptCoord;
            ptCoord.x() = (vtx.x() - ptMin.x()) / vecTexRatio.x();
            ptCoord.y() = (vtx.y() - ptMin.y()) / vecTexRatio.y();
            pTextureCoord->push_back(ptCoord);
        }

        pGeometry->setTexCoordArray(0, pTextureCoord.get());
    }

    osg::ref_ptr<osg::DrawArrays>   pDrawArray = new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArray.get());

    osg::ref_ptr<osgUtil::Tessellator>  pTessellator = new osgUtil::Tessellator;
    pTessellator->retessellatePolygons(*pGeometry);

    return pGeometry.release();
}

osg::Node *PyramidDetail::createDetailNode(const CreationInfo *pInfo) const
{
	const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    if(m_vecBottomVertices.size() < 3u)
    {
        return NULL;
    }

	osg::Matrixd matrix;

    const osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern   = pEllipsoidModel->computeLocalEastern(vPoint.y(), vPoint.x());
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(vPoint.y(), vPoint.x());
    const osg::Quat  qtStand(osg::Vec3d(0.0, 0.0, -1.0), vecPlumbLine);

    const osg::Quat  qtAzimuth(pPointInfo->m_dblAzimuthAngle, -vecPlumbLine);

    const osg::Vec3d vecLocalRight = qtAzimuth * vecEastern;
    const osg::Quat  qtPitch(pPointInfo->m_dblPitchAngle, vecLocalRight);

    const osg::Quat  qtRotation = qtStand * qtAzimuth * qtPitch;

    //Ðý×ª
    matrix.postMultRotate(qtRotation);

    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    //Æ½ÒÆ
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);
    
    const osg::Quat qtStand1(vecPlumbLine, osg::Vec3d(0.0, 0.0, -1.0));
    std::vector<osg::Vec3d> vecVertices;

    for(unsigned int i = 0; i < m_vecBottomVertices.size(); i++)
    {
        const cmm::math::Point3d &vtx = m_vecBottomVertices[i];

        osg::Vec3d vCoord;
        pEllipsoidModel->convertLatLongHeightToXYZ(vtx.y(), vtx.x(), vtx.z(), vCoord.x(), vCoord.y(), vCoord.z());
        vCoord -= vecTrans;
        vCoord = qtStand1 * vCoord;
        vecVertices.push_back(vCoord);
    }

    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
	osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);

	osg::ref_ptr<osg::StateSet> pState = pGeode->getOrCreateStateSet();
    pState->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);
    pState->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
    pState->setMode(GL_BLEND, osg::StateAttribute::ON);
    pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	
    //const ID &imgSide = pPyramidDetail->getImageID();
    {
        osg::ref_ptr<osg::Geometry>     pSideFace = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array>    pSideVtx = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array>    pSideNormal = new osg::Vec3Array;

		for(size_t i = 0; i < vecVertices.size(); i++)
        {
            const osg::Vec3d vtx0(0.0, 0.0, 0.0);
            const osg::Vec3d &vtx1 = vecVertices[i];
			const osg::Vec3d &vtx2 = vecVertices[i + 1 == vecVertices.size() ? 0 : i + 1];
            pSideVtx->push_back(vtx0);
            pSideVtx->push_back(vtx1);
            pSideVtx->push_back(vtx2);

            const osg::Vec3d vec1 = vtx1 - vtx0;
            const osg::Vec3d vec2 = vtx2 - vtx0;
            osg::Vec3d vecNormal = vec1 ^ vec2;
            vecNormal.normalize();
            pSideNormal->push_back(vecNormal);
        }

        pSideFace->setVertexArray(pSideVtx.get());
        pSideFace->setNormalArray(pSideNormal.get());
        pSideFace->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

		const osg::Vec4 vColor(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA);
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(osg::Vec4(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA));
		pSideFace->setColorArray(color.get());
		pSideFace->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, pSideVtx->size());
        pSideFace->addPrimitiveSet(pDrawArrays.get());
        pGeode->addDrawable(pSideFace.get());
    }

    if(m_bBottomVisible)
    {
        const ID &imgBottom = getBottomImageID();
        const bool bHasTexture = imgBottom.isValid();
        std::vector<osg::Vec3d>    vecBottomVertices(vecVertices.begin(), vecVertices.end());
        osg::ref_ptr<osg::Drawable> pBottomFace;
        pBottomFace = createPolygonSheet(vecBottomVertices, bHasTexture);
        if(pBottomFace.valid())
        {
            osg::StateSet *pStateSet = pBottomFace->getOrCreateStateSet();
            if(bHasTexture)
            {
                computeTextureCoord(pBottomFace->asGeometry());
                bindTexture(imgBottom, pStateSet);
            }
            else
            {
                pStateSet->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
            }

            osg::Geometry *pGeometry = pBottomFace->asGeometry();
            if(pGeometry != NULL)
            {
				const cmm::FloatColor &color = getDynModelColor();
				osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array(1);
				(*pClrArray)[0].set(color.m_fltR, color.m_fltG, color.m_fltB, color.m_fltA);

                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            }
            pGeode->addDrawable(pBottomFace.get());
        }
    }

    pMatrixTransform->addChild(pGeode.get());

    return pMatrixTransform.release();
}

}