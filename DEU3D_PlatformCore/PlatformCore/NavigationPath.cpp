#include "NavigationPath.h"
#include "Utility.h"
#include <osg/Math>

#define BSON_CAMERAPOSE_X               "PositionX"
#define BSON_CAMERAPOSE_Y               "PositionY"
#define BSON_CAMERAPOSE_HEIGHT          "Height"
#define BSON_CAMERAPOSE_PITCHANGLE      "PitchAngle"
#define BSON_CAMERAPOSE_AZIMUTHANGLE    "AzimuthAngle"

#define BSON_KEYFRAME_CAMERAPOSE        "CameraPose"
#define BSON_KEYFRAME_NAME              "Name"
#define BSON_KEYFRAME_TRANS             "Trans_TimeOrSpeed"
#define BSON_KEYFRAME_ROTATE            "Rotate_TimeOrSpeed"
#define BSON_KEYFRAME_ARGFORTIME        "ArgForTime"
#define BSON_KEYFRAME_USERDATA          "UserDefinedData"
#define BSON_KEYFRAME_THUMBNAIL         "Thumbnail"

#define BSON_PATH_FRAMES                "Frames"  
#define BSON_PATH_NAME                  "Name"
#define BSON_CONTAINER_PATHS            "Paths"


NavigationKeyframe::NavigationKeyframe()
{
	m_dblTrans_TimeOrSpeed = 0.0;
	m_dblRotate_TimeOrSpeed = 0.0;
	m_bArgForTime = true;
	m_strUserDefinitionData.clear();
    memset(&m_CameraPose, 0, sizeof(CameraPose));
}


NavigationKeyframe::NavigationKeyframe(const NavigationKeyframe &param)
{
    m_dblTrans_TimeOrSpeed = param.m_dblTrans_TimeOrSpeed;
    m_dblRotate_TimeOrSpeed = param.m_dblRotate_TimeOrSpeed;
    m_bArgForTime = param.m_bArgForTime;
    m_strUserDefinitionData = param.m_strUserDefinitionData;
    m_pThumbnail = param.m_pThumbnail;
    m_CameraPose = param.m_CameraPose;
}


NavigationKeyframe::~NavigationKeyframe()
{
}

const CameraPose &NavigationKeyframe::getCameraPose(void) const
{
    return m_CameraPose;
}

CameraPose &NavigationKeyframe::getCameraPose(void)
{
    return m_CameraPose;
}

void NavigationKeyframe::setCameraPose(const CameraPose &pose)
{
	m_CameraPose = pose;
}

	  
double NavigationKeyframe::getTrans_TimeOrSpeed()
{
	return m_dblTrans_TimeOrSpeed;
}

void NavigationKeyframe::setTrans_TimeOrSpeed(double dTrans_TimeOrSpeed)
{
	m_dblTrans_TimeOrSpeed = dTrans_TimeOrSpeed;
}

double NavigationKeyframe::getRotate_TimeOrSpeed()
{
	return m_dblRotate_TimeOrSpeed;
}

void NavigationKeyframe::setRotate_TimeOrSpeed(double dRotate_TimeOrSpeed)
{
	m_dblRotate_TimeOrSpeed = dRotate_TimeOrSpeed;
}

bool NavigationKeyframe::getArgForTime()
{
	return m_bArgForTime;
}

void NavigationKeyframe::setArgForTime(bool bArgForTime)
{
	m_bArgForTime = bArgForTime;
}


const std::string & NavigationKeyframe::getUserDefinitionData() const
{
	return m_strUserDefinitionData;
}

void NavigationKeyframe::setUserDefinitionData(const std::string &strUserDefinitionData)
{
	m_strUserDefinitionData = strUserDefinitionData;
}


const cmm::image::IDEUImage *NavigationKeyframe::getThumbnail(void) const
{
	return m_pThumbnail.get();
}

cmm::image::IDEUImage *NavigationKeyframe::getThumbnail(void)
{
	return m_pThumbnail.get();
}

void NavigationKeyframe::setThumbnail(cmm::image::IDEUImage *pImage)
{
	m_pThumbnail = pImage;
}


const std::string &NavigationKeyframe::getName(void) const
{
    return m_strName;
}


void  NavigationKeyframe::setName(const std::string &strName)
{
    m_strName = strName;
}


