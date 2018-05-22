#include "PrismDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/CommonOSG.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/TexMat>

namespace param
{

PrismDetail::PrismDetail(void)
{

}

PrismDetail::PrismDetail(unsigned int nDataSetCode): DynModelDetail(nDataSetCode, DETAIL_PRISM_ID)
{
    m_dblHeight = 1.0;
    m_TopImgID.setInvalid();
    m_BottomImgID.setInvalid();
    m_bBottomVisible = true;
    m_bTopVisible = true;
}


PrismDetail::~PrismDetail(void)
{

}


bool PrismDetail::fromBson(bson::bsonDocument &bsonDoc)
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

    bson::bsonBoolEle *pTopVisible = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("TopVisible"));
    if(pTopVisible == NULL)
    {
        return false;
    }
    m_bTopVisible = pTopVisible->BoolValue();

    bson::bsonDoubleEle *pHeight = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Height"));
    if(pHeight == NULL)
    {
        return false;
    }
    m_dblHeight = pHeight->DblValue();

    bson::bsonBinaryEle *pTopImgID = dynamic_cast<bson::bsonBinaryEle *>(bsonDoc.GetElement("TopImgID"));
    if(pTopImgID == NULL)
    {
        return false;
    }
    m_TopImgID = ID::genIDfromBinary(pTopImgID->BinData(), pTopImgID->BinDataLen());

    bson::bsonBinaryEle *pBottomImgID = dynamic_cast<bson::bsonBinaryEle *>(bsonDoc.GetElement("BottomImgID"));
    if(pBottomImgID == NULL)
    {
        return false;
    }
    m_BottomImgID = ID::genIDfromBinary(pBottomImgID->BinData(), pBottomImgID->BinDataLen());

    bson::bsonArrayEle * arr = (bson::bsonArrayEle*)bsonDoc.GetElement("Vertices");
    if (!arr || arr->ChildCount() % 2 != 0) 
    {
        return false;
    }

    for(unsigned i = 0; i < arr->ChildCount(); )
    {
        bson::bsonDoubleEle *x = dynamic_cast<bson::bsonDoubleEle *>(arr->GetElement(i++));
        if (!x)
        {
            return false;
        }

        bson::bsonDoubleEle *y = dynamic_cast<bson::bsonDoubleEle *>(arr->GetElement(i++));
        if (!y)
        {
            return false;
        }

        addVertex(cmm::math::Point2d(x->DblValue(), y->DblValue()));
    }

    return m_vecVertices.size() >= 3;
}


bool PrismDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if (m_vecVertices.size() < 3) 
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
    if(!bsonDoc.AddBoolElement("TopVisible", m_bTopVisible))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Height", m_dblHeight))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("TopImgID", (void *)&m_TopImgID, sizeof(ID)))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("BottomImgID", (void *)&m_BottomImgID, sizeof(ID)))
    {
        return false;
    }

    bson::bsonArrayEle *arr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Vertices"));
    for(size_t i = 0; i < m_vecVertices.size(); ++i)
    {
        arr->AddDblElement(m_vecVertices[i].x());
        arr->AddDblElement(m_vecVertices[i].y());
    }
    return true;
}


double PrismDetail::getBoundingSphereRadius(void) const
{
    cmm::math::Sphered s;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    std::vector<cmm::math::Point2d>::const_iterator itorVtx = m_vecVertices.begin();
    for( ; itorVtx != m_vecVertices.end(); ++itorVtx)
    {
        const cmm::math::Point2d &vtx = *itorVtx;

        cmm::math::Point3d top;
        pEllipsoidModel->convertLatLongHeightToXYZ(vtx.y(), vtx.x(), m_dblHeight * 0.5, top.x(), top.y(), top.z());

        cmm::math::Point3d bottom;
        pEllipsoidModel->convertLatLongHeightToXYZ(vtx.y(), vtx.x(), m_dblHeight * -0.5, bottom.x(), bottom.y(), bottom.z());

        s.expandBy(top);
        s.expandBy(bottom);
    }

    return s.getRadius();
}


