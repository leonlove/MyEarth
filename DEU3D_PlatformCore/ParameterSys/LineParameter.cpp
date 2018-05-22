#include "LineParameter.h"

#include <IDProvider/Definer.h>

#include <assert.h>
#include <osg/CoordinateSystemNode>
#include <osg/PagedLOD>
#include <osg/TexMat>

#include "Detail.h"

namespace param
{

LineParameter::LineParameter(const ID &id) : 
    Parameter(id),
    m_bTerrainMagnet(false)
{
}

LineParameter::~LineParameter(void)
{
    m_vecCoordinates.clear();
	m_vecColors.clear();
}

void LineParameter::addPart(unsigned nOffset, unsigned nCount)
{
    m_vecParts.push_back(std::make_pair(nOffset, nCount));
}

bool LineParameter::getPart(unsigned nIndex, unsigned &nOffset, unsigned &nCount)const
{
    if(nIndex >= m_vecParts.size())
    {
        return false;
    }

    nOffset = m_vecParts[nIndex].first;
    nCount = m_vecParts[nIndex].second;

    return true;
}

unsigned int LineParameter::getPartCount()const
{
    return m_vecParts.size();
}

bool LineParameter::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Parameter::fromBson(bsonDoc))
    {
        return false;
    }

    {
        bson::bsonBoolEle *pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("TerrainMagnet"));
        if(pBoolEle == NULL)
        {
            return false;
        }
        setTerrainMagnet(pBoolEle->BoolValue());
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
        bson::bsonDoubleEle *pDoubleEle = NULL;
        bson::bsonArrayEle *pColorEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Color"));

        if(pColorEle != NULL)
        {
			for(unsigned int i = 0; i < pColorEle->ChildCount(); i++)
			{
				bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pColorEle->GetElement(i));
				bson::bsonDocument &doc = pDocEle->GetDoc();

				pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("r"));
				double dblR = pDoubleEle->DblValue();

				pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("g"));
				double dblG = pDoubleEle->DblValue();

				pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(doc.GetElement("b"));
				double dblB = pDoubleEle->DblValue();

				addColor(cmm::math::Point3d(dblR, dblG, dblB));
			}
        }
		//- end        
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

