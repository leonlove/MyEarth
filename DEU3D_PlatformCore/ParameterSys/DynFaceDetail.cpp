#include "DynFaceDetail.h"

#include <IDProvider/Definer.h>

#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/SharedStateAttributes>

namespace param
{

DynFaceDetail::DynFaceDetail(void)
{
    m_dblBorderWidth = 1.0;

    m_FaceClr.m_fltR = 1.0;
    m_FaceClr.m_fltG = 1.0;
    m_FaceClr.m_fltB = 1.0;
    m_FaceClr.m_fltA = 1.0;

    m_BorderClr.m_fltR = 1.0;
    m_BorderClr.m_fltG = 1.0;
    m_BorderClr.m_fltB = 1.0;
    m_BorderClr.m_fltA = 1.0;
}

DynFaceDetail::DynFaceDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_DYN_FACE_ID))
{
    m_dblBorderWidth = 1.0;

    m_FaceClr.m_fltR = 1.0;
    m_FaceClr.m_fltG = 1.0;
    m_FaceClr.m_fltB = 1.0;
    m_FaceClr.m_fltA = 1.0;

    m_BorderClr.m_fltR = 1.0;
    m_BorderClr.m_fltG = 1.0;
    m_BorderClr.m_fltB = 1.0;
    m_BorderClr.m_fltA = 1.0;
}

DynFaceDetail::~DynFaceDetail(void)
{

}

bool DynFaceDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Detail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;
    {
        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("BorderWidth"));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        setBorderWidth(pDoubleEle->DblValue());
    }

    bson::bsonArrayEle *pArrayEle = NULL;
    {
        pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("FaceColor"));
        if(pArrayEle == NULL || pArrayEle->ChildCount() != 4)
        {
            return false;
        }

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_FaceClr.m_fltR = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_FaceClr.m_fltG = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_FaceClr.m_fltB = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_FaceClr.m_fltA = pDoubleEle->DblValue();
    }

    {
        pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("BorderColor"));
        if(pArrayEle == NULL || pArrayEle->ChildCount() != 4)
        {
            return false;
        }

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_BorderClr.m_fltR = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_BorderClr.m_fltG = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_BorderClr.m_fltB = pDoubleEle->DblValue();

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        m_BorderClr.m_fltA = pDoubleEle->DblValue();
    }

    return true;
}

bool DynFaceDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("BorderWidth", m_dblBorderWidth))
    {
        return false;
    }

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("FaceColor"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_FaceClr.m_fltR) ||
        !pArrayEle->AddDblElement(m_FaceClr.m_fltG) ||
        !pArrayEle->AddDblElement(m_FaceClr.m_fltB) ||
        !pArrayEle->AddDblElement(m_FaceClr.m_fltA))
    {
        return false;
    }

    pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BorderColor"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_BorderClr.m_fltR) ||
        !pArrayEle->AddDblElement(m_BorderClr.m_fltG) ||
        !pArrayEle->AddDblElement(m_BorderClr.m_fltB) ||
        !pArrayEle->AddDblElement(m_FaceClr.m_fltA))
    {
        return false;
    }

    return true;
}

//创建动态面模型
osg::Node *DynFaceDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PolyCreationInfo *pPolyInfo = dynamic_cast<const PolyCreationInfo *>(pInfo);
    if(pPolyInfo == NULL)
    {
        return NULL;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::ref_ptr<osg::Vec3dArray> pCorrdArray = new osg::Vec3dArray;
    osg::Vec3d vTemp1, vTemp2;
    for(unsigned int i = pPolyInfo->m_nOffset; i < pPolyInfo->m_nOffset + pPolyInfo->m_nCount; i++)
    {
        pEllipsoidModel->convertLatLongHeightToXYZ((*pPolyInfo->m_pPoints)[pPolyInfo->m_nOffset + i][1],
                                                   (*pPolyInfo->m_pPoints)[pPolyInfo->m_nOffset + i][0],
                                                   (*pPolyInfo->m_pPoints)[pPolyInfo->m_nOffset + i][2],
                                                    vTemp1[0], vTemp1[1], vTemp1[2]);
        pCorrdArray->push_back(vTemp1);
        vTemp2 += vTemp1;
    }

    vTemp2 /= (double)pPolyInfo->m_nCount;
    osg::Matrix mtxTrans;
    mtxTrans.setTrans(vTemp2);

    for(unsigned int i = 0; i < pCorrdArray->size(); i++)
    {
        (*pCorrdArray)[i] -= vTemp2;
    }

    osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;
    pVertex->assign(pCorrdArray->begin(), pCorrdArray->end());

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    pMatrixTransform->setMatrix(mtxTrans);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pMatrixTransform->addChild(pGeode);

    //Border
    {
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        pGeode->addDrawable(pGeometry);

        pGeometry->setVertexArray(pVertex.get());
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pVertex->size()));

        //设置属性
        osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();

        osg::Vec4 vClr(m_BorderClr.m_fltR, m_BorderClr.m_fltG, m_BorderClr.m_fltB, m_BorderClr.m_fltA);
        osg::Material *pMaterial = osg::SharedStateAttributes::instance()->getMaterialByColor(vClr);
        pStateSet->setAttributeAndModes(pMaterial);
        if(vClr[3] < 0.98)
        {
            pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }

        if(m_dblBorderWidth > 1.0)
        {
            osg::LineWidth *pLineWidth = osg::SharedStateAttributes::instance()->getLineWidth(m_dblBorderWidth);
            pStateSet->setAttribute(pLineWidth);
        }
    }

    //Face
    {

        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        pGeode->addDrawable(pGeometry);

        pGeometry->setVertexArray(pVertex.get());
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertex->size()));

        //设置属性
        osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();

        osg::Vec4 vClr(m_FaceClr.m_fltR, m_FaceClr.m_fltG, m_FaceClr.m_fltB, m_FaceClr.m_fltA);
        osg::Material *pMaterial = osg::SharedStateAttributes::instance()->getMaterialByColor(vClr);
        pStateSet->setAttributeAndModes(pMaterial);
        if(vClr[3] < 0.98)
        {
            pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
    }

    return pMatrixTransform.release();
}

}