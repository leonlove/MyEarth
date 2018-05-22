#include "CubeDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/CommonOSG.h>
#include <osg/CoordinateSystemNode>
#include <osg/MatrixTransform>
#include <osg/TexMat>

namespace param
{

CubeDetail::CubeDetail()
{
    m_dblLength = 1.0;
    m_dblWidth = 1.0;
    m_dblHeight = 1.0;
    m_bTopVisible = true;
    m_bBottomVisible = true;
}

CubeDetail::CubeDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_CUBE_ID)
{
    m_dblLength = 1.0;
    m_dblWidth = 1.0;
    m_dblHeight = 1.0;
    m_bTopVisible = true;
    m_bBottomVisible = true;
}

CubeDetail::~CubeDetail(void)
{

}

bool CubeDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Length"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    double dblLength = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Width"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    double dblWidth = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Height"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    double dblHeight = pDoubleEle->DblValue();

    setCubeSize(dblLength, dblWidth, dblHeight);

    return true;
}

bool CubeDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddDblElement("Length", m_dblLength) ||
        !bsonDoc.AddDblElement("Width", m_dblWidth) ||
        !bsonDoc.AddDblElement("Height", m_dblHeight))
    {
        return false;
    }

    return true;
}

double CubeDetail::getBoundingSphereRadius(void) const
{
    const double radius = sqrt(m_dblLength * m_dblLength + m_dblWidth * m_dblWidth + m_dblHeight * m_dblHeight);
    return radius;
}

//创建长方体
osg::Node *CubeDetail::createDetailNode(const CreationInfo *pInfo) const
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

        // 本截管道的中心点
        const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;

        //缩放
        const double dblLength = (ptTemp2 - ptTemp1).length();
        matrix.postMultScale(osg::Vec3(m_dblWidth, m_dblHeight, dblLength));

        // 把管线旋转到正确的方向
        osg::Quat   qtRotation;

        osg::Vec3d vecCenterUp = ptCenter;
        vecCenterUp.normalize();

        osg::Vec3d vLineDir = ptTemp2 - ptTemp1;
        vLineDir.normalize();

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

        osgUtil::CommonModelCreater::instance()->createStandardModel(osgUtil::CommonModelCreater::CUBE, eCoverType, m_bImageStretched, vColor, pNode);

        if(!m_bImageStretched)
        {
            osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
            osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(dblLength, m_dblWidth, 1.0));
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
        const osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();
        //没有挖洞，用标准体创建
        if(m_vecHoles.empty())
        {
            osgUtil::CommonModelCreater::instance()->createStandardModel(osgUtil::CommonModelCreater::CUBE, eCoverType, m_bImageStretched, vColor, pNode);

            matrix.postMultScale(osg::Vec3d(m_dblWidth, m_dblLength, m_dblHeight));

            if(!m_bImageStretched)
            {
                osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
                osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(m_dblHeight, m_dblWidth, 1.0));
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

            pNode = osgUtil::CommonModelCreater::instance()->createCubeWithHole(m_dblLength, m_dblWidth, m_dblHeight, eCoverType, vecTopVertices, vecBottomVertices, m_bImageStretched, vColor);
        }

        const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), pPointInfo->m_dblAzimuthAngle, pPointInfo->m_dblPitchAngle);
        matrix.postMultRotate(qtRotation);

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
    pMatrixTransform->addChild(pNode);
    return pMatrixTransform.release();
}

}
