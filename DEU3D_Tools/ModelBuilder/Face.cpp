#include "Face.h"
#include <Common/deuMath.h>
#include <IDProvider/Definer.h>
#include "ParameterSys/Parameter.h"

ParamFace::ParamFace(unsigned short dataset_code)
{
    if (dataset_code <= 100)
    {
        throw std::string("错误：数据集编号必须大于100");
    }

    _id = ID::genNewID();
    _id.ObjectID.m_nDataSetCode = dataset_code;
    _id.ObjectID.m_nType        = PARAM_FACE_ID;
}

ParamFace::ParamFace(param::IFaceParameter *base)
{
    if (base == NULL)
    {
        throw std::string("错误：面参数不能为空");
    }

    _base = base; 
    _id   = base->getID();
}

ParamFace::~ParamFace(void)
{
}

const ID& ParamFace::getID()const
{
   return _id;
}

param::IParameter* ParamFace::base()
{
    return dynamic_cast<param::IParameter*>(_base.get());
}

cmm::math::Sphered ParamFace::getBoundingSphere()
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

bool ParamFace::writeToDB(const ID &idParent, deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    bson::bsonDocument doc;
    param::Parameter *pParameter = dynamic_cast<param::Parameter *>(_base.get());
    if (!pParameter) 
    {
        return false;
    }

    if (!pParameter->toBson(doc))
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
