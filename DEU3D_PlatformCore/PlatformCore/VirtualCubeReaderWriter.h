#ifndef VIRTUAL_CUBE_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE
#define VIRTUAL_CUBE_READER_WRITER_H_74F46DC1_23B3_4ED1_80AA_8A00EC294B00_INCLUDE

#include <osgDB/ReaderWriter>
#include <IDProvider/ID.h>
#include <OpenThreads/Mutex>
#include <Common/IStateQuerier.h>
#include "../VirtualTileManager/IVirtualCubeManager.h"
#include <common/Pyramid.h>
#include "DEUProxyNode.h"
#include "LODFixer.h"
#include <osgDB/fstream>

#include "VirtualCubeNode.h"

class VirtualCubeReaderWriter : public osgDB::ReaderWriter
{
public:
    explicit VirtualCubeReaderWriter(const vcm::IVirtualCubeManager *pVCubeManager);
    virtual ~VirtualCubeReaderWriter(void);

    virtual const char*             className(void) const { return "Virtual Cube Reader"; }

    typedef std::pair<const void *, unsigned>   VCubeData;
    osgDB::ReaderWriter::ReadResult readNode(const ID &id, const VCubeData &pairVTData, const osgDB::Options* options) const;

    void readObjects(const ID &id, const VCubeData &vecVCData, bool vecValid[], ObjectCube &cubeObjectIDs) const;

protected:
    OpenSP::sp<const vcm::IVirtualCubeManager>    m_pVCubeManager;
    osg::ref_ptr<LODFixer>      m_pLODFixer;
};

#endif