bool NavigationKeyframe::fromBson(bson::bsonElement &val)
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument &doc = doc_elem->GetDoc();

    bson::bsonElement *CameraPose = doc.GetElement(BSON_KEYFRAME_CAMERAPOSE);
    if (!CameraPose)
    {
        return false;
    }

    if (!fromBsonCamera(*CameraPose))
    {
        return false;
    }

    bson::bsonElement *Name                 = doc.GetElement(BSON_KEYFRAME_NAME);
    bson::bsonElement *Trans_TimeOrSpeed    = doc.GetElement(BSON_KEYFRAME_TRANS);
    bson::bsonElement *Rotate_TimeOrSpeed   = doc.GetElement(BSON_KEYFRAME_ROTATE);
    bson::bsonElement *ArgForTime           = doc.GetElement(BSON_KEYFRAME_ARGFORTIME);
    bson::bsonElement *UserDefinedData      = doc.GetElement(BSON_KEYFRAME_USERDATA);
    bson::bsonElement *Thumbnail            = doc.GetElement(BSON_KEYFRAME_THUMBNAIL);

    if (!Name               || Name->GetType()              != bson::bsonStringType  ||
        !Trans_TimeOrSpeed  || Trans_TimeOrSpeed->GetType() != bson::bsonDoubleType ||
        !Rotate_TimeOrSpeed || Rotate_TimeOrSpeed->GetType()!= bson::bsonDoubleType ||
        !ArgForTime         || ArgForTime->GetType()        != bson::bsonDoubleType ||
        !UserDefinedData    || UserDefinedData->GetType()   != bson::bsonStringType)
    {
        return false;
    }

    m_strName                  = Name->StrValue();
    m_dblTrans_TimeOrSpeed     = Trans_TimeOrSpeed->DblValue();
    m_dblRotate_TimeOrSpeed    = Rotate_TimeOrSpeed->DblValue();
    m_bArgForTime              = (ArgForTime->DblValue() > 0.5f ? true : false);
    m_strUserDefinitionData    = UserDefinedData->StrValue();
	m_pThumbnail			   = NULL;

    if (Thumbnail)
    {
        m_pThumbnail = cmm::image::createDEUImage();
        if (!m_pThumbnail->fromBson(*Thumbnail))
        {
            return false;
        }

    }
    return true;
}


bool NavigationKeyframe::toBson(bson::bsonElement &val) const
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem  = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument    &doc       = doc_elem->GetDoc();

    toBsonCamera(val);
    doc.AddStringElement(BSON_KEYFRAME_NAME, m_strName.c_str());
    doc.AddDblElement(BSON_KEYFRAME_TRANS, m_dblTrans_TimeOrSpeed);
    doc.AddDblElement(BSON_KEYFRAME_ROTATE, m_dblRotate_TimeOrSpeed);
    doc.AddDblElement(BSON_KEYFRAME_ARGFORTIME, m_bArgForTime);
    doc.AddStringElement(BSON_KEYFRAME_USERDATA, m_strUserDefinitionData.c_str());
    if (m_pThumbnail != NULL)
    {                          
        bson::bsonDocumentEle *Thumbnail = dynamic_cast<bson::bsonDocumentEle*>(doc.AddDocumentElement(BSON_KEYFRAME_THUMBNAIL));
        if(Thumbnail)
        {
            m_pThumbnail->toBson(*Thumbnail);
        }       
    }

    return true;
}

bool NavigationKeyframe::fromBsonCamera(bson::bsonElement &val)
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument &doc = doc_elem->GetDoc();

    bson::bsonElement *posX         = doc.GetElement(BSON_CAMERAPOSE_X);
    bson::bsonElement *posY         = doc.GetElement(BSON_CAMERAPOSE_Y);
    bson::bsonElement *Height       = doc.GetElement(BSON_CAMERAPOSE_HEIGHT);
    bson::bsonElement *PitchAngle   = doc.GetElement(BSON_CAMERAPOSE_PITCHANGLE);
    bson::bsonElement *AzimuthAngle = doc.GetElement(BSON_CAMERAPOSE_AZIMUTHANGLE);

    if (!posX || posX->GetType() != bson::bsonDoubleType)
    {
        return false;
    }

    if (!posY || posY->GetType() != bson::bsonDoubleType)
    {
        return false;
    }

    if (!Height || Height->GetType() != bson::bsonDoubleType)
    {
        return false;
    }

    if (!PitchAngle || PitchAngle->GetType() != bson::bsonDoubleType)
    {
        return false;
    }

    if (!AzimuthAngle || AzimuthAngle->GetType() != bson::bsonDoubleType)
    {
        return false;
    }

    m_CameraPose.m_dblPositionX    = posX->DblValue();
    m_CameraPose.m_dblPositionY    = posY->DblValue();
    m_CameraPose.m_dblHeight       = Height->DblValue();
    m_CameraPose.m_dblPitchAngle   = PitchAngle->DblValue();
    m_CameraPose.m_dblAzimuthAngle = AzimuthAngle->DblValue();

    return true;
}

