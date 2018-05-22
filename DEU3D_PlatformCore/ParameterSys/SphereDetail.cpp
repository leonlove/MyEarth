#include "SphereDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/TexMat>

namespace param
{

SphereDetail::SphereDetail(void)
{
    m_dblRadius = 1.0;
}


SphereDetail::SphereDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_SPHERE_ID)
{
    m_dblRadius = 1.0;
}


SphereDetail::~SphereDetail(void)
{

}


bool SphereDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Radius"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    const double dblRadius = pDoubleEle->DblValue();
    setRadius(dblRadius);

    return true;
}


bool SphereDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Radius", m_dblRadius))
    {
        return false;
    }

    return true;
}


double SphereDetail::getBoundingSphereRadius(void) const
{
    return m_dblRadius;
}


//´´½¨Çò
osg::Node *SphereDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    const osg::Vec4 vColor(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA);
    osg::ref_ptr<osg::Node> pNode;
    osgUtil::CommonModelCreater::instance()->createStandardModel(osgUtil::CommonModelCreater::SPHERE, osgUtil::CommonModelCreater::NONE, m_bImageStretched, vColor, pNode);

    osg::Matrixd matrix;
    matrix.postMultScale(osg::Vec3(m_dblRadius, m_dblRadius, m_dblRadius));

    const osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    pMatrixTransform->setMatrix(matrix);

    if(!m_bImageStretched)
    {
        osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
        osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(m_dblRadius, m_dblRadius, 1.0));
        pTexmat->setMatrix(mtx);
        pMatrixTransform->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexmat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }

    pMatrixTransform->addChild(pNode.get());

    if(m_ImgID.isValid())
    {
        osg::StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();
        bindTexture(m_ImgID, pStateSet);
    }

    return pMatrixTransform.release();
}

}