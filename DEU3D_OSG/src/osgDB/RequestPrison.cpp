#include <osgDB/RequestPrison>
#include <osg/Timer>

using namespace osgDB;

bool RequestPrison::checkIfInPrison(const ID &id)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPrisonerReport);
    PrisonerReport::const_iterator itorFind = m_mapPrisonerReport.find(id);
    if(itorFind != m_mapPrisonerReport.end())
    {
        const double dblCurTime = osg::Timer::instance()->time_s();
        const double dblPrisonTime = dblCurTime - itorFind->second;
        if(dblPrisonTime >= m_dblPrisonTime)
        {
            m_mapPrisonerReport.erase(itorFind);
            return false;
        }
        return true;
    }
    return false;
}


bool RequestPrison::addToPrison(const ID &id)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPrisonerReport);
    m_mapPrisonerReport[id] = osg::Timer::instance()->time_s();

    //releasePrisoner();
    return true;
}


unsigned RequestPrison::getItemsCount(void) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxPrisonerReport);
    return m_mapPrisonerReport.size();
}