//创建棱柱
osg::Node *PrismDetail::createDetailNode(const CreationInfo *pInfo) const
{
    if(m_vecVertices.size() < 2u)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Node> pNode;
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo)
    {
        pNode = createAsPointParameter(pPointInfo);
    }

    const PolyCreationInfo *pPolyInfo = dynamic_cast<const PolyCreationInfo *>(pInfo);
    if(pPolyInfo)
    {
        pNode = createAsLineParameter(pPolyInfo);
    }

    if(!pNode.valid())  return NULL;

    if(m_ImgID.isValid())
    {
        osg::StateSet *pStateSet = pNode->getOrCreateStateSet();
        bindTexture(m_ImgID, pStateSet);
    }

    return pNode.release();
}


osg::Node *PrismDetail::createAsPointParameter(const PointCreationInfo *pPointInfo) const
{
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

    //旋转
    matrix.postMultRotate(qtRotation);

    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    //平移
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> pNode;

    const osg::Quat qtStand1(vecPlumbLine, osg::Vec3d(0.0, 0.0, -1.0));
    osg::ref_ptr<osg::Vec3Array> pCoordArray = new osg::Vec3Array;
    for(unsigned int i = 0; i < m_vecVertices.size(); i++)
    {
        const cmm::math::Point2d &vtx = m_vecVertices[i];

        osg::Vec3d vCoord;
        pEllipsoidModel->convertLatLongHeightToXYZ(vtx.y(), vtx.x(), vPoint.z(), vCoord.x(), vCoord.y(), vCoord.z());
        vCoord -= vecTrans;
        vCoord = qtStand1 * vCoord;
        pCoordArray->push_back(vCoord);
    }
    pCoordArray->push_back(pCoordArray->front());

    const osg::Vec4 vColor(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA);
    osgUtil::CommonModelCreater::CoverType eCoverType = osgUtil::CommonModelCreater::ALL;
    if(m_bTopVisible && !m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::TOP;
    }
    else if(!m_bTopVisible && m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::BOTTOM;
    }
    else if(!m_bTopVisible && !m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::NONE;
    }

    //没有挖洞
    if(m_vecHoles.empty())
    {
        pNode = osgUtil::CommonModelCreater::instance()->createPrism(pCoordArray, m_dblHeight, eCoverType, m_bImageStretched, vColor);
    }
    else
    {
        std::vector<std::vector<osg::Vec3> > vecTopVertices;
        std::vector<std::vector<osg::Vec3> > vecBottomVertices;

        for(unsigned int i = 0u; i < m_vecHoles.size(); i++)
        {
            IHole *pIHole = m_vecHoles[i].get();
            if(pIHole->isOnTopFace())
            {
                vecTopVertices.push_back(Hole2Vertex(pIHole, vPoint, m_dblHeight * 0.5, pPointInfo->m_dblAzimuthAngle));
            }
            else
            {
                vecBottomVertices.push_back(Hole2Vertex(pIHole, vPoint, m_dblHeight * 0.5, pPointInfo->m_dblAzimuthAngle));
            }
        }

        pNode = osgUtil::CommonModelCreater::instance()->createPrismWithHole(pCoordArray, m_dblHeight, eCoverType, vecTopVertices, vecBottomVertices, m_bImageStretched, vColor);
    }

    pMatrixTransform->setMatrix(matrix);
    pMatrixTransform->addChild(pNode);
    return pMatrixTransform.release();
}


