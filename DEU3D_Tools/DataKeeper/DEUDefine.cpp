#include "DEUDefine.h"

bool CDeuIDString::getModelIDByIDString(const std::string &strID, std::string &modelID)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    modelID = (char*)id.ObjectID.m_UniqueID;

    return true;
}


bool CDeuIDString::getDataSetCodeByIDString(const std::string &strID, unsigned int &nDataSetCode)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    nDataSetCode = id.ObjectID.m_nDataSetCode;

    return true;
}


bool CDeuIDString::getLevelByIDString(const std::string &strID, unsigned int &nLevel)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    nLevel = id.TileID.m_nLevel;

    return true;
}


bool CDeuIDString::getRowByIDString(const std::string &strID, unsigned int &nRow)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    nRow = id.TileID.m_nRow;

    return true;
}


bool CDeuIDString::getColByIDString(const std::string &strID, unsigned int &nCol)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    nCol = id.TileID.m_nCol;

    return true;
}


bool CDeuIDString::getTypeByIDString (const std::string &strID, int &nType)
{
    ID id = ID::genIDfromString(strID);

    if (!id.isValid())
    {
        return false;
    }

    nType = (int)id.TileID.m_nType;

    return true;
}

void CDeuCallEvent::CallEventState(const int nIndex, const char* strDesc, const ID* pID, ea::IEventAdapter *pEventAdapter)
{
    if (strDesc == NULL || pID == NULL || pEventAdapter == NULL)
    {
        return;
    }

    OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
    if (pEventObj == NULL)
    {
        return;
    }

    pEventObj->setAction(ea::ACTION_SUBMIT_DATA);
    std::string strInfo = strDesc;
    pEventObj->putExtra("IndexNum", nIndex);
    pEventObj->putExtra("Descrip", strInfo);

    std::string strID = "";
    if (pID)
    {
        strID = pID->toString();
    }
    pEventObj->putExtra("Pointer", strID);

    pEventAdapter->sendBroadcast(pEventObj);
}

void CDeuCallEvent::CallBreakEventState(const char* strDesc, const ID* pID, ea::IEventAdapter *pEventAdapter)
{
    if (strDesc == NULL || pID == NULL || pEventAdapter == NULL)
    {
        return;
    }

    std::string strInfo = strDesc;

    OpenSP::sp<ea::IEventObject> pEventObj = ea::createEventObject();
    if (pEventObj == NULL)
    {
        return;
    }

    pEventObj->setAction(ea::ACTION_SUBMIT_BREAK);
    pEventObj->putExtra("Descrip", strInfo);
    pEventObj->putExtra("isFinish", 1);

    std::string strID = "";
    if (pID)
    {
        strID = pID->toString();
    }
    pEventObj->putExtra("Pointer", strID);

    pEventAdapter->sendBroadcast(pEventObj);
}
