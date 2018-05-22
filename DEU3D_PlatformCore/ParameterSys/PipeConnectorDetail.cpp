#include "PipeConnectorDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/CommonOSG.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/CullFace>

#include "PipeConnectorBuilder.h"

namespace param
{

PipeConnectorDetail::PipeConnectorDetail(void)
{
    m_strType = "normal";
}


PipeConnectorDetail::PipeConnectorDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_PIPE_CONNECTOR_ID)
{
    m_strType = "normal";
}


PipeConnectorDetail::~PipeConnectorDetail(void)
{

}


bool PipeConnectorDetail::addJoint(const cmm::math::Point3d &posTarget, double dblLength, double dblRadius)
{
    const Joint  joint = {dblLength, dblRadius, posTarget};
    m_vecJoints.push_back(joint);
    return true;
}


bool PipeConnectorDetail::setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, double dblRadius)
{
    if (i < m_vecJoints.size())
    {
        m_vecJoints[i].m_dblRadius = dblRadius;
        m_vecJoints[i].m_posTarget = posTarget;
        m_vecJoints[i].m_dblLength = dblLength;
        return true;
    }
    return false;
}


bool PipeConnectorDetail::getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, double &dblRadius) const
{
    if (i < m_vecJoints.size())
    {
        posTarget = m_vecJoints[i].m_posTarget;
        dblRadius = m_vecJoints[i].m_dblRadius;
        dblLength = m_vecJoints[i].m_dblLength;
        return true;
    }

    return false;
}


double PipeConnectorDetail::getBoundingSphereRadius(void) const
{
    double r = 0.0;

    for(size_t i = 0; i < m_vecJoints.size(); i++)
    {
        const Joint &joint = m_vecJoints[i];
        if(joint.m_dblRadius > r) 
        {
            r = m_vecJoints[i].m_dblRadius;
        }
        if(joint.m_dblLength > r)
        {
            r = joint.m_dblLength;
        }
    }

    return r;
}


bool PipeConnectorDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if (DynModelDetail::fromBson(bsonDoc) == false) return false;

    bson::bsonArrayEle* joints = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement("Joints"));
    if (joints == NULL || joints->GetType() != bson::bsonArrayType) return false;

    for(unsigned i = 0; i < joints->ChildCount(); i++)
    {
        bson::bsonDocumentEle * joint = dynamic_cast<bson::bsonDocumentEle*>(joints->GetElement(i));
        if (joint == NULL) continue;

        bson::bsonDocument &joint_doc = joint->GetDoc();
        bson::bsonDoubleEle *radius = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("Radius"));
        bson::bsonDoubleEle *length = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("Length"));
        bson::bsonDoubleEle *target_x = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetX"));
        bson::bsonDoubleEle *target_y = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetY"));
        bson::bsonDoubleEle *target_z = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetZ"));

        if (!radius || !length || !target_x || !target_y || !target_z) continue;

        addJoint(cmm::math::Point3d(target_x->DblValue(), target_y->DblValue(), target_z->DblValue()), length->DblValue(), radius->DblValue());
    }

    bson::bsonStringEle * type = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("PipeType"));

    if (type)
    {
        m_strType = type->StrValue();
        if (m_strType != "normal" && m_strType != "endblock" && m_strType != "pipehat" && m_strType != "weld")
        {
            return false;
        }
    }

    return true;
}


bool PipeConnectorDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if (DynModelDetail::toBson(bsonDoc) == false) return false;

    bson::bsonArrayEle *joints = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Joints"));

    for(size_t i = 0; i < m_vecJoints.size(); i++)
    {
        bson::bsonDocumentEle *joints_doc_ele = dynamic_cast<bson::bsonDocumentEle*>(joints->AddDocumentElement());
        bson::bsonDocument    &joints_doc      = joints_doc_ele->GetDoc();

        if (!joints_doc.AddDblElement("Radius", m_vecJoints[i].m_dblRadius)) continue;

        if(!joints_doc.AddDblElement("Length", m_vecJoints[i].m_dblLength)) continue;

        if (!joints_doc.AddDblElement("TargetX", m_vecJoints[i].m_posTarget.x())) continue;

        if (!joints_doc.AddDblElement("TargetY", m_vecJoints[i].m_posTarget.y())) continue;

        if (!joints_doc.AddDblElement("TargetZ", m_vecJoints[i].m_posTarget.z())) continue;
    }

    bsonDoc.AddStringElement("PipeType", m_strType.c_str());

    return true;
}


//创建管接头
osg::Node *PipeConnectorDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    if(m_vecJoints.empty())
    {
        return NULL;
    }

    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::Matrixd matrix;
    const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(vPoint.x(), vPoint.y()), 0.0, 0.0);
    matrix.postMultRotate(qtRotation);

    osg::Vec3d vecTrans;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());
    matrix.postMultTranslate(vecTrans);

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);

    const osg::Quat qtReverse = qtRotation.inverse();
    const osg::Vec3d ptCenterPos = qtReverse * vecTrans;

    PipeConnectorBuilder pcb;
    for(size_t i = 0; i < m_vecJoints.size(); i++)
    {
        cmm::math::Point3d posTarget;
        double dblLength, dblRadius;
        getJoint(i, posTarget, dblLength, dblRadius);

        osg::Vec3d vecTargetDir;
        pEllipsoidModel->convertLatLongHeightToXYZ(posTarget.y(), posTarget.x(), posTarget.z(), vecTargetDir.x(), vecTargetDir.y(), vecTargetDir.z());
        vecTargetDir  = qtReverse * vecTargetDir;
        vecTargetDir -= ptCenterPos;

        vecTargetDir.normalize();
        vecTargetDir *= dblLength;

        pcb.addJoint(vecTargetDir, dblRadius);
    }
    pcb.setCorner(osg::Vec3d(0.0, 0.0, 0.0));
    pcb.setType(m_strType);
    pcb.setColor(osg::Vec4(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA));

    osg::ref_ptr<osg::Node> pConnectorNode = pcb.buildPipeConnector();
    if(!pConnectorNode.valid())
    {
        return NULL;
    }

    osg::StateSet *pStateSet = pConnectorNode->getOrCreateStateSet();
    if(m_Color.m_fltA < 0.98)
    {
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    else
    {
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);
    }

    pMatrixTransform->addChild(pConnectorNode.get());

    return pMatrixTransform.release();
}

}