osg::Node *PrismDetail::createAsLineParameter(const PolyCreationInfo *pPolyInfo) const
{
    const osg::Vec4 vColor(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::Matrixd matrix;
    osg::ref_ptr<osg::Node> pNode;

    osgUtil::CommonModelCreater::CoverType eCoverType = osgUtil::CommonModelCreater::ALL;
    if(m_bTopVisible && !m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::TOP;
    }
    else if(!m_bTopVisible && m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::BOTTOM;
    }
    else if(!m_bTopVisible && !m_bBottomVisible)
    {
        eCoverType = osgUtil::CommonModelCreater::NONE;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    const osg::Vec3d &pt1 = pPolyInfo->m_pPoints->at(pPolyInfo->m_nOffset);
    const osg::Vec3d &pt2 = pPolyInfo->m_pPoints->at(pPolyInfo->m_nOffset + 1u);

    osg::Vec3d ptTemp1, ptTemp2;
    pEllipsoidModel->convertLatLongHeightToXYZ(pt1.y(), pt1.x(), pt1.z(), ptTemp1.x(), ptTemp1.y(), ptTemp1.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(pt2.y(), pt2.x(), pt2.z(), ptTemp2.x(), ptTemp2.y(), ptTemp2.z());

    // 本截管道的中心点
    const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;

    //缩放
    osg::Vec3d vLineDir = ptTemp2 - ptTemp1;
    const double dblLength = vLineDir.normalize();
    matrix.postMultScale(osg::Vec3d(1.0, 1.0, dblLength));

    // 把管线旋转到正确的方向
    osg::Quat   qtRotation;

    osg::Vec3d vecCenterUp = ptCenter;
    vecCenterUp.normalize();


    // 将管道放倒
    osg::Quat   qt0;
    const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
    qt0.makeRotate(vecAxisZ, vLineDir);
    qtRotation *= qt0;

    osg::Vec3d vecAxisY(0.0, 1.0, 0.0);
    vecAxisY = qt0 * vecAxisY;

    const double dblCos = vLineDir * vecCenterUp;
    const double dblAngle = acos(dblCos);
    const osg::Quat   qt2(osg::PI_2 - dblAngle, vLineDir ^ vecCenterUp);

    const osg::Vec3d vecPipeUp = qt2 * vecCenterUp;

    osg::Quat   qt1;
    qt1.makeRotate(vecAxisY, vecPipeUp);
    qtRotation *= qt1;      // 顺着自己的轴向打滚，将它放平

    //旋转
    matrix.postMultRotate(qtRotation);

    //平移
    matrix.postMultTranslate(ptCenter);

    osg::ref_ptr<osg::Vec3Array> pCoordArray = new osg::Vec3Array;
    for(unsigned int i = 0; i < m_vecVertices.size(); i++)
    {
        const cmm::math::Point2d &vtx = m_vecVertices[i];

        const osg::Vec3d vCoord(vtx.x(), vtx.y(), 0.0);

        pCoordArray->push_back(vCoord);
    }

    pNode = osgUtil::CommonModelCreater::instance()->createPrism(pCoordArray.get(), 1.0f, eCoverType, m_bImageStretched, vColor);

    if(!m_bImageStretched)
    {
        osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
        osg::Matrix mtx = osg::Matrix::scale(osg::Vec3d(dblLength, 1.0, 1.0));
        pTexmat->setMatrix(mtx);
        pMatrixTransform->getOrCreateStateSet()->setTextureAttributeAndModes(0u, pTexmat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }

    if(m_ImgID.isValid())
    {
        pMatrixTransform->getOrCreateStateSet()->getOrCreateUniform("texture", osg::Uniform::SAMPLER_2D)->set(0);
        pMatrixTransform->getOrCreateStateSet()->getOrCreateUniform("btexture", osg::Uniform::BOOL)->set(true);
    }
    else
    {
        pMatrixTransform->getOrCreateStateSet()->getOrCreateUniform("btexture", osg::Uniform::BOOL)->set(false);
    }

    pMatrixTransform->setMatrix(matrix);
    pMatrixTransform->addChild(pNode);
    return pMatrixTransform.release();
}



}
