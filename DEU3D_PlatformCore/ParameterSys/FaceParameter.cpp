#include "FaceParameter.h"
#include <assert.h>
#include <osg/CoordinateSystemNode>

#include <osg/PagedLOD>
#include "Detail.h"

namespace param
{

FaceParameter::FaceParameter(const ID &id) : Parameter(id)
{
}


FaceParameter::~FaceParameter(void)
{
    m_vecCoordinates.clear();
}

void FaceParameter::addPart(unsigned nOffset, unsigned nCount)
{
    m_vecParts.push_back(std::make_pair(nOffset, nCount));
}

bool FaceParameter::getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const
{
    if(nIndex >= m_vecParts.size())
    {
        return false;
    }

    nOffset = m_vecParts[nIndex].first;
    nCount = m_vecParts[nIndex].second;

    return true;
}

unsigned int FaceParameter::getPartCount()const
{
    return m_vecParts.size();
}

bool FaceParameter::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Parameter::fromBson(bsonDoc))
    {
        return false;
    }

    {
        bson::bsonDoubleEle *pDoubleEle = NULL;
        bson::bsonArrayEle *pCoordinateEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Coordinate"));

        if(pCoordinateEle == NULL)
        {
            return false;
        }

        for(unsigned int i = 0; i < pCoordinateEle->ChildCount(); i++)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pCoordinateEle->GetElement(i));
            bson::bsonDocument &doc = pDocEle->GetDoc();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("x"));
            double dblX = pDoubleEle->DblValue();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("y"));
            double dblY = pDoubleEle->DblValue();

            pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("z"));
            double dblZ = pDoubleEle->DblValue();

            addCoordinate(cmm::math::Point3d(dblX, dblY, dblZ));
        }
    }

    {
        bson::bsonInt32Ele *pInt32Ele = NULL;
        bson::bsonArrayEle *pPartEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Part"));

        if(pPartEle == NULL)
        {
            return true;
        }
        for(unsigned int i = 0; i < pPartEle->ChildCount(); i++)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pPartEle->GetElement(i));
            bson::bsonDocument &doc = pDocEle->GetDoc();

            pInt32Ele = dynamic_cast<bson::bsonInt32Ele *>(doc.GetElement("Offset"));
            unsigned int nOffset = pInt32Ele->Int32Value();

            pInt32Ele = dynamic_cast<bson::bsonInt32Ele *>(doc.GetElement("Count"));
            unsigned int nCount = pInt32Ele->Int32Value();

            addPart(nOffset, nCount);
        }
    }

    return true;
}

bool FaceParameter::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Parameter::toBson(bsonDoc))
    {
        return false;
    }

    {
        bson::bsonArrayEle *pCoordinateEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Coordinate"));

        if(pCoordinateEle == NULL)
        {
            return false;
        }

        std::vector<cmm::math::Point3d>::const_iterator itor = m_vecCoordinates.begin();
        for(; itor != m_vecCoordinates.end(); ++itor)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pCoordinateEle->AddDocumentElement());
            if(pDocEle == NULL)
            {
                continue;
            }

            bson::bsonDocument &doc = pDocEle->GetDoc();

            if(!doc.AddDblElement("x", itor->x()) ||
                !doc.AddDblElement("y", itor->y()) ||
                !doc.AddDblElement("z", itor->z()))
            {
                return false;
            }
        }
    }

    if(!m_vecParts.empty())
    {
        bson::bsonArrayEle *pPartEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Part"));

        if(pPartEle == NULL)
        {
            return false;
        }

        std::vector<std::pair<unsigned, unsigned> >::const_iterator itor = m_vecParts.begin();
        for(; itor != m_vecParts.end(); ++itor)
        {
            bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pPartEle->AddDocumentElement());
            if(pDocEle == NULL)
            {
                continue;
            }

            bson::bsonDocument &doc = pDocEle->GetDoc();

            if(!doc.AddInt32Element("Offset", (*itor).first) ||
                !doc.AddInt32Element("Count", (*itor).second))
            {
                return false;
            }
        }
    }

    //添加属性数据
	bsonDoc.AddStringElement("ID", getID().toString().c_str());

	//包围球
    cmm::math::Sphered bound = getBoundingSphere();
	bson::bsonArrayEle *pBoundingSphere = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BoundingSphere"));
    pBoundingSphere->AddDblElement(bound.getCenter().x());
	pBoundingSphere->AddDblElement(bound.getCenter().y());
	pBoundingSphere->AddDblElement(bound.getCenter().z());
    pBoundingSphere->AddDblElement(bound.getRadius());

    //最大可视范围
    bsonDoc.AddDblElement("MaxRange", getMaxRange());

    return true;
}

void FaceParameter::addCoordinate(const cmm::math::Point3d &point)
{
    m_vecCoordinates.push_back(point);
}


