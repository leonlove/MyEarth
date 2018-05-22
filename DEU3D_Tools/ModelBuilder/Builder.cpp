#include "Builder.h"
#include <EventAdapter\IEventObject.h>
#include "ErrorCode.h"

OpenSP::sp<ea::IEventAdapter> Builder::_ea;
unsigned int Builder::_resolutionH = 1280;
unsigned int Builder::_resolutionV = 768;
float        Builder::_max_pixel_size = 320.0;
float        Builder::_min_pixel_size = 1.0;

Builder::Builder(void)
{

}

Builder::~Builder(void)
{
}

void Builder::setEventAdapter(ea::IEventAdapter *ea)
{
    _ea = ea;
}

void Builder::writeLog(const char *info)
{
    if (_ea)
    {
		std::string strInfo(info);
        cmm::variant_data data(strInfo);

        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_MODEL_BUILDER);
        pEventObject->putExtra("LogInfo", data);
        _ea->sendBroadcast(pEventObject.get());
    }
}

void Builder::updateProgress(unsigned progress, unsigned total)
{
    if (_ea)
    {
        cmm::variant_data p(progress);
        cmm::variant_data t(total);

        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_MODEL_BUILDER);
        pEventObject->putExtra("Progress_Current", p);
        pEventObject->putExtra("Progress_Total", t);
        _ea->sendBroadcast(pEventObject.get());
    }
}

bson::bsonArrayEle* Builder::readOrCreateIDList(bson::bsonDocument &doc, deudbProxy::IDEUDBProxy *db)
{
    ID idCfg;
    memset(&idCfg, 0, sizeof(ID));

    void         *pData  = NULL;
    unsigned int len     = 0;

    if(db->readBlock(idCfg, pData, len))
    {
        //符号ID列表已存在，则追加
        bson::bsonStream old;
        if (!old.Write(pData, len))
        {
            goto CREATE;
        }

        old.Reset();
        if (!doc.Read(&old))
        {
            goto CREATE;
        }

        delete[] pData;
        return (bson::bsonArrayEle *)doc.GetElement("IDList");
    }

CREATE:
    return dynamic_cast<bson::bsonArrayEle *>(doc.AddArrayElement("IDList"));
}

cmm::IDEUException *Builder::writeIDList(bson::bsonDocument &doc, deudbProxy::IDEUDBProxy *db)
{
    ID idCfg;
    memset(&idCfg, 0, sizeof(ID));

    std::string str;
    doc.JsonString(str);

    bson::bsonStream stream;
    doc.Write(&stream);

    OpenSP::sp<cmm::IDEUException> e = cmm::createDEUException();
    if (!doc.Write(&stream))
    {
        e->setReturnCode(MAKE_ERR_CODE(5));
        e->setMessage("bson流转换失败");
        e->setTargetSite("Layer::writeIDList()");
        return e.release();
    }

    if (!db->updateBlock(idCfg, stream.Data(), stream.DataLen()))
    {
        e->setReturnCode(MAKE_ERR_CODE(6));
        e->setMessage("写数据库失败");
        e->setTargetSite("Layer::writeIDList()");
        return e.release();
    }

    e->Reset();
    return e.release();
}
