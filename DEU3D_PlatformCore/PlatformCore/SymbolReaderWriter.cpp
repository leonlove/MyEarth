#include "SymbolReaderWriter.h"

#include "SymbolNode.h"

osgDB::ReaderWriter::ReadResult SymbolReaderWriter::readNode(const ID &id, void *pData, unsigned int nLen, const osgDB::Options* options) const
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

    OpenSP::sp<param::ISymbol> pSymbol = param::createSymbol(id);
    if(!pSymbol->fromBson(bsonDoc))
    {
        return rr;
    }

    osg::ref_ptr<SymbolNode> pSymbolNode = new SymbolNode(pSymbol.get());

    return pSymbolNode.release();
}