bool NavigationKeyframe::toBsonCamera(bson::bsonElement &val) const
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem  = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonElement     *pose_elem = doc_elem->GetDoc().AddDocumentElement(BSON_KEYFRAME_CAMERAPOSE);
    bson::bsonDocumentEle *doc_pose = dynamic_cast<bson::bsonDocumentEle*>(pose_elem);
    bson::bsonDocument &doc = doc_pose->GetDoc();

    doc.AddDblElement(BSON_CAMERAPOSE_X, m_CameraPose.m_dblPositionX);
    doc.AddDblElement(BSON_CAMERAPOSE_Y, m_CameraPose.m_dblPositionY);
    doc.AddDblElement(BSON_CAMERAPOSE_HEIGHT, m_CameraPose.m_dblHeight);
    doc.AddDblElement(BSON_CAMERAPOSE_PITCHANGLE, m_CameraPose.m_dblPitchAngle);
    doc.AddDblElement(BSON_CAMERAPOSE_AZIMUTHANGLE, m_CameraPose.m_dblAzimuthAngle);

    return true;
}






NavigationPath::NavigationPath()
{
	m_strPathName.clear();
	m_ListNavigationKeyframe.clear();
}

NavigationPath::NavigationPath(const NavigationPath &param)
{
    m_ListNavigationKeyframe.clear();
    for (unsigned n = 0; n < param.getItemCount(); n++)
    {
        appendItem((INavigationKeyframe *)param.getItem(n));
    }
}

NavigationPath::~NavigationPath()
{
}


void NavigationPath::replaceItem(unsigned int nIndex, INavigationKeyframe * pKeyframe)
{
	if (nIndex >= m_ListNavigationKeyframe.size())
	{
		return;
	}
	NavigationKeyframe *p = dynamic_cast<NavigationKeyframe *>(pKeyframe);
	m_ListNavigationKeyframe[nIndex] = p;
}


const INavigationKeyframe* NavigationPath::getItem(unsigned int nIndex) const
{
	if (nIndex >= m_ListNavigationKeyframe.size())
	{
		return NULL;
	}
	return m_ListNavigationKeyframe[nIndex].get();
}

INavigationKeyframe* NavigationPath::getItem(unsigned int nIndex)
{
	if (nIndex >= m_ListNavigationKeyframe.size())
	{
		return NULL;
	}
	return m_ListNavigationKeyframe[nIndex].get();
}

void NavigationPath::deleteItem(unsigned int nIndex)
{
	if (nIndex >= m_ListNavigationKeyframe.size())
	{
		return;
	}
	m_ListNavigationKeyframe.erase(m_ListNavigationKeyframe.begin() + nIndex);
}

void NavigationPath::appendItem(INavigationKeyframe * pKeyframe)
{
	if (NULL == pKeyframe) 
	{
		return;
	}

	NavigationKeyframe* p = dynamic_cast<NavigationKeyframe *>(pKeyframe);
	m_ListNavigationKeyframe.push_back(p);
}

void NavigationPath::appendItemByCameraPos(const CameraPose &pose, double dLongitude, double dLatitude, double dHeight)
{
	OpenSP::sp<NavigationKeyframe> beginKeyframe = new NavigationKeyframe;
	beginKeyframe->setCameraPose(pose);
	beginKeyframe->setArgForTime(true);
	beginKeyframe->setRotate_TimeOrSpeed(8.0);
	beginKeyframe->setTrans_TimeOrSpeed(8.0);
	appendItem(beginKeyframe.get());

	double dDistance = getDistance(pose.m_dblPositionX, pose.m_dblPositionY, dLongitude, dLatitude);
	if (dDistance == 0.0)
	{
		beginKeyframe->setRotate_TimeOrSpeed(0.2);
		beginKeyframe->setTrans_TimeOrSpeed(0.2);
	}
	else if (dDistance < 500000.0)
	{
		Region(beginKeyframe, dLongitude, dLatitude, dDistance);
	}
	else if (dDistance > 500000.0 && dDistance <= 5000000.0)
	{
		Province(beginKeyframe, dLongitude, dLatitude, dDistance);
	}
	else
	{
		Country(beginKeyframe, dLongitude, dLatitude, dDistance);
	}

	return;
}

