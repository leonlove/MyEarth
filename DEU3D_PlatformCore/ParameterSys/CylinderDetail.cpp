#include "CylinderDetail.h"

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osgUtil/CommonOSG.h>
#include <IDProvider/Definer.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TexMat>

namespace param
{

CylinderDetail::CylinderDetail(void)
{
    m_dblRadius = 1.0;
    m_dblHeight = 1.0;
    m_bTopVisible = true;
    m_bBottomVisible = true;
}

CylinderDetail::CylinderDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_CYLINDER_ID)
{
    m_dblRadius = 1.0;
    m_dblHeight = 1.0;
    m_bTopVisible = true;
    m_bBottomVisible = true;
}

CylinderDetail::~CylinderDetail(void)
{

}
bool CylinderDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonBoolEle *pBoolEle = NULL;
    pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("TopVisible"));
    if(pBoolEle == NULL)
    {
        return false;
    }
    setTopVisible(pBoolEle->BoolValue());

    pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("BottomVisible"));
    if(pBoolEle == NULL)
    {
        return false;
    }
    setBottomVisible(pBoolEle->BoolValue());

    bson::bsonDoubleEle *pDoubleEle = NULL;

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Radius"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setRadius(pDoubleEle->DblValue());

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Height"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setHeight(pDoubleEle->DblValue());

    return true;
}

bool CylinderDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBoolElement("TopVisible", m_bTopVisible) ||
        !bsonDoc.AddBoolElement("BottomVisible", m_bBottomVisible) ||
        !bsonDoc.AddDblElement("Radius", m_dblRadius) ||
        !bsonDoc.AddDblElement("Height", m_dblHeight))
    {
        return false;
    }

    return true;
}

double CylinderDetail::getBoundingSphereRadius(void) const
{
    return std::max(m_dblRadius, m_dblHeight * 0.5);
}

//创建圆柱
osg::Node *CylinderDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    const PolyCreationInfo  *pPolyInfo  = dynamic_cast<const PolyCreationInfo *>(pInfo);
    if(pPointInfo == NULL && pPolyInfo == NULL)
    {
        return NULL;
    }

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

    //线参数
    if(pPolyInfo != NULL)
    {
        const osg::Vec3d &pt1 = pPolyInfo->m_pPoints->at(pPolyInfo->m_nOffset);
        const osg::Vec3d &pt2 = pPolyInfo->m_pPoints->at(pPolyInfo->m_nOffset + 1u);

        osg::Vec3d ptTemp1, ptTemp2;
        pEllipsoidModel->convertLatLongHeightToXYZ(pt1.y(), pt1.x(), pt1.z(), ptTemp1.x(), ptTemp1.y(), ptTemp1.z());
        pEllipsoidModel->convertLatLongHeightToXYZ(pt2.y(), pt2.x(), pt2.z(), ptTemp2.x(), ptTemp2.y(), ptTemp2.z());

        //管线的长度
        osg::Vec3d vecLen = ptTemp2 - ptTemp1;
        const double dblLength = vecLen.normalize();

        //缩放
        matrix.postMultScale(osg::Vec3d(m_dblRadius, m_dblRadius, dblLength));

        //旋转
        const osg::Quat qtRotation(osg::Vec3d(0.0, 0.0, 1.0), vecLen);
        matrix.postMultRotate(qtRotation);

        //平移
        const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;
        matrix.postMultTranslate(ptCenter);

        osgUtil::CommonModelCreater::instance()->createStandardModel(osgUtil::CommonModelCreater::CYLINDER, eCoverType, m_bImageStretched, vColor, pNode);

        if(!m_bImageStretched)
        {
            osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
            osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(dblLength, m_dblRadius, 1.0));
            pTexmat->setMatrix(mtx);
            pMatrixTransform->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexmat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
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
    }
    //点参数
    else
    {
        osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();
        //没有挖洞，用标准体创建
        if(m_vecHoles.empty())
        {
            osgUtil::CommonModelCreater::instance()->createStandardModel(osgUtil::CommonModelCreater::CYLINDER, eCoverType, m_bImageStretched, vColor, pNode);

            //缩放
            matrix.postMultScale(osg::Vec3d(m_dblRadius, m_dblRadius, m_dblHeight));

            if(!m_bImageStretched)
            {
                osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
                osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(m_dblHeight, m_dblRadius, 1.0));
                pTexmat->setMatrix(mtx);
                pMatrixTransform->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexmat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            }
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

            pNode = osgUtil::CommonModelCreater::instance()->createCylinderWithHole(m_dblRadius, m_dblHeight, eCoverType, vecTopVertices, vecBottomVertices, m_bImageStretched, vColor);
        }

        //旋转
        const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), pPointInfo->m_dblAzimuthAngle, pPointInfo->m_dblPitchAngle);
        matrix.postMultRotate(qtRotation);

        //平移
        osg::Vec3d vecTrans;
        pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
        matrix.postMultTranslate(vecTrans);
    }

    if(m_ImgID.isValid())
    {
        osg::StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();
        bindTexture(m_ImgID, pStateSet);
    }

    pMatrixTransform->setMatrix(matrix);
    pMatrixTransform->addChild(pNode.get());
    return pMatrixTransform.release();
}

}