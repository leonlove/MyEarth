#include "NavigationParam.h"
#include <osg/Math>
#include "Utility.h"

NavigationParam::NavigationParam(void)
{
    Default();
}

NavigationParam::NavigationParam(const NavigationParam &param)
{
    setParam(param);
}

void NavigationParam::Default(void)
{
    m_bMouseInertia         = true;            // 支持鼠标漫游惯性
	m_bUnderGroundViewMode	= true;			   // 默认为地下浏览模式
    m_bKeyboardInertia      = false;            // 支持键盘漫游惯性
    m_dblFrictionalFactor   = 0.04;            // 摩擦系数

    m_dblShiftLMag          = 4.0;
    m_dblShiftRMag          = 0.25;

    m_dblKeyboardTranslationSpeed  = 2.0;                               // 键盘的平移速度
    m_dblKeyboardRotationSpeed     = osg::DegreesToRadians(10.0);        // 键盘的旋转速度
    m_dblKeyboardSpeedBindHeight   = 100.0;

    m_bLockWithTerrain     = true;          // 漫游时是否跟随地形高程
    m_dblLockTerrainHeight = 80.0;          // 漫游时若跟随地形高程，指定跟随的最小高度

    m_NavKeyboardConfig[NK_ResetCamera]  = ' ';
    m_NavKeyboardConfig[NK_StopInertia]  = 'p';

#if 0
    m_NavKeyboardConfig[NK_Forward]  = 0xFF52;
    m_NavKeyboardConfig[NK_Backward] = 0xFF54;
    m_NavKeyboardConfig[NK_Left]     = 0xFF51;
    m_NavKeyboardConfig[NK_Right]    = 0xFF53;

    m_NavKeyboardConfig[NK_Up]       = 0xFF50;
    m_NavKeyboardConfig[NK_Down]     = 0xFF57;

    m_NavKeyboardConfig[NK_LookUp]   = 0xFF55;
    m_NavKeyboardConfig[NK_LookDown] = 0xFF56;
#else
    m_NavKeyboardConfig[NK_Forward]  = 'w';
    m_NavKeyboardConfig[NK_Backward] = 's';
    m_NavKeyboardConfig[NK_Left]     = 'a';
    m_NavKeyboardConfig[NK_Right]    = 'd';

    m_NavKeyboardConfig[NK_Up]       = 'q';
    m_NavKeyboardConfig[NK_Down]     = 'e';

    m_NavKeyboardConfig[NK_LookUp]   = 'r';
    m_NavKeyboardConfig[NK_LookDown] = 't';
#endif
    m_NavKeyboardConfig[NK_RotateRight]   = 'c';
    m_NavKeyboardConfig[NK_RotateLeft]    = 'v';

    m_NavKeyboardConfig[NK_RotateRight_V] = 'z';
    m_NavKeyboardConfig[NK_RotateLeft_V]  = 'x';

    m_NavKeyboardConfig[NK_NavPath_Pause]        = 0xFF13;
    m_NavKeyboardConfig[NK_NavPath_StepForward] = ']';
    m_NavKeyboardConfig[NK_NavPath_StepBackward]= '[';

}



bool NavigationParam::isEqual(const NavigationParam &param) const
{
    if(!osg::equivalent(m_dblKeyboardTranslationSpeed, param.m_dblKeyboardTranslationSpeed))    return false;
    if(!osg::equivalent(m_dblKeyboardRotationSpeed, param.m_dblKeyboardRotationSpeed))            return false;

    if(!osg::equivalent(m_dblShiftLMag, param.m_dblShiftLMag))                                return false;
    if(!osg::equivalent(m_dblShiftRMag, param.m_dblShiftRMag))                                return false;

    if(!!m_bMouseInertia != !!param.m_bMouseInertia)                                        return false;
		if(!!m_bMouseInertia != !!param.m_bMouseInertia)                                        return false;
    if(!!m_bUnderGroundViewMode != !!param.m_bUnderGroundViewMode)                                        return false;
    if(!osg::equivalent(m_dblFrictionalFactor, param.m_dblFrictionalFactor))                return false;

    if(!osg::equivalent(m_dblKeyboardSpeedBindHeight, param.m_dblKeyboardSpeedBindHeight))    return false;

    if(m_NavKeyboardConfig != param.m_NavKeyboardConfig)                                    return false;

    return true;
}


void NavigationParam::setParam(const NavigationParam &param)
{
    if(this == &param)    return;

    m_dblKeyboardTranslationSpeed  = param.m_dblKeyboardTranslationSpeed;
    m_dblKeyboardRotationSpeed = param.m_dblKeyboardRotationSpeed;

    m_dblShiftLMag     = param.m_dblShiftLMag;
    m_dblShiftRMag     = param.m_dblShiftRMag;
    m_bMouseInertia    = param.m_bMouseInertia;
	m_bUnderGroundViewMode    = param.m_bUnderGroundViewMode;
    m_bKeyboardInertia = param.m_bKeyboardInertia;
    m_dblFrictionalFactor = param.m_dblFrictionalFactor;

    m_dblKeyboardSpeedBindHeight = param.m_dblKeyboardSpeedBindHeight;

    m_NavKeyboardConfig = param.m_NavKeyboardConfig;
}


const NavigationParam &NavigationParam::operator=(const NavigationParam &param)
{
    setParam(param);
    return *this;
}


bool NavigationParam::operator==(const NavigationParam &param) const
{
    return isEqual(param);
}