void NavigationPath::insertItem(unsigned int nIndex, INavigationKeyframe * pKeyframe)
{
	ListNavigationKeyframe::iterator itorPos = m_ListNavigationKeyframe.begin();
	if(nIndex <= m_ListNavigationKeyframe.size())
	{
		itorPos += nIndex;
		NavigationKeyframe *pPath = dynamic_cast<NavigationKeyframe *>(pKeyframe);
		m_ListNavigationKeyframe.insert(itorPos, pPath);
	}
}

unsigned int NavigationPath::getItemCount(void) const
{
	return m_ListNavigationKeyframe.size();
}

void NavigationPath::clear()
{
    m_ListNavigationKeyframe.clear();
}

const std::string & NavigationPath::getPathName() const
{
	return m_strPathName;
}

void NavigationPath::setPathName(const std::string &strName)
{
	m_strPathName = strName;
}


bool NavigationPath::fromBson(bson::bsonElement &val)
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument &bsonDoc = doc_elem->GetDoc();
    bson::bsonElement *name = bsonDoc.GetElement(BSON_PATH_NAME);
    if(!name)   return false;

    m_strPathName = name->StrValue();

	bson::bsonArrayEle* ary = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement(BSON_PATH_FRAMES));
	for(unsigned int i = 0; i < ary->ChildCount(); i++)
	{
		bson::bsonElement *pPathElement = ary->GetElement(i);
        if (NULL == pPathElement)
		{
			return false;
		}
		OpenSP::sp<NavigationKeyframe>  pFrame = new NavigationKeyframe;
		pFrame->fromBson(*pPathElement);
		m_ListNavigationKeyframe.push_back(pFrame);
	}
		
    return true;
}

bool NavigationPath::toBson(bson::bsonElement &val) const
{
	if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem  = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument    &bsonDoc       = doc_elem->GetDoc();
    bsonDoc.AddStringElement(BSON_PATH_NAME, m_strPathName.c_str());

    bson::bsonArrayEle* pArray = (bson::bsonArrayEle*)bsonDoc.AddArrayElement(BSON_PATH_FRAMES);
	//将每一个关键帧转换为Bson
	for(size_t i = 0; i < m_ListNavigationKeyframe.size(); i++)
    {
		if (!m_ListNavigationKeyframe[i]->toBson(*pArray->AddDocumentElement()))
		{
			return false;
		}
    }
    
	return true;
}

double NavigationPath::getDistance(double lat1, double long1, double lat2, double long2)
{
	double radlat1 = lat1;
	double radlat2 = lat2;
	double a = radlat1 - radlat2;
	double b = long1 - long2;

	double s = 2 * asin(sqrt(pow(sin(a / 2), 2))) + cos(radlat1) * cos(radlat2) * pow(sin(b/2),2);
	s = s * 6378.137;
	s = osg::round(s * 10000) / 10000;
	s = s * 1000;
	return s;
}

