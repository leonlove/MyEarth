#ifndef SYMBOL_READER_WRITER_H_654CB317_1EF9_4ED5_A24C_E7D0C8AA77BE_INCLUDE
#define SYMBOL_READER_WRITER_H_654CB317_1EF9_4ED5_A24C_E7D0C8AA77BE_INCLUDE

#include <osgDB/ReaderWriter>

class SymbolReaderWriter : public osgDB::ReaderWriter
{
public:
    explicit SymbolReaderWriter(void) {};
    virtual ~SymbolReaderWriter(void) {};

    virtual const char*             className() const { return "Symbol Reader"; }

    osgDB::ReaderWriter::ReadResult readNode(const ID &id, void *pData, unsigned int nLen, const osgDB::Options* options) const;
};

#endif
