#include "Parameter.h"
#include "PointParameter.h"
#include "LineParameter.h"
#include "FaceParameter.h"

#include <algorithm>
#include <IDProvider/Definer.h>
#include <Common/memPool.h>

namespace param
{

IParameter *createParameter(const ID &id)
{
    if(id.ObjectID.m_nType == PARAM_POINT_ID)
    {
        OpenSP::sp<PointParameter> pPointParameter = new PointParameter(id);
        return pPointParameter.release();
    }
    else if(id.ObjectID.m_nType == PARAM_LINE_ID)
    {
        OpenSP::sp<LineParameter> pLineParameter = new LineParameter(id);
        return pLineParameter.release();
    }
    else if(id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        OpenSP::sp<FaceParameter> pFaceParameter = new FaceParameter(id);
        return pFaceParameter.release();
    }

    return NULL;
}


Parameter::Parameter(const ID &id) :
    m_ID(id),
    m_bFollowByTerrain(false),
    m_nCoverOrder(0),
    m_dblHeight(0.0),
    m_nActProperty(-1)
{
}

Parameter::~Parameter(void)
{
}

void Parameter::addDetail(const ID &id, double dMinRange, double dMaxRange)
{
    m_vecDetails.push_back(id); 
    m_vecVisibleRange.push_back(std::pair<double, double>(dMinRange, dMaxRange));
}

bool Parameter::getDetail(unsigned i, ID &id, double &dMinRange, double &dMaxRange) const
{
    if (i < m_vecDetails.size()) 
    {
        id = m_vecDetails[i]; 
        dMinRange = m_vecVisibleRange[i].first; 
        dMaxRange = m_vecVisibleRange[i].second; 
        return true;
    } 

    return false;
}

bool Parameter::fromBson(bson::bsonDocument &bsonDoc)
{
    // Ù–‘
    {
        bson::bsonDocumentEle *pPropertiesEle = dynamic_cast<bson::bsonDocumentEle *>(bsonDoc.GetElement("Properties"));
        if(pPropertiesEle != NULL)
        {
            bson::bsonDocument &doc = pPropertiesEle->GetDoc();
            readProperties(doc);
        }
    }

    //Symbol
    {
        bson::bsonArrayEle *pDetails = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.GetElement("Details"));
        if(pDetails == NULL)
        {
            return false;
        }

        for (unsigned int i = 0; i < pDetails->ChildCount(); i++)
        {
            bson::bsonDocumentEle *detail = (bson::bsonDocumentEle*)pDetails->GetElement(i);
            if (detail == NULL) return false;

            bson::bsonBinaryEle *pID = (bson::bsonBinaryEle*)detail->GetDoc().GetElement("ID");
            if (pID == NULL) return false;

            ID id = ID::genIDfromBinary(pID->BinData(), pID->BinDataLen());
            m_vecDetails.push_back(id);

            bson::bsonDoubleEle *pMin = (bson::bsonDoubleEle *)detail->GetDoc().GetElement("MinRange");
            bson::bsonDoubleEle *pMax = (bson::bsonDoubleEle *)detail->GetDoc().GetElement("MaxRange");

            if (!pMin || !pMax) return false;

            m_vecVisibleRange.push_back(std::pair<double, double>(pMin->DblValue(), pMax->DblValue()));
        }
    }

    {
        bson::bsonBoolEle *pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("FollowByTerrain"));
        if(pBoolEle == NULL)
        {
            return false;
        }
        setFollowByTerrain(pBoolEle->BoolValue());
    }

    {
        bson::bsonInt32Ele *pIntEle = dynamic_cast<bson::bsonInt32Ele *>(bsonDoc.GetElement("Order"));
        if(pIntEle == NULL)
        {
            return false;
        }
        setCoverOrder((unsigned int)pIntEle->Int32Value());
    }

    {
        bson::bsonDoubleEle *pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("Height"));
        if(pDoubleEle == NULL)
        {
            return false;
        }
        setHeight(pDoubleEle->DblValue());
    }

    {
        bson::bsonInt32Ele *pIntEle = dynamic_cast<bson::bsonInt32Ele *>(bsonDoc.GetElement("ActProperty"));
        if(pIntEle == NULL)
        {
            return false;
        }
        setActProperty((unsigned int)pIntEle->Int32Value());
    }


    return true;
}

