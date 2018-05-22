#include "DynLineDetail.h"

#include <IDProvider/Definer.h>

#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineNode>

#include <osg/SharedStateAttributes>

namespace param
{

DynLineDetail::DynLineDetail()
{
    m_dblLineWidth = 1.0;
    m_LineClr.m_fltR = 1.0;
    m_LineClr.m_fltG = 1.0;
    m_LineClr.m_fltB = 1.0;
    m_LineClr.m_fltA = 1.0;
}

DynLineDetail::DynLineDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_DYN_LINE_ID))
{
    m_dblLineWidth = 1.0;
    m_LineClr.m_fltR = 1.0;
    m_LineClr.m_fltG = 1.0;
    m_LineClr.m_fltB = 1.0;
    m_LineClr.m_fltA = 1.0;
}

DynLineDetail::~DynLineDetail(void)
{

}

bool DynLineDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Detail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonElement *pEle = bsonDoc.GetElement("ImageID");
    if(pEle == NULL)
    {
        return false;
    }

    bson::bsonElementType eType = pEle->GetType();
    if(eType == bson::bsonStringType)
    {
        bson::bsonStringEle *pStringEle = dynamic_cast<bson::bsonStringEle *>(pEle);
        m_LineImgID = ID::genIDfromString(pStringEle->StrValue());
    }
    else if(eType == bson::bsonBinType)
    {
        bson::bsonBinaryEle *pBinEle = dynamic_cast<bson::bsonBinaryEle *>(pEle);
        m_LineImgID = ID::genIDfromBinary(pBinEle->BinData(), pBinEle->BinDataLen());
    }
    else
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;
    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Width"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setLineWidth(pDoubleEle->DblValue());


    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Color"));
    if(pArrayEle == NULL || pArrayEle->ChildCount() != 4)
    {
        return false;
    }

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_LineClr.m_fltR = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_LineClr.m_fltG = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_LineClr.m_fltB = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_LineClr.m_fltA = pDoubleEle->DblValue();

    return true;
}

bool DynLineDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("ImageID", (void *)&m_LineImgID, sizeof(ID)) ||
        !bsonDoc.AddDblElement("Width", m_dblLineWidth))
    {
        return false;
    }

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Color"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_LineClr.m_fltR) ||
        !pArrayEle->AddDblElement(m_LineClr.m_fltG) ||
        !pArrayEle->AddDblElement(m_LineClr.m_fltB) ||
        !pArrayEle->AddDblElement(m_LineClr.m_fltA))
    {
        return false;
    }

    return true;
}

//创建动态线模型
osg::Node *DynLineDetail::createDetailNode(const CreationInfo *pInfo) const
{
	const PolyCreationInfo *pPolyInfo = dynamic_cast<const PolyCreationInfo *>(pInfo);
	if(pPolyInfo == NULL || pPolyInfo->m_nCount < 2u)
	{
		return NULL;
	}

// 	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
// 	osg::ref_ptr<osg::LineNode> pLineNode = new osg::LineNode;
// 	pLineNode->setColor(osg::Vec4(m_LineClr.m_fltR, m_LineClr.m_fltG, m_LineClr.m_fltB, m_LineClr.m_fltA));
// 	pLineNode->setLineWidth(m_dblLineWidth);
// 
// 	for(unsigned int i = pPolyInfo->m_nOffset; i < pPolyInfo->m_nOffset + pPolyInfo->m_nCount; i++)
// 	{
// 		osg::Vec3d point;
// 		const osg::Vec3d &coord = pPolyInfo->m_pPoints->at(i);
// 		pEllipsoidModel->convertLatLongHeightToXYZ(coord.y(), coord.x(), coord.z(), point.x(), point.y(), point.z());
// 		pLineNode->addVertex(point);
// 	}

	osg::ref_ptr<osg::Node> pLineNode = createAsNonIntegration(pPolyInfo);

	return pLineNode.release();
}

osg::Node* DynLineDetail::createAsNonIntegration(const PolyCreationInfo* pPolyInfo) const
{
	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
	std::vector<osg::Vec3d> vecVertices;

	for(unsigned int i = pPolyInfo->m_nOffset; i < pPolyInfo->m_nOffset + pPolyInfo->m_nCount; i++)
	{
		osg::Vec3d point;
		const osg::Vec3d &coord = pPolyInfo->m_pPoints->at(i);
		pEllipsoidModel->convertLatLongHeightToXYZ(coord.y(), coord.x(), coord.z(), point.x(), point.y(), point.z());

		vecVertices.push_back(point);
	}

	if(vecVertices.size() < 2u)   return NULL;

	unsigned int i = 0u;
	osg::Vec3d ptCenter(0.0, 0.0, 0.0);
	for(i = 0u; i < vecVertices.size(); i++)
	{
		ptCenter += vecVertices[i];
	}
	ptCenter /= double(vecVertices.size());

	osg::ref_ptr<osg::Vec3Array>   pCoordArray = new osg::Vec3Array;
	for(i = 0u; i < vecVertices.size(); i++)
	{
		const osg::Vec3d &vtx = vecVertices[i];
		pCoordArray->push_back(vtx - ptCenter);
	}

	float fTotalLen = 0.0;
	for(i = 1u; i < pCoordArray->size(); i++)
	{
		const osg::Vec3 &point = (*pCoordArray)[i];

		const osg::Vec3d &ptPrev = (*pCoordArray)[i - 1u];
		const float fltLen = (ptPrev - point).length();
		fTotalLen += fltLen;
	}


	osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(osg::Matrixd::translate(ptCenter));

	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
	pMatrixTransform->addChild(pGeode);

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
	pGeode->addDrawable(pGeometry);

	pGeometry->setVertexArray(pCoordArray.get());
	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pCoordArray->size()));

	//设置属性
	osg::StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();
	pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

	osg::ref_ptr<osg::Vec4Array> pColorArray = osg::SharedStateAttributes::instance()->getColorArrayByColor(osg::Vec4(m_LineClr.m_fltR, m_LineClr.m_fltG, m_LineClr.m_fltB, m_LineClr.m_fltA));
	pGeometry->setColorArray(pColorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	if(m_dblLineWidth > 1.0f)
	{
		osg::LineWidth *pLineWidth = osg::SharedStateAttributes::instance()->getLineWidth(m_dblLineWidth);
		pStateSet->setAttribute(pLineWidth);
	}

	return pMatrixTransform.release();
}

}