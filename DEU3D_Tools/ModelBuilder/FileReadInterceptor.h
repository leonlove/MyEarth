#ifndef FILE_READ_INTERCEPTOR_H_20E33712_3100_4974_B24D_DE3ED656109D_INCLUDE
#define FILE_READ_INTERCEPTOR_H_20E33712_3100_4974_B24D_DE3ED656109D_INCLUDE

#include <map>
#include <EventAdapter/IEventReceiver.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>
#include <OpenThreads/Mutex>
#include <EventAdapter/IEventAdapter.h>
#include <Common/Common.h>
#include <Common/Pyramid.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <OpenThreads/Block>
#include <osgDB/Callbacks>
#include <algorithm>

#include <Network/IDEUNetwork.h>

//#include "Polygon.h"
//#include "StateManager.h"
//#include "TextCenterLayouter.h"

//#include "VirtualTileReaderWriter.h"

//#include "TerrainModificationManager.h"

class FileReadInterceptor : public osgDB::ReadFileCallback
{
public:
    explicit FileReadInterceptor();
protected:
    virtual ~FileReadInterceptor(void);

public:
    bool    setLocalDatabase(const std::string &strDB);

protected:  // Method from osgDB::ReadFileCallback
    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readNode(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readImage(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);
    virtual osgDB::ReaderWriter::ReadResult readImage(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo);

protected:
    osgDB::ReaderWriter::ReadResult        readModelByID(const ID &id, const osgDB::Options *pOptions);

protected:
    OpenSP::sp<deudbProxy::IDEUDBProxy>       m_pDeuDB;

protected:
    struct MissionIncreaser
    {
        MissionIncreaser(OpenThreads::Atomic &status) : m_status(status)
        {
            ++m_status;
        }
        ~MissionIncreaser(void)
        {
            --m_status;
        }
        OpenThreads::Atomic    &m_status;
    };

    OpenThreads::Atomic                     m_MissionStatus;
};

#endif
