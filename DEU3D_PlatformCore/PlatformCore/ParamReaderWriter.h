#ifndef PARAM_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE
#define PARAM_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE

#include <osgDB/ReaderWriter>
#include <IDProvider/ID.h>
#include <ParameterSysNew/IParameter.h>

class ParamReaderWriter : public osgDB::ReaderWriter
{
public:
    ParamReaderWriter(void);
    ~ParamReaderWriter(void);

    virtual const char*             className() const { return "Param Reader"; }

    osgDB::ReaderWriter::ReadResult readNode(const ID &id, void *pData, unsigned int nLen, const osgDB::Options* options) const;
};

#endif