bool FaceParameter::insertCoordinate(unsigned nIndex, const cmm::math::Point3d &point)
{
    if(nIndex > m_vecCoordinates.size())
    {
        assert(false);
        return false;
    }

    std::vector<cmm::math::Point3d>::const_iterator itorPos = m_vecCoordinates.begin();
    itorPos += nIndex;
    m_vecCoordinates.insert(itorPos, point);
    return true;
}


bool FaceParameter::setCoordinate(unsigned nIndex, const cmm::math::Point3d &point)
{
    if(nIndex >= m_vecCoordinates.size())
    {
        assert(false);
        return false;
    }

    m_vecCoordinates[nIndex] = point;

    return true;
}


bool FaceParameter::removeCoordinate(unsigned nIndex)
{
    if(nIndex >= m_vecCoordinates.size())
    {
        assert(false);
        return false;
    }

    std::vector<cmm::math::Point3d>::const_iterator itorPos = m_vecCoordinates.begin();
    itorPos += nIndex;
    m_vecCoordinates.erase(itorPos);
    return true;
}


unsigned FaceParameter::getCoordinateCount(void) const
{
    return m_vecCoordinates.size();
}


const cmm::math::Point3d &FaceParameter::getCoordinate(unsigned nIndex) const
{
    if(nIndex >= m_vecCoordinates.size())
    {
        assert(false);
        static const cmm::math::Point3d ptInvalid(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        return ptInvalid;
    }

    return m_vecCoordinates[nIndex];
}

void FaceParameter::setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates)
{
    m_vecCoordinates.assign(vecCoorinates.begin(), vecCoorinates.end());
}

const std::vector<cmm::math::Point3d> &FaceParameter::getCoordinates(void) const
{
    return m_vecCoordinates;
}

cmm::math::Sphered  FaceParameter::getBoundingSphere(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    cmm::math::Sphered s;
    for(size_t i = 0; i < m_vecCoordinates.size(); i++)
    {
        const cmm::math::Point3d &ptCoord = m_vecCoordinates[i];

        cmm::math::Point3d ptOnGlobe;
        pEllipsoidModel->convertLatLongHeightToXYZ(ptCoord.y(), ptCoord.x(), ptCoord.z() + m_dblHeight, ptOnGlobe.x(), ptOnGlobe.y(), ptOnGlobe.z());
        s.expandBy(ptOnGlobe);
    }

    cmm::math::Point3d ptCenter = s.getCenter();
    cmm::math::Point3d ptCoord;
    pEllipsoidModel->convertXYZToLatLongHeight(ptCenter.x(), ptCenter.y(), ptCenter.z(), ptCoord.y(), ptCoord.x(), ptCoord.z());
    s.setCenter(ptCoord);

    return s;
}

osg::Node *FaceParameter::createParameterNode(void) const
{
    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    osg::ref_ptr<osg::Vec3dArray> pCoodArray = new osg::Vec3dArray;
    for(unsigned int i = 0; i < m_vecCoordinates.size(); i++)
    {
        pCoodArray->push_back(osg::Vec3d(m_vecCoordinates[i].x(), m_vecCoordinates[i].y(), m_vecCoordinates[i].z() + m_dblHeight));
    }

    osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    for(unsigned int i = 0; i < m_vecCoordinates.size(); i++)
    {
        osg::Vec3d vTemp;
        pEllipsoidModel->convertLatLongHeightToXYZ(m_vecCoordinates[i].x(), m_vecCoordinates[i].y(), m_vecCoordinates[i].z() + m_dblHeight, vTemp[0], vTemp[1], vTemp[2]);
        pVertex->push_back(vTemp);
    }

    std::vector<ID> vecDetails;
    std::vector<std::pair<double, double> > vecRanges;
    if(!getSortedDetails(vecDetails, vecRanges))
    {
        return NULL;
    }

    for(unsigned int i = 0u; i < m_vecParts.size(); i++)
    {
        osg::ref_ptr<Detail::PolyCreationInfo> pCreateInfo = new Detail::PolyCreationInfo;
        pCreateInfo->m_pPoints = pCoodArray;
        pCreateInfo->m_nOffset = m_vecParts[i].first;
        pCreateInfo->m_nCount = m_vecParts[i].second;

        osg::BoundingSphered bs;
        for(unsigned int j = m_vecParts[i].first; j < m_vecParts[i].second; j++)
        {
            bs.expandBy((*pVertex)[j]);
        }

        osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;

        for(unsigned int j = 0; j < m_vecDetails.size(); j++)
        {
            pPagedLOD->setRange(j, m_vecVisibleRange[j].first, m_vecVisibleRange[j].second);
            pPagedLOD->setFileID(j, m_vecDetails[j]);
        }

        pPagedLOD->setCenter(bs.center());
        pPagedLOD->setRadius(bs.radius());

        pPagedLOD->setChildCreationInfo(pCreateInfo);

        pGroup->addChild(pPagedLOD);
    }

    return pGroup.release();
}

}

