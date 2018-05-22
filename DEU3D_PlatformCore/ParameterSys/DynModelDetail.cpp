#include "DynModelDetail.h"

#include "Hole.h"
#include <osg/CoordinateSystemNode>

namespace param
{

DynModelDetail::DynModelDetail()
{
    m_Color.m_fltR = 1.0;
    m_Color.m_fltG = 1.0;
    m_Color.m_fltB = 1.0;
    m_Color.m_fltA = 1.0; 
}

DynModelDetail::DynModelDetail(unsigned int nDataSetCode, DeuObjectIDType type) : Detail(ID(nDataSetCode, type))
{
    m_Color.m_fltR = 1.0;
    m_Color.m_fltG = 1.0;
    m_Color.m_fltB = 1.0;
    m_Color.m_fltA = 1.0;
}

DynModelDetail::~DynModelDetail(void)
{

};

bool DynModelDetail::fromBson(bson::bsonDocument &bsonDoc)
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
        m_ImgID = ID::genIDfromString(pStringEle->StrValue());
    }
    else if(eType == bson::bsonBinType)
    {
        bson::bsonBinaryEle *pBinEle = dynamic_cast<bson::bsonBinaryEle *>(pEle);
        m_ImgID = ID::genIDfromBinary(pBinEle->BinData(), pBinEle->BinDataLen());
    }
    else
    {
        return false;
    }

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Color"));
    if(pArrayEle == NULL || pArrayEle->ChildCount() != 4u)
    {
        return false;
    }

    bson::bsonDoubleEle *pDoubleEle = NULL;

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(0u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_Color.m_fltR = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(1u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_Color.m_fltG = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(2u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_Color.m_fltB = pDoubleEle->DblValue();

    pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(pArrayEle->GetElement(3u));
    if(pDoubleEle == NULL)
    {
        return false;
    }
    m_Color.m_fltA = pDoubleEle->DblValue();

    bson::bsonArrayEle *pHoles = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Holes"));
    if (pHoles == NULL)
    {
        return true;
    }

    for(size_t i = 0; i < pHoles->ChildCount(); i++)
    {
        bson::bsonDocumentEle *Hole = dynamic_cast<bson::bsonDocumentEle*>(pHoles->GetElement(i));
        IHole *pHole = generateHole();

        if (!pHole->fromBson(Hole->GetDoc()))
        {
            return false;
        }
        m_vecHoles[i] = pHole;
    }

    return true;
}

bool DynModelDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("ImageID", (void *)&m_ImgID, sizeof(ID)))
    {
        return false;
    }

    bson::bsonArrayEle *pArrayEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Color"));
    if(pArrayEle == NULL)
    {
        return false;
    }

    if(!pArrayEle->AddDblElement(m_Color.m_fltR) ||
        !pArrayEle->AddDblElement(m_Color.m_fltG) ||
        !pArrayEle->AddDblElement(m_Color.m_fltB) ||
        !pArrayEle->AddDblElement(m_Color.m_fltA))
    {
        return false;
    }

    bson::bsonArrayEle *pHoles = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Holes"));
    for(size_t i = 0; i < m_vecHoles.size(); i++)
    {
        bson::bsonDocumentEle *doc = dynamic_cast<bson::bsonDocumentEle*>(pHoles->AddDocumentElement());
        m_vecHoles[i]->toBson(doc->GetDoc());
    }

    return true;
}


IHole *DynModelDetail::generateHole()
{
    IHole *pHole = new Hole();
    m_vecHoles.push_back(pHole);
    return pHole;
}


void DynModelDetail::getHoleList(std::vector<IHole *> &holeList) const
{
    for(unsigned int i = 0; i < m_vecHoles.size(); i++)
    {
        holeList.push_back(m_vecHoles[i].get());
    }
}

