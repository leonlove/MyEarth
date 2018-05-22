#include "ParamReaderWriter.h"

#include <Common/DEUBson.h>
#include <osgDB/ReadFile>
#include <ParameterSysNew\Parameter.h>
#include <ParameterSys/Parameter.h>
#include "ParameterNode.h"

ParamReaderWriter::ParamReaderWriter(void)
{
}

ParamReaderWriter::~ParamReaderWriter(void)
{
}

osgDB::ReaderWriter::ReadResult ParamReaderWriter::readNode(const ID &id, void *pData, unsigned int nLen, const osgDB::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rr;

    if(pData == NULL || nLen == 0)
    {
        return rr;
    }

    bson::bsonDocument  bsonDoc;
    bson::bsonStream    bs;
    bs.Write(pData, nLen);
    bs.Reset();
    bsonDoc.Read(&bs);

    OpenSP::sp<param::IParameter> pIParameter = param::createParameter(id);
    if(!pIParameter.valid())
    {
        return rr;
    }

    if(pIParameter->fromBson(bsonDoc))
    {
        param::Parameter *pParameter = dynamic_cast<param::Parameter *>(pIParameter.get());

        if(pParameter != NULL)
        {
            rr = pParameter->createParameterNode();
        }
    }

    return rr;
}