bool Parameter::toBson(bson::bsonDocument &bsonDoc) const
{
    // Ù–‘
    {
        if(!m_vecProperties.empty())
        {
            bson::bsonDocumentEle *pPropertiesEle = dynamic_cast<bson::bsonDocumentEle *>(bsonDoc.AddDocumentElement("Properties"));
            if(pPropertiesEle != NULL)
            {
                bson::bsonDocument &doc = pPropertiesEle->GetDoc();
                writeProperties(doc);
            }
        }
    }

    if(!bsonDoc.AddBoolElement("FollowByTerrain", m_bFollowByTerrain) || 
        !bsonDoc.AddInt32Element("Order", m_nCoverOrder) ||
        !bsonDoc.AddDblElement("Height", m_dblHeight) ||
        !bsonDoc.AddInt32Element("ActProperty", m_nActProperty))
    {
        return false;
    }

    //Symbol
    bson::bsonArrayEle *arrDetails = (bson::bsonArrayEle *)bsonDoc.AddArrayElement("Details");

    for(size_t i = 0; i < m_vecDetails.size(); i++)
    {
        bson::bsonDocumentEle *docEle = (bson::bsonDocumentEle*)arrDetails->AddDocumentElement();
        docEle->GetDoc().AddBinElement("ID", (void*)&m_vecDetails[i].m_nLowBit, sizeof(m_vecDetails[i].m_nLowBit) * 3);
        docEle->GetDoc().AddDblElement("MinRange", m_vecVisibleRange[i].first);
        docEle->GetDoc().AddDblElement("MaxRange", m_vecVisibleRange[i].second);
    }

    return true;
}

unsigned int Parameter::addProperty(const std::string &strKey, const std::string &prop)
{
    if(strKey.empty() || prop.empty())
    {
        return ~0u;
    }
    m_vecProperties.push_back(std::make_pair(strKey, prop));
    return m_vecProperties.size() - 1;
}

void Parameter::getProperty(unsigned int nIndex, std::string &strKey, std::string &strProp) const
{
    if(nIndex < m_vecProperties.size())
    {
        const std::pair<std::string, std::string> &val = m_vecProperties[nIndex];
        strKey = val.first;
        strProp = val.second;
    }
}

unsigned int Parameter::getPropertyCount() const
{
    return m_vecProperties.size();
}

unsigned int Parameter::findProperty(const std::string &strKey, std::string &strVal) const
{
    for(unsigned int i = 0; i < getPropertyCount(); i++)
    {
        std::string tmp = m_vecProperties[i].first;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

        std::string tmp1 = strKey;
        std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), ::tolower);

        if (tmp == tmp1) 
        {
            strVal = m_vecProperties[i].second;
            return i;
        }
    }
    return -1;
}

void Parameter::readProperties(bson::bsonDocument &bsonProp)
{
    for(unsigned int i = 0; i < bsonProp.ChildCount(); i++)
    {
        std::pair<std::string , cmm::variant_data> prop;
        bson::bsonElement *pEle = bsonProp.GetElement(i);
        prop.first = pEle->EName();
        if(prop.first.empty())
        {
            continue;
        }

        const bson::bsonElementType type = pEle->GetType();
        if(type == bson::bsonStringType)
        {
            const char *pszValue = pEle->StrValue();
            std::string strValue;
            if(pszValue)
            {
                strValue = pszValue;
            }
            prop.second = strValue;
        }
        m_vecProperties.push_back(prop);
    }
}


void Parameter::writeProperties(bson::bsonDocument &bsonProp) const
{
    std::vector<std::pair<std::string, std::string> >::const_iterator itor = m_vecProperties.begin();
    for(; itor != m_vecProperties.end(); ++itor)
    {
        bsonProp.AddStringElement(itor->first.c_str(), itor->second.c_str());
    }
}

double Parameter::getMaxRange(void) const
{
    double max = -1.0;
    for(size_t i = 0; i < m_vecVisibleRange.size(); i++)
    {
        if (m_vecVisibleRange[i].second > max)
        {
            max = m_vecVisibleRange[i].second;
        }
    }

    return max;
}


bool Parameter::getSortedDetails(std::vector<ID> &vecDetails, std::vector<std::pair<double, double> > &vecRanges) const
{
    if(m_vecDetails.size() != m_vecVisibleRange.size())
    {
        assert(false);
        return false;
    }

    vecDetails = m_vecDetails;
    vecRanges = m_vecVisibleRange;

    if(m_vecDetails.size() <= 1u)
    {
        return true;
    }

    for(unsigned i = 0u; i < vecRanges.size() - 1u; i++)
    {
        double dblMaxVal0 = vecRanges[i].second;
        for(unsigned j = i + 1u; j < vecRanges.size(); j++)
        {
            const double dblMaxVal1 = vecRanges[j].second;
            if(dblMaxVal1 > dblMaxVal0)
            {
                const std::pair<double, double> tmpRange = vecRanges[i];
                const ID tmpDetail = vecDetails[i];

                vecRanges[i] = vecRanges[j];
                vecDetails[i] = vecDetails[j];

                vecRanges[j] = tmpRange;
                vecDetails[j] = tmpDetail;

                dblMaxVal0 = dblMaxVal1;
            }
        }
    }

    return true;
}


}