bool NavigationParam::operator!=(const NavigationParam &param) const
{
    return !isEqual(param);
}
/*
const NavigationKeyframe &NavigationKeyframe::operator=(const NavigationKeyframe &param)
{
	if(this == &param)  return *this;
    m_CameraPose            = param.m_CameraPose;
    m_dblTrans_TimeOrSpeed  = param.m_dblTrans_TimeOrSpeed;
    m_dblRotate_TimeOrSpeed = param.m_dblRotate_TimeOrSpeed;
    m_bArgForTime           = param.m_bArgForTime;
    m_strUserDefinitionData = param.m_strUserDefinitionData;
    return *this;
}

void NavigationKeyframe::toJson(bson::bsonElement &val)
{
    if (val.GetType() != bson::bsonDocType)
    {
        return;
    }

    m_CameraPose.toJson(val);

    bson::bsonDoubleEle   *Trans_TimeOrSpeed    = NULL;
    bson::bsonDoubleEle   *Rotate_TimeOrSpeed   = NULL;
    bson::bsonDoubleEle   *ArgForTime           = NULL;
    bson::bsonStringEle   *UserDefinedData      = NULL;
    bson::bsonDocumentEle *Thumbnail            = NULL;

    bson::bsonDocumentEle *doc_elem  = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument    &doc       = doc_elem->GetDoc();

    for(unsigned int i = 0; i < doc.ChildCount(); i++)
    {
        if (strcmp(doc.GetElement(i)->EName(), "Trans_TimeOrSpeed"))
        {
            if (doc.GetElement(i)->GetType() == bson::bsonDoubleType)
            {
                doc.GetElement(i)->SetDblValue(m_dblTrans_TimeOrSpeed);
            }
        }
        else if (strcmp(doc.GetElement(i)->EName(), "Rotate_TimeOrSpeed"))
        {
            if (doc.GetElement(i)->GetType() == bson::bsonDoubleType)
            {
                doc.GetElement(i)->SetDblValue(m_dblRotate_TimeOrSpeed);
            }
        }
        else if (strcmp(doc.GetElement(i)->EName(), "ArgForTime"))
        {
            if (doc.GetElement(i)->GetType() == bson::bsonDoubleType)
            {
                doc.GetElement(i)->SetDblValue(m_bArgForTime ? 2.0f : 0.0f);
            }
        }
        else if (strcmp(doc.GetElement(i)->EName(), "UserDefinedData"))
        {
            if (doc.GetElement(i)->GetType() == bson::bsonStringType)
            {
                doc.GetElement(i)->SetStrValue(m_strUserDefinitionData.c_str());
            }
        }
        else if (strcmp(doc.GetElement(i)->EName(), "Thumbnail"))
        {
            if (doc.GetElement(i)->GetType() == bson::bsonDocType)
            {
                Thumbnail = dynamic_cast<bson::bsonDocumentEle*>(doc.GetElement(i));
            }
        }
    }

    if (!Trans_TimeOrSpeed)
    {
        doc.AddDblElement("Trans_TimeOrSpeed", m_dblTrans_TimeOrSpeed);
    }
    if (!Rotate_TimeOrSpeed)
    {
        doc.AddDblElement("Rotate_TimeOrSpeed", m_dblRotate_TimeOrSpeed);
    }
    if (!ArgForTime)
    {
        doc.AddDblElement("ArgForTime", m_bArgForTime);
    }
    if (!UserDefinedData)
    {
        doc.AddStringElement("UserDefinedData", m_strUserDefinitionData.c_str());
    }
    if (!Thumbnail)
    {
        Thumbnail = dynamic_cast<bson::bsonDocumentEle*>(doc.AddDocumentElement("Thumbnail"));
    }

    if (m_pThumbnail)
    {
        m_pThumbnail->toJson(*Thumbnail);
    }
}

bool NavigationKeyframe::fromJson(bson::bsonElement &val)
{
    if (val.GetType() != bson::bsonDocType)
    {
        return false;
    }

    bson::bsonDocumentEle *doc_elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
    bson::bsonDocument &doc = doc_elem->GetDoc();

    bson::bsonElement *CameraPose = doc.GetElement("CameraPose");
    if (!CameraPose)
    {
        return false;
    }

    if (!m_CameraPose.fromJson(*CameraPose))
    {
        return false;
    }

    bson::bsonElement *Trans_TimeOrSpeed    = doc.GetElement("Trans_TimeOrSpeed");
    bson::bsonElement *Rotate_TimeOrSpeed   = doc.GetElement("Rotate_TimeOrSpeed");
    bson::bsonElement *ArgForTime           = doc.GetElement("ArgForTime");
    bson::bsonElement *UserDefinedData      = doc.GetElement("UserDefinedData");
    bson::bsonElement *Thumbnail            = doc.GetElement("Thumbnail");

    if (!Trans_TimeOrSpeed  || Trans_TimeOrSpeed->GetType() != bson::bsonDoubleType ||
        !Rotate_TimeOrSpeed || Rotate_TimeOrSpeed->GetType()!= bson::bsonDoubleType ||
        !ArgForTime         || ArgForTime->GetType()        != bson::bsonDoubleType ||
        !UserDefinedData    || UserDefinedData->GetType()   != bson::bsonStringType)
    {
        return false;
    }

    m_dblTrans_TimeOrSpeed     = Trans_TimeOrSpeed->DblValue();
    m_dblRotate_TimeOrSpeed    = Rotate_TimeOrSpeed->DblValue();
    m_bArgForTime              = (ArgForTime->DblValue() > 1.0f ? true : false);
    m_strUserDefinitionData    = UserDefinedData->StrValue();
	m_pThumbnail			   = NULL;

    if (Thumbnail)
    {
		Utility u(NULL, NULL);
		IUtility *pUtility = dynamic_cast<IUtility *>(&u);
        m_pThumbnail = pUtility->createImageInstance();
        m_pThumbnail->fromJson(*Thumbnail);
    }
    return true;
}
*/