#include "Line.h"
#include <IDProvider/Definer.h>
#include "ParameterSys/Parameter.h"

ParamLine::ParamLine(unsigned short dataset_code)
{
    if (dataset_code <= 100)
    {
        throw std::string("错误：数据集编号必须大于100");
    }

    _id = ID::genNewID();
    _id.ObjectID.m_nDataSetCode = dataset_code;
    _id.ObjectID.m_nType = PARAM_LINE_ID;
}

ParamLine::ParamLine(param::ILineParameter* base)
{
    if (base == NULL)
    {
        throw std::string("错误：线参数不能为空");
    }

    _base = base;
    _id   = base->getID();
}

ParamLine::~ParamLine(void)
{
}

const ID& ParamLine::getID()const
{
    return _id;
}

param::IParameter* ParamLine::base()
{
    return dynamic_cast<param::IParameter*>(_base.get());
}

cmm::math::Sphered ParamLine::getBoundingSphere()
{
    if (_bs.getRadius() < 0.001f)
    {
        if (_base->getCoordinateCount() == 0)
        {
            return _bs;
        }

        _bs = _base->getBoundingSphere();
    }
    
    return _bs;
}


bool ParamLine::writeToDB(const ID &idParent, deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    static size_t cnt = 0, size = 0;

    bson::bsonDocument doc;
    param::Parameter *pParam = dynamic_cast<param::Parameter *>(_base.get());
    if (!pParam)
    {
        return false;
    }

    if (!pParam->toBson(doc))
    {
        return false;
    }

    bson::bsonStream bs;
    if (!doc.Write(&bs))
    {
        return false;
    }

    return db->addBlock(getID(), bs.Data(), bs.DataLen());
}

