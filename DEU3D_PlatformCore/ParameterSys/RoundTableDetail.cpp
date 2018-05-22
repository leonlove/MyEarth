#include "RoundTableDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/CommonOSG.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

namespace param
{

RoundTableDetail::RoundTableDetail()
{
}

RoundTableDetail::RoundTableDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_ROUND_TABLE_ID)
{
    m_dblTopRadius = 1.0;
    m_dblBottomRadius = 1.0;
    m_dblHeight =1.0;
    m_bTopVisible = true;
    m_bBottomVisible = true;
}


RoundTableDetail::~RoundTableDetail(void)
{

}


bool RoundTableDetail::fromBson(bson::bsonDocument &bsonDoc)
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

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("RadiusTop"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setRadiusTop(pDoubleEle->DblValue());

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("RadiusBottom"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setRadiusBottom(pDoubleEle->DblValue());

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Height"));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    setHeight(pDoubleEle->DblValue());

    return true;
}


bool RoundTableDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBoolElement("TopVisible", m_bTopVisible) ||
        !bsonDoc.AddBoolElement("BottomVisible", m_bBottomVisible) ||
        !bsonDoc.AddDblElement("RadiusTop", m_dblTopRadius) ||
        !bsonDoc.AddDblElement("RadiusBottom", m_dblBottomRadius) ||
        !bsonDoc.AddDblElement("Height", m_dblHeight))
    {
        return false;
    }

    return true;
}


double RoundTableDetail::getBoundingSphereRadius(void) const
{
    double max = (m_dblBottomRadius > m_dblTopRadius ? m_dblBottomRadius : m_dblTopRadius);
    max = (max > m_dblHeight ? max : m_dblHeight);

    return max;
}


osg::Node *RoundTableDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    osg::Matrixd matrix;
    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();
    const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), pPointInfo->m_dblAzimuthAngle, pPointInfo->m_dblPitchAngle);
    matrix.postMultRotate(qtRotation);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> pNode;

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

    //Ã»ÓÐÍÚ¶´
    if(m_vecHoles.empty())
    {
        pNode = osgUtil::CommonModelCreater::instance()->createRoundTable(m_dblTopRadius, m_dblBottomRadius, m_dblHeight, eCoverType, m_bImageStretched, vColor);
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

        pNode = osgUtil::CommonModelCreater::instance()->createRoundTableWithHole(m_dblTopRadius, m_dblBottomRadius, m_dblHeight, eCoverType, vecTopVertices, vecBottomVertices, m_bImageStretched, vColor);
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