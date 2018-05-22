#include "PolygonPipeConnectorDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osgUtil/CommonOSG.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/CullFace>

#include "PolygonPipeConnectorBuilder.h"

namespace param
{

PolygonPipeConnectorDetail::PolygonPipeConnectorDetail(void)
{
    m_strType = "normal";
}


PolygonPipeConnectorDetail::PolygonPipeConnectorDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_POLYGON_PIPE_CONNECTOR_ID)
{
    m_strType = "normal";
}


PolygonPipeConnectorDetail::~PolygonPipeConnectorDetail(void)
{

}


bool PolygonPipeConnectorDetail::addJoint(const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon)
{
    m_vecJoints.resize(m_vecJoints.size() + 1u);
    Joint &joint = m_vecJoints.back();
    joint.m_posTarget = posTarget;
    joint.m_dblLength = dblLength;
    joint.m_polygon = polygon;
    return true;
}


bool PolygonPipeConnectorDetail::setJoint(unsigned i, const cmm::math::Point3d &posTarget, double dblLength, const cmm::math::Polygon2 &polygon)
{
    if (i < m_vecJoints.size())
    {
        m_vecJoints[i].m_polygon = polygon;
        m_vecJoints[i].m_posTarget = posTarget;
        m_vecJoints[i].m_dblLength = dblLength;
        return true;
    }
    return false;
}


bool PolygonPipeConnectorDetail::getJoint(unsigned i, cmm::math::Point3d &posTarget, double &dblLength, cmm::math::Polygon2 &polygon) const
{
    if (i < m_vecJoints.size())
    {
        posTarget = m_vecJoints[i].m_posTarget;
        polygon   = m_vecJoints[i].m_polygon;
        dblLength = m_vecJoints[i].m_dblLength;
        return true;
    }

    return false;
}


double PolygonPipeConnectorDetail::getBoundingSphereRadius(void) const
{
    double r = 0.0;

    for(size_t i = 0; i < m_vecJoints.size(); i++)
    {
        const Joint &joint = m_vecJoints[i];
        const cmm::math::Box2d bb = joint.m_polygon.getBound();
        const double dblBoxRadius = sqrt(bb.height() * bb.height() + bb.width() * bb.width());
        if(dblBoxRadius > r) 
        {
            r = dblBoxRadius;
        }
        if(joint.m_dblLength > r)
        {
            r = joint.m_dblLength;
        }
    }

    return r;
}


bool PolygonPipeConnectorDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if (DynModelDetail::fromBson(bsonDoc) == false) return false;

    bson::bsonArrayEle* joints = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement("Joints"));
    if (joints == NULL || joints->GetType() != bson::bsonArrayType) return false;

    for(unsigned i = 0; i < joints->ChildCount(); i++)
    {
        bson::bsonDocumentEle * joint = dynamic_cast<bson::bsonDocumentEle*>(joints->GetElement(i));
        if (joint == NULL) continue;

        bson::bsonDocument &joint_doc = joint->GetDoc();
        bson::bsonDocumentEle *polygon = dynamic_cast<bson::bsonDocumentEle*>(joint_doc.GetElement("Polygon"));
        bson::bsonDoubleEle *length = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("Length"));
        bson::bsonDoubleEle *target_x = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetX"));
        bson::bsonDoubleEle *target_y = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetY"));
        bson::bsonDoubleEle *target_z = dynamic_cast<bson::bsonDoubleEle*>(joint_doc.GetElement("TargetZ"));

        if (!polygon || !length || !target_x || !target_y || !target_z) continue;

        cmm::math::Polygon2 vertices;
        const bson::bsonDocument &docPolygon = polygon->GetDoc();
        for(unsigned j = 0; j < docPolygon.ChildCount(); j++)
        {
            const bson::bsonArrayEle *pVertex = dynamic_cast<const bson::bsonArrayEle *>(docPolygon.GetElement(j));
            if(!pVertex)    continue;

            const bson::bsonDoubleEle *pElementX = dynamic_cast<const bson::bsonDoubleEle *>(pVertex->GetElement(0u));
            if(!pElementX)  continue;

            const bson::bsonDoubleEle *pElementY = dynamic_cast<const bson::bsonDoubleEle *>(pVertex->GetElement(1u));
            if(!pElementY)  continue;

            vertices.addVertex(cmm::math::Point2d(pElementX->DblValue(), pElementY->DblValue()));
        }

        addJoint(cmm::math::Point3d(target_x->DblValue(), target_y->DblValue(), target_z->DblValue()), length->DblValue(), vertices);
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


bool PolygonPipeConnectorDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if (DynModelDetail::toBson(bsonDoc) == false) return false;

    bson::bsonArrayEle *joints = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Joints"));

    for(size_t i = 0u; i < m_vecJoints.size(); i++)
    {
        bson::bsonDocumentEle *joints_doc_ele = dynamic_cast<bson::bsonDocumentEle*>(joints->AddDocumentElement());
        bson::bsonDocument    &joints_doc     = joints_doc_ele->GetDoc();

        const Joint &joint = m_vecJoints[i];
        bson::bsonDocumentEle *pPolygonEle = dynamic_cast<bson::bsonDocumentEle *>(joints_doc.AddDocumentElement("Polygon"));
        bson::bsonDocument &docPolygon = pPolygonEle->GetDoc();
        for(size_t j = 0u; j < joint.m_polygon.getVerticesCount(); j++)
        {
            const cmm::math::Point2d &vtx = joint.m_polygon.getSafeVertex(j);
            bson::bsonArrayEle *pVtxEle = dynamic_cast<bson::bsonArrayEle *>(docPolygon.AddArrayElement("vertex"));
            pVtxEle->AddDblElement(vtx.x());
            pVtxEle->AddDblElement(vtx.y());
        }

        if(!joints_doc.AddDblElement("Length", joint.m_dblLength)) continue;

        if (!joints_doc.AddDblElement("TargetX", joint.m_posTarget.x())) continue;

        if (!joints_doc.AddDblElement("TargetY", joint.m_posTarget.y())) continue;

        if (!joints_doc.AddDblElement("TargetZ", joint.m_posTarget.z())) continue;
    }

    bsonDoc.AddStringElement("PipeType", m_strType.c_str());

    return true;
}


//创建管接头
osg::Node *PolygonPipeConnectorDetail::createDetailNode(const CreationInfo *pInfo) const
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

    PolygonPipeConnectorBuilder pcb;
    for(size_t i = 0; i < m_vecJoints.size(); i++)
    {
        const Joint &joint = m_vecJoints[i];

        osg::Vec3d vecTargetDir;
        pEllipsoidModel->convertLatLongHeightToXYZ(joint.m_posTarget.y(), joint.m_posTarget.x(), joint.m_posTarget.z(), vecTargetDir.x(), vecTargetDir.y(), vecTargetDir.z());
        vecTargetDir  = qtReverse * vecTargetDir;
        vecTargetDir -= ptCenterPos;

        vecTargetDir.normalize();
        vecTargetDir *= joint.m_dblLength;

        pcb.addPort(vecTargetDir, joint.m_polygon);
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