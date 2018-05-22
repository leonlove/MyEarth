#include "StaticModelDetail.h"

#include <IDProvider/Definer.h>

#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osgDB/ReadFile>
#include <osgUtil/CommonOSG.h>

namespace param
{

StaticModelDetail::StaticModelDetail(void)
{
    m_ModelID.setInvalid();
    m_bOnGlobe = false;
}


StaticModelDetail::StaticModelDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_STATIC_MODEL_ID))
{
    m_ModelID.setInvalid();
    m_bOnGlobe = false;
}


StaticModelDetail::~StaticModelDetail(void)
{

}


bool StaticModelDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Detail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonElement *pEle = bsonDoc.GetElement("ModelID");
    if(pEle == NULL)
    {
        return false;
    }

    bson::bsonElementType eType = pEle->GetType();
    if(eType == bson::bsonStringType)
    {
        bson::bsonStringEle *pStringEle = dynamic_cast<bson::bsonStringEle *>(pEle);
        m_ModelID = ID::genIDfromString(pStringEle->StrValue());
    }
    else if(eType == bson::bsonBinType)
    {
        bson::bsonBinaryEle *pBinEle = dynamic_cast<bson::bsonBinaryEle *>(pEle);
        m_ModelID = ID::genIDfromBinary(pBinEle->BinData(), pBinEle->BinDataLen());
    }
    else
    {
        return false;
    }

    pEle = bsonDoc.GetElement("IsOnGlobe");
    if (pEle)
    {
        m_bOnGlobe = pEle->BoolValue();
    }

    return true;
}


bool StaticModelDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("ModelID", (void *)&m_ModelID, sizeof(ID)))
    {
        return false;
    }

    if(!bsonDoc.AddBoolElement("IsOnGlobe", m_bOnGlobe))
    {
        return false;
    }

    return true;
}


//创建静态模型
osg::Node *StaticModelDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    osg::Matrixd matrix;

    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();
    if(!m_bOnGlobe)
    {
		//- V1.0
		if (pPointInfo->m_dblRollAngle > 10504.0)
		{
			matrix.postMultScale(pPointInfo->m_Scale);

			const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), pPointInfo->m_dblAzimuthAngle, pPointInfo->m_dblPitchAngle);
			matrix.postMultRotate(qtRotation);
		}
		//- V2.0
		else
		{
			osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
			const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(vPoint.y(), vPoint.x());
			const osg::Quat  qtStand(osg::Vec3d(0.0, 0.0, -1.0), vecPlumbLine);

			const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(vPoint.y(), vPoint.x());
			const osg::Quat  qtForward(osg::Vec3d(1.0, 0.0, 0.0), vecEastern);

			const osg::Quat qtRotation1 = qtForward * qtStand;
			const osg::Quat qtRotation2(pPointInfo->m_dblPitchAngle, pPointInfo->m_dblRollAngle, pPointInfo->m_dblAzimuthAngle);

			matrix.postMultRotate(qtRotation1);
			matrix.preMultRotate(qtRotation2);
			matrix.preMultScale(pPointInfo->m_Scale);
		}
    }
	else
	{
		matrix.postMultScale(pPointInfo->m_Scale);
	}

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);
    pMatrixTransform->addChild(osgDB::readNodeFile(m_ModelID));
    pMatrixTransform->setID(m_id);

    return pMatrixTransform.release();
}

}