bool LineParameter::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Parameter::toBson(bsonDoc))
    {
        return false;
    }

    {
        if(!bsonDoc.AddBoolElement("TerrainMagnet", m_bTerrainMagnet))
        {
            return false;
        }
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

	{
		bson::bsonArrayEle *pColorEle = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("Color"));

		if(pColorEle == NULL)
		{
			return false;
		}

		std::vector<cmm::math::Point3d>::const_iterator itor = m_vecColors.begin();
		for(; itor != m_vecColors.end(); ++itor)
		{
			bson::bsonDocumentEle *pDocEle = dynamic_cast<bson::bsonDocumentEle *>(pColorEle->AddDocumentElement());
			if(pDocEle == NULL)
			{
				continue;
			}

			bson::bsonDocument &doc = pDocEle->GetDoc();

			if(!doc.AddDblElement("r", itor->x()) ||
				!doc.AddDblElement("g", itor->y()) ||
				!doc.AddDblElement("b", itor->z()))
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

void LineParameter::addCoordinate(const cmm::math::Point3d &point)
{
    m_vecCoordinates.push_back(point);
}

void LineParameter::addColor(const cmm::math::Vector3d &color)
{
	m_vecColors.push_back(color);
}

bool LineParameter::insertCoordinate(unsigned nIndex, const cmm::math::Point3d &point)
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

bool LineParameter::setCoordinate(unsigned nIndex, const cmm::math::Point3d &point)
{
    if(nIndex >= m_vecCoordinates.size())
    {
        assert(false);
        return false;
    }

    m_vecCoordinates[nIndex] = point;

    return true;
}

bool LineParameter::removeCoordinate(unsigned nIndex)
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

unsigned LineParameter::getCoordinateCount(void) const
{
    return m_vecCoordinates.size();
}

const cmm::math::Point3d &LineParameter::getCoordinate(unsigned nIndex) const
{
    if(nIndex >= m_vecCoordinates.size())
    {
        assert(false);
        static const cmm::math::Point3d ptInvalid(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        return ptInvalid;
    }

    return m_vecCoordinates[nIndex];
}


void LineParameter::setCoordinates(const std::vector<cmm::math::Point3d> &vecCoorinates)
{
    m_vecCoordinates.assign(vecCoorinates.begin(), vecCoorinates.end());
}


const std::vector<cmm::math::Point3d> &LineParameter::getCoordinates(void) const
{
    return m_vecCoordinates;
}


cmm::math::Sphered  LineParameter::getBoundingSphere(void) const
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


osg::Node *LineParameter::createParameterNode(void) const
{
    std::vector<ID> vecDetails;
    std::vector<std::pair<double, double> > vecRanges;
    if(!getSortedDetails(vecDetails, vecRanges))
    {
        return NULL;
    }

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
        pEllipsoidModel->convertLatLongHeightToXYZ(m_vecCoordinates[i].y(), m_vecCoordinates[i].x(), m_vecCoordinates[i].z() + m_dblHeight, vTemp[0], vTemp[1], vTemp[2]);
        pVertex->push_back(vTemp);
    }

	bool bPointCloudDetail = false;
	for(unsigned int i = 0; i < vecDetails.size(); i++)
	{
		if(vecDetails[i].ObjectID.m_nType == DETAIL_DYN_POINT_CLOUD_ID)
		{
			bPointCloudDetail = true;
			break;
		}
	}

    bool bOnlyLineDetail = true;
    for(unsigned int i = 0; i < vecDetails.size(); i++)
    {
        if(vecDetails[i].ObjectID.m_nType != DETAIL_DYN_LINE_ID)
        {
            bOnlyLineDetail = false;
            break;
        }
    }

    //只有线参数，不做拆分
    if(bOnlyLineDetail)
    {
        unsigned int nPartSize = m_vecParts.size();
        //没有分段
        if(nPartSize == 0)
        {
            osg::ref_ptr<Detail::PolyCreationInfo> pCreateInfo = new Detail::PolyCreationInfo;
            pCreateInfo->m_pPoints = pCoodArray;
            pCreateInfo->m_nOffset = 0;
            pCreateInfo->m_nCount = pCoodArray->size();

            osg::BoundingSphered bs;
            for(unsigned int i = 0; i < pVertex->size(); i++)
            {
                bs.expandBy((*pVertex)[i]);
            }

            osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
            for(unsigned int i = 0; i < vecDetails.size(); i++)
            {
                pPagedLOD->setRange(i, vecRanges[i].first, vecRanges[i].second);
                pPagedLOD->setFileID(i, vecDetails[i]);
            }

            pPagedLOD->setCenter(bs.center());
            pPagedLOD->setRadius(bs.radius());

            pPagedLOD->setChildCreationInfo(pCreateInfo);

            pGroup->addChild(pPagedLOD);
        }
        //有分段
        else
        {
            for(unsigned int i = 0; i < nPartSize; i++)
            {
                osg::ref_ptr<Detail::PolyCreationInfo> pCreateInfo = new Detail::PolyCreationInfo;
                pCreateInfo->m_pPoints = pCoodArray;
                pCreateInfo->m_nOffset = m_vecParts[i].first;
                pCreateInfo->m_nCount = m_vecParts[i].second;

                osg::BoundingSphered bs;
                unsigned int nEndPos = m_vecParts[i].first + m_vecParts[i].second;
                for(unsigned int j = m_vecParts[i].first; j < nEndPos; j++)
                {
                    bs.expandBy((*pVertex)[j]);
                }

                osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
                for(unsigned int j = 0; j < vecDetails.size(); j++)
                {
                    pPagedLOD->setRange(j, vecRanges[j].first, vecRanges[j].second);
                    pPagedLOD->setFileID(j, vecDetails[j]);
                }

                pPagedLOD->setCenter(bs.center());
                pPagedLOD->setRadius(bs.radius());

                pPagedLOD->setChildCreationInfo(pCreateInfo);

                pGroup->addChild(pPagedLOD);
            }
        }

        return pGroup.release();
    }
    //不止线参数，每两个点做拆分
    else
    {
		if (bPointCloudDetail)
		{
			osg::ref_ptr<osg::Vec3dArray> pColorArray = new osg::Vec3dArray;
			for(unsigned int i = 0; i < m_vecColors.size(); i++)
			{
				pColorArray->push_back(osg::Vec3d(m_vecColors[i].x(), m_vecColors[i].y(), m_vecColors[i].z()));
			}

			osg::ref_ptr<Detail::PointCloudCreationInfo> pCreateInfo = new Detail::PointCloudCreationInfo;
			pCreateInfo->m_pPoints = pCoodArray;
			pCreateInfo->m_pColors = pColorArray;

			osg::BoundingSphered bs;
			for(unsigned int i = 0; i < pVertex->size(); i++)
			{
				bs.expandBy((*pVertex)[i]);
			}

			osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
			for(unsigned int i = 0; i < vecDetails.size(); i++)
			{
				pPagedLOD->setRange(i, vecRanges[i].first, vecRanges[i].second);
				pPagedLOD->setFileID(i, vecDetails[i]);
			}

			pPagedLOD->setCenter(bs.center());
			pPagedLOD->setRadius(bs.radius());
			pPagedLOD->setChildCreationInfo(pCreateInfo);

			pGroup->addChild(pPagedLOD);
		}
		else
		{
			unsigned int nPartSize = m_vecParts.size();
			if(nPartSize == 0)
			{
				for(unsigned int i = 0; i < pVertex->size() - 1; i++)
				{
					osg::Vec3 vLen = (*pVertex)[i + 1] - (*pVertex)[i];
					double dblLen = vLen.length();
					if(cmm::math::floatEqual(dblLen, 0.0))
					{
						continue;
					}

					osg::ref_ptr<Detail::PolyCreationInfo> pCreateInfo = new Detail::PolyCreationInfo;
					pCreateInfo->m_pPoints = pCoodArray;
					pCreateInfo->m_nOffset = i;
					pCreateInfo->m_nCount = 2;

					osg::BoundingSphered bs;
					bs.expandBy((*pVertex)[i]);
					bs.expandBy((*pVertex)[i + 1]);

					osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
					{
						osg::ref_ptr<osg::TexMat> pTexmat = new osg::TexMat;
						osg::Matrix mtx = osg::Matrix::scale(osg::Vec3(dblLen, 1.0f, 1.0f));
						pTexmat->setMatrix(mtx);
						pPagedLOD->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexmat.get(), osg::StateAttribute::OFF);
					}

					for(unsigned int i = 0; i < vecDetails.size(); i++)
					{
						pPagedLOD->setRange(i, vecRanges[i].first, vecRanges[i].second);
						pPagedLOD->setFileID(i, vecDetails[i]);
					}

					pPagedLOD->setCenter(bs.center());
					pPagedLOD->setRadius(bs.radius());

					pPagedLOD->setChildCreationInfo(pCreateInfo);

					pGroup->addChild(pPagedLOD);
				}
			}
			else
			{
				for(unsigned int i = 0; i < nPartSize; i++)
				{
					for(unsigned int j = m_vecParts[i].first; j < m_vecParts[i].second - 1; j++)
					{
						osg::ref_ptr<Detail::PolyCreationInfo> pCreateInfo = new Detail::PolyCreationInfo;
						pCreateInfo->m_pPoints = pCoodArray;
						pCreateInfo->m_nOffset = j;
						pCreateInfo->m_nCount = 2;

						if((*pVertex)[j] == (*pVertex)[j + 1])
						{
							continue;
						}

						osg::BoundingSphered bs;
						bs.expandBy((*pVertex)[j]);
						bs.expandBy((*pVertex)[j + 1]);

						osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
						for(unsigned int k = 0; k < vecDetails.size(); k++)
						{
							pPagedLOD->setRange(k, vecRanges[k].first, vecRanges[k].second);
							pPagedLOD->setFileID(k, vecDetails[k]);
						}

						pPagedLOD->setCenter(bs.center());
						pPagedLOD->setRadius(bs.radius());

						pPagedLOD->setChildCreationInfo(pCreateInfo);

						pGroup->addChild(pPagedLOD);
					}
				}
			}
			//- else end
		}
        
        return pGroup.release();
    }

    return NULL;
}

}

