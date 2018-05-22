#ifndef VIRTUAL_TILE_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE
#define VIRTUAL_TILE_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE

#include <osgDB/ReaderWriter>
#include <IDProvider/ID.h>
#include <OpenThreads/Mutex>
#include <Common/IStateQuerier.h>
#include "../VirtualTileManager/IVirtualTileManager.h"
#include <common/Pyramid.h>
#include "DEUProxyNode.h"
#include "LODFixer.h"
#include <osgDB/fstream>

#define DUMP_V_TILE_LOG 0

class VirtualTileReaderWriter : public osgDB::ReaderWriter
{
public:
    explicit VirtualTileReaderWriter(const vtm::IVirtualTileManager *pVTileManager);
    virtual ~VirtualTileReaderWriter(void);

    virtual const char*             className(void) const { return "Virtual Tile Reader"; }

    typedef std::pair<const void *, unsigned>   VTileData;
    osgDB::ReaderWriter::ReadResult readNode(const ID &id, const VTileData &pairVTData, const osgDB::Options* options) const;

    void readObjects(const ID &id, const VTileData &vecVTData, bool vecValid[], ObjectMatrix &mtxObjectIDs) const;

protected:
    OpenSP::sp<const vtm::IVirtualTileManager>    m_pVTileManager;
    osg::ref_ptr<LODFixer>      m_pLODFixer;

#if DUMP_V_TILE_LOG
    mutable osgDB::ofstream     m_fileLogOut;
#endif
};

#endif