std::vector<osg::Vec3> DynModelDetail::Hole2Vertex(const IHole *pHole, const osg::Vec3d &vPoint, double dblOffset, double dblAzimuthAngle) const
{
    const std::string &strHoleType = pHole->getHoleType();

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(vPoint.y(), vPoint.x());
    const osg::Quat qtStand(vecPlumbLine, osg::Vec3d(0.0, 0.0, -1.0));

    osg::Vec3d vecTrans;
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    osg::Vec3d vHolePoint;

    std::vector<osg::Vec3> vecVertices;

    if(0 == strHoleType.compare(HOLETYPE_CIRCLE))
    {
        cmm::math::Point2d ptHoleCenter;
        double dblHoleRadius;
        pHole->getCircle(ptHoleCenter, dblHoleRadius);

        pEllipsoidModel->convertLatLongHeightToXYZ(ptHoleCenter._v[1], ptHoleCenter._v[0], vPoint._v[2] + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);
        vHolePoint -= vecTrans;
        vHolePoint = qtStand * vHolePoint;

        genCircleVertices(vHolePoint, dblHoleRadius, vecVertices);
    }
    else if(0 == strHoleType.compare(HOLETYPE_RECTANGLE))
    {
        cmm::math::Point2d ptHoleCenter;
        double dblWidth, dblHeight, dblHoleAzimuthAngle;
        pHole->getRectangle(ptHoleCenter, dblWidth, dblHeight, dblHoleAzimuthAngle);

        double dblDeltaAngle = dblHoleAzimuthAngle - dblAzimuthAngle;
        osg::Quat qt;
        qt.makeRotate(dblDeltaAngle, osg::Vec3d(0.0, 0.0, 1.0));

        pEllipsoidModel->convertLatLongHeightToXYZ(ptHoleCenter._v[1], ptHoleCenter._v[0], vPoint._v[2] + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);
        vHolePoint -= vecTrans;
        vHolePoint = qtStand * vHolePoint;

        osg::Vec3d vCorner;

        //вСио╫г
        vCorner.set(vHolePoint[0] - dblWidth * 0.5, vHolePoint[1] + dblHeight * 0.5, vHolePoint[2]);
        vecVertices.push_back(qt * vCorner);
        //срио╫г
        vCorner.set(vHolePoint[0] + dblWidth * 0.5, vHolePoint[1] + dblHeight * 0.5, vHolePoint[2]);
        vecVertices.push_back(qt * vCorner);
        //сроб╫г
        vCorner.set(vHolePoint[0] + dblWidth * 0.5, vHolePoint[1] - dblHeight * 0.5, vHolePoint[2]);
        vecVertices.push_back(qt * vCorner);
        //вСоб╫г
        vCorner.set(vHolePoint[0] - dblWidth * 0.5, vHolePoint[1] - dblHeight * 0.5, vHolePoint[2]);
        vecVertices.push_back(qt * vCorner);

        return vecVertices;
    }
    else if(0 == strHoleType.compare(HOLETYPE_POLYGON))
    {
        std::vector<cmm::math::Point2d> vecVertex;
        pHole->getAllPolygonVertex(vecVertex);

        unsigned int nCount = vecVertex.size();
        for(unsigned int i = 0; i < nCount; i++)
        {
            pEllipsoidModel->convertLatLongHeightToXYZ(vecVertex[i]._v[1], vecVertex[i]._v[0], vPoint._v[2] + dblOffset, vHolePoint[0], vHolePoint[1], vHolePoint[2]);
            vHolePoint -= vecTrans;
            vHolePoint = qtStand * vHolePoint;
            vecVertices.push_back(vHolePoint);
        }
    }

    return vecVertices;
}

void DynModelDetail::genCircleVertices(const osg::Vec3d &ptCenter, double dblRadius, std::vector<osg::Vec3> &vecVertices) const
{
    const unsigned nEdgeCount = 40u;
    const double dblDeltaTheta = osg::PI * 2.0 / (double)nEdgeCount;
    double dblTheta = 0.0;
    for(unsigned int i = 0u; i < nEdgeCount; i++, dblTheta += dblDeltaTheta)
    {
        vecVertices.push_back(osg::Vec3d(ptCenter.x() + cos(dblTheta) * dblRadius, ptCenter.y() + sin(dblTheta) * dblRadius, ptCenter.z()));
    }
}

}