void NavigationPath::Region(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance)
{
	OpenSP::sp<NavigationKeyframe> keyframe = new NavigationKeyframe;

	keyframe->getCameraPose().m_dblAzimuthAngle = beginKeyframe->getCameraPose().m_dblAzimuthAngle;
	keyframe->getCameraPose().m_dblHeight = 800000.0;
	keyframe->getCameraPose().m_dblPitchAngle = beginKeyframe->getCameraPose().m_dblPitchAngle;
	keyframe->getCameraPose().m_dblPositionX = dLongitude;
	keyframe->getCameraPose().m_dblPositionY = dLatitude;
	keyframe->setRotate_TimeOrSpeed(8.0);
	keyframe->setTrans_TimeOrSpeed(8.0);
	keyframe->setArgForTime(true);

	if (beginKeyframe->getCameraPose().m_dblHeight > 800000.0)
	{
		beginKeyframe->setRotate_TimeOrSpeed(5.0);
		beginKeyframe->setTrans_TimeOrSpeed(5.0);
		appendItem(keyframe);
	}
	else
	{
		OpenSP::sp<NavigationKeyframe> keyframe1 = new NavigationKeyframe;
		keyframe1->getCameraPose().m_dblPositionX = beginKeyframe->getCameraPose().m_dblPositionX;
		keyframe1->getCameraPose().m_dblPositionY = beginKeyframe->getCameraPose().m_dblPositionY;
		keyframe1->setArgForTime(true);

		OpenSP::sp<NavigationKeyframe> keyframe2 = new NavigationKeyframe;
		keyframe2->getCameraPose().m_dblPositionX = dLongitude;
		keyframe2->getCameraPose().m_dblPositionY = dLatitude;
		keyframe2->setArgForTime(true);

		if (dDistance < 250000.0)
		{
			if (dDistance == 0)
			{
				beginKeyframe->setRotate_TimeOrSpeed(0.2);
				beginKeyframe->setTrans_TimeOrSpeed(0.2);
			}
			else if (dDistance < 500.0)
			{
				beginKeyframe->setRotate_TimeOrSpeed(3.0);
				beginKeyframe->setTrans_TimeOrSpeed(3.0);
			}
			else if (dDistance < 5000.0)
			{
				beginKeyframe->setRotate_TimeOrSpeed(5.0);
				beginKeyframe->setTrans_TimeOrSpeed(5.0);
			}
			else if (dDistance < 20000.0)
			{
				beginKeyframe->setRotate_TimeOrSpeed(7.0);
				beginKeyframe->setTrans_TimeOrSpeed(7.0);
			}
			else if (dDistance < 100000.0)
			{
				beginKeyframe->setRotate_TimeOrSpeed(9.0);
				beginKeyframe->setTrans_TimeOrSpeed(9.0);
			}
			else
			{
				keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
				keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
				keyframe1->getCameraPose().m_dblHeight = 500000.0;
				keyframe1->setRotate_TimeOrSpeed(4.0);
				keyframe1->setTrans_TimeOrSpeed(4.0);
				appendItem(keyframe1);

				keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
				keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
				keyframe2->getCameraPose().m_dblHeight = 500000.0;
				keyframe2->setRotate_TimeOrSpeed(5.0);
				keyframe2->setTrans_TimeOrSpeed(5.0);
				appendItem(keyframe2);
			}
		}
		else
		{
			beginKeyframe->setRotate_TimeOrSpeed(8.0);
			beginKeyframe->setTrans_TimeOrSpeed(8.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 800000.0;
			keyframe1->setRotate_TimeOrSpeed(5.0);
			keyframe1->setTrans_TimeOrSpeed(5.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
	}

	return;
}

void NavigationPath::Province(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance)
{
	OpenSP::sp<NavigationKeyframe> keyframe = new NavigationKeyframe;

	keyframe->getCameraPose().m_dblAzimuthAngle = beginKeyframe->getCameraPose().m_dblAzimuthAngle;
	keyframe->getCameraPose().m_dblHeight = 800000.0;
	keyframe->getCameraPose().m_dblPitchAngle = beginKeyframe->getCameraPose().m_dblPitchAngle;
	keyframe->getCameraPose().m_dblPositionX = dLongitude;
	keyframe->getCameraPose().m_dblPositionY = dLatitude;
	keyframe->setRotate_TimeOrSpeed(10.0);
	keyframe->setTrans_TimeOrSpeed(10.0);
	keyframe->setArgForTime(true);

	if (beginKeyframe->getCameraPose().m_dblHeight > 800000.0)
	{
		beginKeyframe->setRotate_TimeOrSpeed(10.0);
		beginKeyframe->setTrans_TimeOrSpeed(10.0);
		appendItem(keyframe);
	}
	else
	{
		OpenSP::sp<NavigationKeyframe> keyframe1 = new NavigationKeyframe;
		keyframe1->getCameraPose().m_dblPositionX = beginKeyframe->getCameraPose().m_dblPositionX;
		keyframe1->getCameraPose().m_dblPositionY = beginKeyframe->getCameraPose().m_dblPositionY;
		keyframe1->setArgForTime(true);

		OpenSP::sp<NavigationKeyframe> keyframe2 = new NavigationKeyframe;
		keyframe2->getCameraPose().m_dblPositionX = dLongitude;
		keyframe2->getCameraPose().m_dblPositionY = dLatitude;
		keyframe2->setArgForTime(true);

		if (dDistance < 1000000.0)
		{
			beginKeyframe->setRotate_TimeOrSpeed(10.0);
			beginKeyframe->setTrans_TimeOrSpeed(10.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 2000000.0;
			keyframe1->setRotate_TimeOrSpeed(6.0);
			keyframe1->setTrans_TimeOrSpeed(6.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
		else if (dDistance < 3000000.0)
		{
			beginKeyframe->setRotate_TimeOrSpeed(10.0);
			beginKeyframe->setTrans_TimeOrSpeed(10.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 3000000.0;
			keyframe1->setRotate_TimeOrSpeed(8.0);
			keyframe1->setTrans_TimeOrSpeed(8.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
		else
		{
			beginKeyframe->setRotate_TimeOrSpeed(10.0);
			beginKeyframe->setTrans_TimeOrSpeed(10.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 10000000.0;
			keyframe1->setRotate_TimeOrSpeed(14.0);
			keyframe1->setTrans_TimeOrSpeed(14.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
	}

	return;
}

void NavigationPath::Country(OpenSP::sp<NavigationKeyframe> beginKeyframe, double dLongitude, double dLatitude, double dDistance)
{
	OpenSP::sp<NavigationKeyframe> keyframe = new NavigationKeyframe;

	keyframe->getCameraPose().m_dblAzimuthAngle = beginKeyframe->getCameraPose().m_dblAzimuthAngle;
	keyframe->getCameraPose().m_dblHeight = 800000.0;
	keyframe->getCameraPose().m_dblPitchAngle = beginKeyframe->getCameraPose().m_dblPitchAngle;
	keyframe->getCameraPose().m_dblPositionX = dLongitude;
	keyframe->getCameraPose().m_dblPositionY = dLatitude;
	keyframe->setRotate_TimeOrSpeed(6.0);
	keyframe->setTrans_TimeOrSpeed(6.0);
	keyframe->setArgForTime(true);

	if (beginKeyframe->getCameraPose().m_dblHeight > 5000000.0)
	{
		beginKeyframe->setRotate_TimeOrSpeed(10.0);
		beginKeyframe->setTrans_TimeOrSpeed(10.0);
		appendItem(keyframe);
	}
	else
	{
		OpenSP::sp<NavigationKeyframe> keyframe1 = new NavigationKeyframe;
		keyframe1->getCameraPose().m_dblPositionX = beginKeyframe->getCameraPose().m_dblPositionX;
		keyframe1->getCameraPose().m_dblPositionY = beginKeyframe->getCameraPose().m_dblPositionY;
		keyframe1->setArgForTime(true);

		OpenSP::sp<NavigationKeyframe> keyframe2 = new NavigationKeyframe;
		keyframe2->getCameraPose().m_dblPositionX = dLongitude;
		keyframe2->getCameraPose().m_dblPositionY = dLatitude;
		keyframe2->setArgForTime(true);

		if (dDistance < 8000000.0)
		{
			beginKeyframe->setRotate_TimeOrSpeed(10.0);
			beginKeyframe->setTrans_TimeOrSpeed(10.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 6000000.0;
			keyframe1->setRotate_TimeOrSpeed(10.0);
			keyframe1->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
		else
		{
			beginKeyframe->setRotate_TimeOrSpeed(10.0);
			beginKeyframe->setTrans_TimeOrSpeed(10.0);

			keyframe1->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe1->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe1->getCameraPose().m_dblHeight = 10000000.0;
			keyframe1->setRotate_TimeOrSpeed(10.0);
			keyframe1->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe1);

			keyframe2->getCameraPose().m_dblAzimuthAngle = 1.5707963267948966;
			keyframe2->getCameraPose().m_dblPitchAngle = 0.0;
			keyframe2->getCameraPose().m_dblHeight = 800000.0;
			keyframe2->setRotate_TimeOrSpeed(10.0);
			keyframe2->setTrans_TimeOrSpeed(10.0);
			appendItem(keyframe2);
		}
	}

	return;
}



NavigationPathContainer::NavigationPathContainer()
{
	m_ListNavigationPath.clear();
}
	
NavigationPathContainer::~NavigationPathContainer()
{
}

void NavigationPathContainer::replaceItem(unsigned int nIndex, INavigationPath * pNavigationPath)
{
	if (nIndex >= m_ListNavigationPath.size())
	{
		return;
	}
	NavigationPath *pPath = dynamic_cast<NavigationPath *>(pNavigationPath);
	m_ListNavigationPath[nIndex] = pPath;
}

INavigationPath * NavigationPathContainer::getItem(unsigned int nIndex)
{
	if (nIndex >= m_ListNavigationPath.size())
	{
		return NULL;
	}
	return m_ListNavigationPath[nIndex].get();
}

const INavigationPath * NavigationPathContainer::getItem(unsigned int nIndex) const
{
	if (nIndex >= m_ListNavigationPath.size())
	{
		return NULL;
	}
	return m_ListNavigationPath[nIndex].get();
}

void NavigationPathContainer::deleteItem(unsigned int nIndex)
{
	if (nIndex >= m_ListNavigationPath.size())
	{
		return;
	}
	m_ListNavigationPath.erase(m_ListNavigationPath.begin() + nIndex);
}

void NavigationPathContainer::appendItem(INavigationPath * pNavigationPath)
{
	if (NULL == pNavigationPath) 
	{
		return;
	}

	NavigationPath *pPath = dynamic_cast<NavigationPath *>(pNavigationPath);
	m_ListNavigationPath.push_back(pPath);
}

void NavigationPathContainer::insertItem(unsigned int nIndex, INavigationPath * pNavigationPath)
{
	ListNavigationPath::iterator itorPos = m_ListNavigationPath.begin();
	if(nIndex < m_ListNavigationPath.size())
	{
		itorPos += nIndex;
		NavigationPath *pPath = dynamic_cast<NavigationPath *>(pNavigationPath);
		m_ListNavigationPath.insert(itorPos, pPath);
	}
}

unsigned int NavigationPathContainer::getItemCount(void) const
{
	return m_ListNavigationPath.size();
}

void NavigationPathContainer::clear()
{
    m_ListNavigationPath.clear();
}

bool NavigationPathContainer::saveToFile(const std::string &strFileName) const
{
	FILE *pFile = fopen(strFileName.c_str(), "w");
    if(NULL == pFile)  
	{
		return false;
	}
    
    bson::bsonDocument bsonDoc;
    bson::bsonArrayEle* pArray = (bson::bsonArrayEle*)bsonDoc.AddArrayElement(BSON_CONTAINER_PATHS);

	//将每一条漫游路径转换为Bson
	for(size_t i = 0; i < m_ListNavigationPath.size(); i++)
    {
		if (!m_ListNavigationPath[i]->toBson(*pArray->AddDocumentElement()))
		{
			return false;
		}
    }

    std::string s;

	//bson转换为json
    bsonDoc.JsonString(s);
    size_t len = s.size();

	//将json写入文件
    fwrite(s.c_str(), len, 1, pFile);
    fclose(pFile);
    
	return true;
}

bool NavigationPathContainer::loadFromFile(const std::string &strFileName)
{
    m_ListNavigationPath.clear();
    FILE *pFile = fopen(strFileName.c_str(), "rb");
    if (NULL == pFile)
	{
		return false;
	}
    const unsigned nFileLen = cmm::getFileLength(pFile);
    if(nFileLen <= 0u)
	{
		return false;
	}

    std::string strJson;
    strJson.resize(nFileLen);
 
	//从文件中获取json对象
    unsigned iReadLen = 0;
    unsigned iReadTotalLen = 0;
    while (iReadTotalLen < nFileLen)
    {
        iReadLen = fread((char *)strJson.data()+iReadTotalLen, 1, nFileLen, pFile);
        iReadTotalLen += iReadLen;
        if (iReadLen < 1)
        {
            break;
        }
    }
    fclose(pFile);

    bson::bsonDocument bsonDoc;

	//将json转换为bson
    bsonDoc.FromJsonString(strJson);
    bson::bsonArrayEle* ary = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement(BSON_CONTAINER_PATHS));
    if(NULL == ary) 
	{
		return false;
	}
    for(unsigned int i = 0; i < ary->ChildCount(); i++)
    {
        bson::bsonElement *pPathElement = ary->GetElement(i);
        if (NULL == pPathElement)
		{
			continue;    
		}
        OpenSP::sp<NavigationPath> pPath = new NavigationPath();
		//从bson对象获取漫游路径
        if(!pPath->fromBson(*pPathElement))
        {
            continue;
        }
		//将漫游路径保存到列表
		m_ListNavigationPath.push_back(pPath);
	}

	return true;
}