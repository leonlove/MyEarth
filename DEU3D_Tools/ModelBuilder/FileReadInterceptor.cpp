#include "FileReadInterceptor.h"

#include <osgDB/Registry>
#include <osgDB/Options>
#include <osgDB/FileNameUtils>
#include <osg/ValueObject>
#include <osgDB/WriteFile>
#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Optimizer>
#include <osg/Timer>
#include <osg/PagedLOD>
#include <osg/SharedObjectPool>

#include <istream>
#include <strstream>
#include <stdlib.h>
#include <time.h>

#include <Common/Common.h>
#include <Common/deuImage.h>
#include <EventAdapter/EventAdapter.h>
#include <EventAdapter/IEventFilter.h>
#include <EventAdapter/IEventObject.h>

#include <ParameterSys/Parameter.h>
#include <ParameterSys/Detail.h>
#include <IDProvider/Definer.h>

#define DEBUG_LOG 1


FileReadInterceptor::FileReadInterceptor()
{
}


FileReadInterceptor::~FileReadInterceptor(void)
{
    while((unsigned)m_MissionStatus > 0u)
    {
        OpenThreads::Thread::YieldCurrentThread();
        OpenThreads::Thread::microSleep(1000);
    }
}

bool FileReadInterceptor::setLocalDatabase(const std::string &strDB)
{
    OpenSP::sp<deudbProxy::IDEUDBProxy> pDeuDB = deudbProxy::createDEUDBProxy();
    if(!pDeuDB->openDB(strDB))
    {
        return false;
    }
    m_pDeuDB = pDeuDB;
    return true;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readNode(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

    MissionIncreaser Increaser(m_MissionStatus);

    // 获取文件的后缀名
    const std::string strExtension = osgDB::getFileExtension(strFileName);
    if(!strExtension.empty())
    {
        rr = osgDB::Registry::instance()->readNodeImplementation(strFileName, pOptions);
        return rr;
    }

    const ID id = ID::genIDfromString(strFileName);
    if(id.isValid())
    {
        return readNode(id, pOptions, pCreationInfo);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readNode(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);

    MissionIncreaser Increaser(m_MissionStatus);

    if(!id.isValid())   return rr;

    if(id.ObjectID.m_nType == MODEL_ID || id.ObjectID.m_nType == SHARE_MODEL_ID)
    {
        rr = readModelByID(id, pOptions);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readModelByID(const ID &id, const osgDB::Options *pOptions)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);
    if(!m_pDeuDB.valid())
    {
        return rr;
    }

    void *pBuffer = NULL;
    unsigned nLength = 0u;

    const bool bFromLocal = m_pDeuDB->readBlock(id, pBuffer, nLength);
    if(pBuffer && nLength > 0u)
    {
        const std::string strIVE = "ive";
        osg::ref_ptr<osgDB::ReaderWriter> pIveRW = osgDB::Registry::instance()->getReaderWriterForExtension(strIVE);
        if(pIveRW.valid())
        {
            std::strstream ss((char *)pBuffer, nLength);
            rr = pIveRW->readNode(ss, pOptions);
        }
        deudbProxy::freeMemory(pBuffer);
    }

    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readImage(const std::string &strFileName, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);
    return rr;
}


osgDB::ReaderWriter::ReadResult FileReadInterceptor::readImage(const ID &id, const osgDB::Options *pOptions, const osg::Referenced *pCreationInfo)
{
    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);
    return rr;
}

