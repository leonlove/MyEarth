/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/DatabasePager>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osg/Geode>
#include <osg/Timer>
#include <osg/Texture>
#include <osg/Notify>

#include <OpenThreads/ScopedLock>

#include <IDProvider/Definer.h>

#include <algorithm>
#include <functional>
#include <set>
#include <iterator>

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace osgDB;
using namespace OpenThreads;

// Convert function objects that take pointer args into functions that a
// reference to an osg::ref_ptr. This is quite useful for doing STL
// operations on lists of ref_ptr. This code assumes that a function
// with an argument const Foo* should be composed into a function of
// argument type ref_ptr<Foo>&, not ref_ptr<const Foo>&. Some support
// for that should be added to make this more general.

namespace
{
template <typename U>
struct PointerTraits
{
    typedef class NullType {} PointeeType;
};

template <typename U>
struct PointerTraits<U*>
{
    typedef U PointeeType;
};

template <typename U>
struct PointerTraits<const U*>
{
    typedef U PointeeType;
};

template <typename FuncObj>
class RefPtrAdapter
    : public std::unary_function<const osg::ref_ptr<typename PointerTraits<typename FuncObj::argument_type>::PointeeType>,
                                 typename FuncObj::result_type>
{
public:
    typedef typename PointerTraits<typename FuncObj::argument_type>::PointeeType PointeeType;
    typedef osg::ref_ptr<PointeeType> RefPtrType;
    explicit RefPtrAdapter(const FuncObj& funcObj) : _func(funcObj) {}
    typename FuncObj::result_type operator()(const RefPtrType& refPtr) const
    {
        return _func(refPtr.get());
    }
protected:
        FuncObj _func;
};

template <typename FuncObj>
RefPtrAdapter<FuncObj> refPtrAdapt(const FuncObj& func)
{
    return RefPtrAdapter<FuncObj>(func);
}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CountPagedLODList
//
struct DatabasePager::DatabasePagerCompileCompletedCallback : public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
    DatabasePagerCompileCompletedCallback(osgDB::DatabasePager* pager, osgDB::DatabasePager::DatabaseRequest* databaseRequest):
        _pager(pager),
        _databaseRequest(databaseRequest) {}

    virtual bool compileCompleted(osgUtil::IncrementalCompileOperation::CompileSet* compileSet)
    {
        _pager->compileCompleted(_databaseRequest.get());
        return true;
    }

    osgDB::DatabasePager*                               _pager;
    osg::ref_ptr<osgDB::DatabasePager::DatabaseRequest> _databaseRequest;
};


void DatabasePager::compileCompleted(DatabaseRequest* databaseRequest)
{
    //OSG_NOTICE<<"DatabasePager::compileCompleted("<<databaseRequest<<")"<<std::endl;
    _dataToMergeList->add(databaseRequest);
    _dataToCompileList->remove(databaseRequest);
}

// This class is a helper for the management of SetBasedPagedLODList.
class DatabasePager::ExpirePagedLODsVisitor : public osg::NodeVisitor
{
public:
    ExpirePagedLODsVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }

    META_NodeVisitor("osgDB","ExpirePagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        _childPagedLODs.insert(&plod);
        markRequestsExpired(&plod);
        traverse(plod);
    }

    // Remove expired children from a PagedLOD. On return
    // removedChildren contains the nodes removed by the call to
    // PagedLOD::removeExpiredChildren, and the _childPagedLODs member
    // contains all the PagedLOD objects found in those children's
    // subgraphs.
    bool removeExpiredChildrenAndFindPagedLODs(osg::PagedLOD* plod, double expiryTime, unsigned int expiryFrame, osg::NodeList& removedChildren)
    {
        size_t sizeBefore = removedChildren.size();

        plod->removeExpiredChildren(expiryTime, expiryFrame, removedChildren);

        for(size_t i = sizeBefore; i<removedChildren.size(); ++i)
        {
            removedChildren[i]->accept(*this);
        }
        return sizeBefore!=removedChildren.size();
    }

    typedef std::set<osg::ref_ptr<osg::PagedLOD> > PagedLODset;
    PagedLODset         _childPagedLODs;
private:
    void markRequestsExpired(osg::PagedLOD* plod)
    {
        unsigned numFiles = plod->getNumFileNames();
        for (unsigned i = 0; i < numFiles; ++i)
        {
            DatabasePager::DatabaseRequest* request
                = dynamic_cast<DatabasePager::DatabaseRequest*>(plod->getDatabaseRequest(i).get());
            if (request)
                request->_groupExpired = true;
        }
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SetBasedPagedLODList
//
class SetBasedPagedLODList : public DatabasePager::PagedLODList
{
public:

    typedef std::set< osg::observer_ptr<osg::PagedLOD> > PagedLODs;
    PagedLODs _pagedLODs;


    virtual PagedLODList* clone() { return new SetBasedPagedLODList(); }
    virtual void clear() { _pagedLODs.clear(); }
    virtual unsigned int size() { return _pagedLODs.size(); }

    virtual void removeExpiredChildren(
        int numberChildrenToRemove, double expiryTime, unsigned int expiryFrame,
        DatabasePager::ObjectList& childrenRemoved, bool visitActive)
    {
        int leftToRemove = numberChildrenToRemove;
        for(PagedLODs::iterator itr = _pagedLODs.begin();
            itr!=_pagedLODs.end() && leftToRemove > 0;
            )
        {
            osg::ref_ptr<osg::PagedLOD> plod;
            if (itr->lock(plod))
            {
                bool plodActive = expiryFrame < plod->getFrameNumberOfLastTraversal();
                if (visitActive==plodActive) // true if (visitActive && plodActive) OR (!visitActive &&!plodActive)
                {
                    DatabasePager::ExpirePagedLODsVisitor expirePagedLODsVisitor;
                    osg::NodeList expiredChildren; // expired PagedLODs
                    expirePagedLODsVisitor.removeExpiredChildrenAndFindPagedLODs(
                        plod.get(), expiryTime, expiryFrame, expiredChildren);
                    // Clear any expired PagedLODs out of the set
                    for (DatabasePager::ExpirePagedLODsVisitor::PagedLODset::iterator
                             citr = expirePagedLODsVisitor._childPagedLODs.begin(),
                             end = expirePagedLODsVisitor._childPagedLODs.end();
                         citr != end;
                        ++citr)
                    {
                        osg::observer_ptr<osg::PagedLOD> clod(*citr);
                        // This child PagedLOD cannot be equal to the
                        // PagedLOD pointed to by itr because it must be
                        // in itr's subgraph. Therefore erasing it doesn't
                        // invalidate itr.
                        if (_pagedLODs.erase(clod) > 0)
                            leftToRemove--;
                    }
                    std::copy(expiredChildren.begin(), expiredChildren.end(), std::back_inserter(childrenRemoved));
                }
            
                // advance the iterator to the next element
                ++itr;
            }
            else
            {
                _pagedLODs.erase(itr++);
                // numberChildrenToRemove includes possibly expired
                // observer pointers.
                leftToRemove--;
            }
        }
    }

    virtual void removeNodes(osg::NodeList& nodesToRemove)
    {
        for(osg::NodeList::iterator itr = nodesToRemove.begin();
            itr != nodesToRemove.end();
            ++itr)
        {
            osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(itr->get());
            osg::observer_ptr<osg::PagedLOD> obs_ptr(plod);
            PagedLODs::iterator plod_itr = _pagedLODs.find(obs_ptr);
            if (plod_itr != _pagedLODs.end())
            {
                _pagedLODs.erase(plod_itr);
            }
        }
    }

    virtual void insertPagedLOD(const osg::observer_ptr<osg::PagedLOD>& plod)
    {
        if (_pagedLODs.count(plod)!=0)
        {
            return;
        }

        _pagedLODs.insert(plod);
    }

    virtual bool containsPagedLOD(const osg::observer_ptr<osg::PagedLOD>& plod) const
    {
        return (_pagedLODs.count(plod)!=0);
    }

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FindCompileableGLObjectsVisitor
//
class DatabasePager::FindCompileableGLObjectsVisitor : public osgUtil::StateToCompile
{
public:
    FindCompileableGLObjectsVisitor(const DatabasePager* pager):
            osgUtil::StateToCompile(osgUtil::GLObjectsVisitor::COMPILE_DISPLAY_LISTS|osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES),
            _pager(pager)
    {
        _assignPBOToImages = _pager->_assignPBOToImages;

        switch(_pager->_drawablePolicy)
        {
            case DatabasePager::DO_NOT_MODIFY_DRAWABLE_SETTINGS:
                // do nothing, leave settings as they came in from loaded database.
                // OSG_NOTICE<<"DO_NOT_MODIFY_DRAWABLE_SETTINGS"<<std::endl;
                break;
            case DatabasePager::USE_DISPLAY_LISTS:
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS;
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                break;
            case DatabasePager::USE_VERTEX_BUFFER_OBJECTS:
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                break;
            case DatabasePager::USE_VERTEX_ARRAYS:
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS;
                _mode = _mode & ~osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_DISPLAY_LISTS;
                _mode = _mode | osgUtil::GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS;
                break;
        }

        if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::Options::BUILD_KDTREES &&
            osgDB::Registry::instance()->getKdTreeBuilder())
        {
            _kdTreeBuilder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();
        }
    }

    META_NodeVisitor("osgDB","FindCompileableGLObjectsVisitor")

    bool requiresCompilation() const { return !empty(); }

    virtual void apply(osg::Geode& geode)
    {
        StateToCompile::apply(geode);

        if (_kdTreeBuilder.valid())
        {
            geode.accept(*_kdTreeBuilder);
        }
    }

    void apply(osg::Texture& texture)
    {
        StateToCompile::apply(texture);
     }

    const DatabasePager*                    _pager;
    osg::ref_ptr<osg::KdTreeBuilder>        _kdTreeBuilder;
    
protected:

    FindCompileableGLObjectsVisitor& operator = (const FindCompileableGLObjectsVisitor&) { return *this; }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SortFileRequestFunctor
//
struct DatabasePager::SortFileRequestFunctor
{
    bool operator() (const osg::ref_ptr<DatabasePager::DatabaseRequest>& lhs,const osg::ref_ptr<DatabasePager::DatabaseRequest>& rhs) const
    {
        if (lhs->_timestampLastRequest > rhs->_timestampLastRequest) return true;
        else if (lhs->_timestampLastRequest < rhs->_timestampLastRequest) return false;
        return true;
    }
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DatabaseRequest
//
void DatabasePager::DatabaseRequest::invalidate()
{
    _valid = false;
    _loadedModel = 0;
    _compileSet = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  RequestQueue
//
DatabasePager::RequestQueue::RequestQueue(DatabasePager* pager):
    _pager(pager),
    _frameNumberLastPruned(osg::UNINITIALIZED_FRAME_NUMBER)
{
}

DatabasePager::RequestQueue::~RequestQueue()
{
    for(RequestList::iterator itr = _requestList.begin();
        itr != _requestList.end();
        ++itr)
    {
        //invalidate(itr->get());
        _pager->invalidate(itr->get());
    }
}

void DatabasePager::invalidate(DatabaseRequest* dr)
{
    // OSG_NOTICE<<"DatabasePager::RequestQueue::invalidate(DatabaseRequest* dr) dr->_compileSet="<<dr->_compileSet.get()<<std::endl;
    // XXX _dr_mutex?
    osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet;

    if (dr->_compileSet.lock(compileSet) && _incrementalCompileOperation.valid())
    {
        _incrementalCompileOperation->remove(compileSet.get());
    }

    dr->invalidate();
}


bool DatabasePager::RequestQueue::pruneOldRequestsAndCheckIfEmpty()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    unsigned int frameNumber = _pager->_frameNumber;
    if (_frameNumberLastPruned != frameNumber)
    {
        for(RequestQueue::RequestList::iterator citr = _requestList.begin();
            citr != _requestList.end();
            )
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
            if ((*citr)->isRequestCurrent(frameNumber))
            {
                ++citr;
            }
            else
            {
                _pager->invalidate(citr->get());

                citr = _requestList.erase(citr);
            }
        }

        _frameNumberLastPruned = frameNumber;

    }

    return _requestList.empty();
}

bool DatabasePager::RequestQueue::empty()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    return _requestList.empty();
}

unsigned int DatabasePager::RequestQueue::size()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    return _requestList.size();
}

void DatabasePager::RequestQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
        _pager->invalidate(citr->get());
    }

    _requestList.clear();

    _frameNumberLastPruned = _pager->_frameNumber;
}


void DatabasePager::RequestQueue::add(DatabasePager::DatabaseRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    addNoLock(databaseRequest);
}

void DatabasePager::RequestQueue::remove(DatabasePager::DatabaseRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        if (citr->get()==databaseRequest)
        {
            _requestList.erase(citr);
            return;
        }
    }
}


void DatabasePager::RequestQueue::addNoLock(DatabasePager::DatabaseRequest* databaseRequest)
{
    _requestList.push_back(databaseRequest);
}

void DatabasePager::RequestQueue::swap(RequestList& requestList)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    _requestList.swap(requestList);
}


void DatabasePager::RequestQueue::takeElement(unsigned nCount, RequestList &requestList)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    unsigned n = 0u;
    const unsigned nLen = _requestList.size();
    for(unsigned i = 0u; i < nLen && !_requestList.empty(); i++)
    {
        osg::ref_ptr<DatabaseRequest> &req = _requestList.front();

        const ID &idObject = req->_loadedModel->getID();

        if(idObject.ObjectID.m_nType != PARAM_POINT_ID &&
            idObject.ObjectID.m_nType != PARAM_POINT_ID &&
            idObject.ObjectID.m_nType != PARAM_POINT_ID)
        {
            requestList.push_back(req);
            _requestList.pop_front();
            continue;
        }

        if(n < nCount)
        {
            requestList.push_back(req);
            _requestList.pop_front();
            n++;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReadQueue
//
DatabasePager::ReadQueue::ReadQueue(DatabasePager* pager) : _pager(pager)
{
    _requests.first = 0;
    _block = new osg::RefBlock;
}

void DatabasePager::ReadQueue::updateBlock()
{
    _block->set((!_requests.second.empty() || !_childrenToDeleteList.empty()) &&
        !_pager->_databasePagerThreadPaused);
}

void DatabasePager::ReadQueue::add(DatabaseRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readMutex);
    addNoLock(databaseRequest);
}

void DatabasePager::ReadQueue::addNoLock(DatabasePager::DatabaseRequest* databaseRequest)
{
    _requests.second.push_back(databaseRequest);//.push_back(databaseRequest);
    updateBlock();
}

unsigned int DatabasePager::ReadQueue::size()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readMutex);
    return _requests.second.size();
}

void DatabasePager::ReadQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readMutex);

    for(std::vector< osg::ref_ptr<DatabaseRequest> >::iterator itor = _requests.second.begin(); itor != _requests.second.end(); ++itor)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
        _pager->invalidate(itor->get());
    }
    _requests.second.clear();

    updateBlock();
}

void DatabasePager::ReadQueue::check(unsigned int nFrameNumber)
{
    if(nFrameNumber > _requests.first)
    {
        //printf("check clear size is %d\n", _requests.second.size());
        clear();
        _requests.first = nFrameNumber;
    }
}

void DatabasePager::ReadQueue::takeFirst(osg::ref_ptr<DatabaseRequest>& databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_readMutex);

    if (_requests.second.empty())   return;

    //直接找最后一个请求，如果最后一个请求都是过期的，也没必要往前再去找了
    osg::ref_ptr<DatabaseRequest> request = _requests.second.back();

    const unsigned int frameNumber = _pager->_frameNumber;

    if(request->isRequestCurrent(frameNumber))
    {
        databaseRequest = request;
        _requests.second.pop_back();
    }
    else
    {
        clear();
    }

    updateBlock();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DatabaseThread
//
DatabasePager::DatabaseThread::DatabaseThread(DatabasePager* pager):
    _done(false),
    _active(false),
    _pager(pager)
{
}


DatabasePager::DatabaseThread::~DatabaseThread()
{
    cancel();
}

int DatabasePager::DatabaseThread::cancel()
{
    int result = 0;

    if( isRunning() )
    {
        setDone(true);
        _pager->_fileRequestQueue->release();

        // then wait for the the thread to stop running.
        const unsigned nSeconds = 1000u * 15u;
        unsigned n = 0u;
        while(isRunning() && n++ < nSeconds)
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to OSG_NOTICE or std::cout destructing earlier than this destructor.
            // OSG_INFO<<"Waiting for DatabasePager::DatabaseThread to cancel"<<std::endl;
            //std::cout << "waiting for end of DatabaseThread." << std::endl;
            OpenThreads::Thread::microSleep(1000u);
            OpenThreads::Thread::YieldCurrentThread();
        }

        if(n >= nSeconds)
        {
            setCancelModeAsynchronous();
            return OpenThreads::Thread::cancel();
        }

        // _startThreadCalled = false;
    }

    return result;

}

void DatabasePager::DatabaseThread::run()
{
    osg::ref_ptr<DatabasePager::ReadQueue> read_queue;

    read_queue = _pager->_fileRequestQueue;

    do{
        _active = false;

        read_queue->block();

        if (_done)
        {
            break;
        }

        _active = true;

        //
        // delete any children if required.
        //
        if (_pager->_deleteRemovedSubgraphsInDatabaseThread/* && !(read_queue->_childrenToDeleteList.empty())*/)
        {
            ObjectList deleteList;
            {
                // Don't hold lock during destruction of deleteList
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(read_queue->_readMutex);
                if (!read_queue->_childrenToDeleteList.empty())
                {
                    deleteList.swap(read_queue->_childrenToDeleteList);
                    read_queue->updateBlock();
                }
            }
        }

        //
        // load any subgraphs that are required.
        //
        osg::ref_ptr<DatabaseRequest> databaseRequest;
        read_queue->takeFirst(databaseRequest);
        if(!databaseRequest.valid())
        {
            OpenThreads::Thread::YieldCurrentThread();
            continue;
        }

        osg::ref_ptr<Options> dr_loadOptions;
        std::string fileName;
        ID id;
        osg::ref_ptr<const osg::Referenced> creationInfo;

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
            dr_loadOptions = databaseRequest->_loadOptions;
            fileName = databaseRequest->_fileName;
            id = databaseRequest->_id;
            creationInfo = databaseRequest->_creationInfo;
        }

        // check if databaseRequest is still relevant
        if (_pager->_frameNumber > (databaseRequest->_frameNumberLastRequest + 1u))
        {
            databaseRequest = 0;
            OpenThreads::Thread::YieldCurrentThread();
            continue;
        }

        if (dr_loadOptions.valid())
        {
            dr_loadOptions = dr_loadOptions->cloneOptions();
        }
        else
        {
            dr_loadOptions = new osgDB::Options;
        }


        // load the data, note safe to write to the databaseRequest since once 
        // it is created this thread is the only one to write to the _loadedModel pointer.
        //OSG_NOTICE<<"In DatabasePager thread readNodeFile("<<databaseRequest->_fileName<<")"<<std::endl;
        //osg::Timer_t before = osg::Timer::instance()->tick();


        // assume that readNode is thread safe...
        ReaderWriter::ReadResult rr;
        try{
            rr = id.isValid() ? Registry::instance()->readNode(id, dr_loadOptions.get(), false, creationInfo) : Registry::instance()->readNode(fileName, dr_loadOptions.get(), false, creationInfo);
        }
        catch(...)
        {
            //printf("The %d exception occured in line %d.\n", nExceptionCount, __LINE__);
            databaseRequest = NULL;
            continue;
        }
        ++databaseRequest->_numOfRequests;

        if (rr.error() || rr.notEnoughMemory())
        {
            continue;
        }

        osg::ref_ptr<osg::Node> loadedModel;
        if (rr.validNode()) loadedModel = rr.getNode();

        if (!loadedModel.valid())
        {
            if(id.isValid())
            {
                _pager->_requestPrison->addToPrison(id);
            }
            OpenThreads::Thread::YieldCurrentThread();
            continue;
        }

        if (unsigned(_pager->_frameNumber) > (databaseRequest->_frameNumberLastRequest + 1u))
        {
            OpenThreads::Thread::YieldCurrentThread();
            continue;
        }

        loadedModel->getBound();

        // find all the compileable rendering objects
        bool bToBeCompiled = false;
        if(_pager->_doPreCompile)
        {
            osgUtil::IncrementalCompileOperation *pCompileOperation = _pager->_incrementalCompileOperation.get();
            if(pCompileOperation)
            {
                DatabasePager::FindCompileableGLObjectsVisitor stateToCompile(_pager);
                loadedModel->accept(stateToCompile);
                const bool bNeedToBeCompiled = pCompileOperation->requiresCompile(stateToCompile);
                if(bNeedToBeCompiled)
                {
                    osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet = new osgUtil::IncrementalCompileOperation::CompileSet(loadedModel.get());
                    compileSet->buildCompileMap(pCompileOperation->getContextSet(), stateToCompile);
                    compileSet->_compileCompletedCallback = new DatabasePagerCompileCompletedCallback(_pager, databaseRequest.get());

                    {
                        OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
                        databaseRequest->_loadedModel = loadedModel;
                        databaseRequest->_compileSet = compileSet;
                    }

                    {
                        OpenThreads::ScopedLock<OpenThreads::Mutex> listLock(_pager->_dataToCompileList->_requestMutex);
                        _pager->_dataToCompileList->addNoLock(databaseRequest.get());
                    }

                    pCompileOperation->add(compileSet.get(), false);
                    bToBeCompiled = true;
                }
            }
        }

        if(!bToBeCompiled)
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_pager->_dr_mutex);
                databaseRequest->_loadedModel = loadedModel;
                databaseRequest->_compileSet = NULL;
            }
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> listLock(_pager->_dataToMergeList->_requestMutex);
                _pager->_dataToMergeList->addNoLock(databaseRequest.get());
            }
        }

        databaseRequest = NULL;

        YieldCurrentThread();
    } while (!testCancel() && !_done);
}


DatabasePager::DatabasePager()
{
    //OSG_INFO<<"Constructing DatabasePager()"<<std::endl;
    _startThreadCalled = false;

    _done = false;
    _acceptNewRequests = true;
    _databasePagerThreadPaused = false;
    
    _numFramesActive = 0;
    _frameNumber.exchange(0);
    _requestPrison = new RequestPrison;

#if __APPLE__
    // OSX really doesn't like compiling display lists, and performs poorly when they are used,
    // so apply this hack to make up for its short comings.
    _drawablePolicy = USE_VERTEX_ARRAYS;
#else
    _drawablePolicy = DO_NOT_MODIFY_DRAWABLE_SETTINGS;
#endif    

    _assignPBOToImages = false;

    _deleteRemovedSubgraphsInDatabaseThread = true;

    _targetMaximumNumberOfPageLOD = 0;

    _doPreCompile = true;


    _fileRequestQueue = new ReadQueue(this);
    
    _dataToCompileList = new RequestQueue(this);
    _dataToMergeList = new RequestQueue(this);
    
    setUpThreads(osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint());

    _activePagedLODList = new SetBasedPagedLODList;
}

void DatabasePager::setIncrementalCompileOperation(osgUtil::IncrementalCompileOperation* ico)
{
    _incrementalCompileOperation = ico;
}

DatabasePager::~DatabasePager()
{
    // cancel the threads
    cancel();

    // destruct all the threads
    _databaseThreads.clear();

    // destruct all the queues
    _fileRequestQueue = 0;
    _dataToCompileList = 0;
    _dataToMergeList = 0;

    // remove reference to the ICO
    _incrementalCompileOperation = 0;

    //_activePagedLODList;
    //_inactivePagedLODList;
}


void DatabasePager::setUpThreads(unsigned int totalNumThreads)
{
    _databaseThreads.clear();

    for(unsigned int i=0; i<totalNumThreads; ++i)
    {
        addDatabaseThread();
    }
}

unsigned int DatabasePager::addDatabaseThread()
{
    unsigned int pos = _databaseThreads.size();
    
    DatabaseThread* thread = new DatabaseThread(this);
    //thread->setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_LOW);
    _databaseThreads.push_back(thread);
    
    if (_startThreadCalled)
    {
        thread->startThread();
    }
    
    return pos;
}

int DatabasePager::setSchedulePriority(OpenThreads::Thread::ThreadPriority priority)
{
    int result = 0;
    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        result = (*dt_itr)->setSchedulePriority(priority);
    }
    return result;
}

bool DatabasePager::isRunning() const
{
    for(DatabaseThreadList::const_iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        if ((*dt_itr)->isRunning()) return true;
    }
    
    return false;
}

int DatabasePager::cancel()
{
    int result = 0;

    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        (*dt_itr)->setDone(true);
    }

    // release the queue blocks in case they are holding up thread cancellation.
    _fileRequestQueue->release();

    for(DatabaseThreadList::iterator dt_itr = _databaseThreads.begin();
        dt_itr != _databaseThreads.end();
        ++dt_itr)
    {
        (*dt_itr)->cancel();
    }

    _done = true;
    _startThreadCalled = false;

    return result;
}

void DatabasePager::clear()
{
    _fileRequestQueue->clear();

    _dataToCompileList->clear();
    _dataToMergeList->clear();

    // note, no need to use a mutex as the list is only accessed from the update thread.
    _activePagedLODList->clear();

    // ??
    // _activeGraphicsContexts
}


bool DatabasePager::getRequestsInProgress() const
{
    if (getFileRequestListSize()>0) return true;

    if (getDataToCompileListSize()>0) 
    {
        return true;
    }

    if (getDataToMergeListSize()>0) return true;

    for(DatabaseThreadList::const_iterator itr = _databaseThreads.begin();
        itr != _databaseThreads.end();
        ++itr)
    {
        if ((*itr)->getActive()) return true;
    }
    return false;
}


void DatabasePager::requestNodeFile(const std::string& fileName, osg::NodePath& nodePath,
                                    const osg::FrameStamp* framestamp,
                                    osg::ref_ptr<osg::Referenced>& databaseRequestRef,
                                    const osg::Referenced* options,
                                    const osg::Referenced *_childCreationInfo)
{
    osgDB::Options* loadOptions = dynamic_cast<osgDB::Options*>(const_cast<osg::Referenced*>(options));
    if (!loadOptions)
    {
       loadOptions = Registry::instance()->getOptions();
    }


    if (!_acceptNewRequests) return;


    if (nodePath.empty())
    {
        return;
    }

    osg::Group* group = nodePath.back()->asGroup();
    if (!group)
    {
        return;
    }

    double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    unsigned int frameNumber = framestamp?framestamp->getFrameNumber():static_cast<unsigned int>(_frameNumber);

    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    if (databaseRequestRef.valid())
    {
        DatabaseRequest* databaseRequest = dynamic_cast<DatabaseRequest*>(databaseRequestRef.get());
        bool requeue = false;
        if (databaseRequest)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_dr_mutex);
            if (!(databaseRequest->valid()))
            {
                databaseRequest = 0;
            }
            else
            {
                databaseRequest->_valid = true;
                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_creationInfo = _childCreationInfo;

                foundEntry = true;

                if (databaseRequestRef->referenceCount()==1)
                {
                    databaseRequest->_frameNumberLastRequest = frameNumber;
                    databaseRequest->_timestampLastRequest = timestamp;
                    databaseRequest->_group = group;
                    databaseRequest->_loadOptions = loadOptions;
                    requeue = true;
                }
            
            }
        }
        if (requeue)
            _fileRequestQueue->add(databaseRequest);
    }

    if (!foundEntry)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_readMutex);
        
        if (!databaseRequestRef.valid() || databaseRequestRef->referenceCount()==1)
        {
            osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

            databaseRequestRef = databaseRequest.get();

            databaseRequest->_valid = true;
            databaseRequest->_fileName = fileName;
            databaseRequest->_frameNumberLastRequest = frameNumber;
            databaseRequest->_timestampLastRequest = timestamp;
            databaseRequest->_group = group;
            databaseRequest->_loadOptions = loadOptions;
            databaseRequest->_creationInfo = _childCreationInfo;

            _fileRequestQueue->addNoLock(databaseRequest.get());
        }
    }
    
    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);
        
        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;
            if (_databaseThreads.empty()) 
            {
                setUpThreads(osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint());
            }

            for(DatabaseThreadList::const_iterator dt_itr = _databaseThreads.begin();
                dt_itr != _databaseThreads.end();
                ++dt_itr)
            {
                (*dt_itr)->startThread();
            }
        }
    }

#ifdef WITH_REQUESTNODEFILE_TIMING
    totalTime += osg::Timer::instance()->delta_m(start_tick, osg::Timer::instance()->tick());
#endif
}

void DatabasePager::requestNodeFile(const ID& id, osg::NodePath& nodePath,
                                    const osg::FrameStamp* framestamp,
                                    osg::ref_ptr<osg::Referenced>& databaseRequestRef,
                                    const osg::Referenced* options,
                                    const osg::Referenced *_childCreationInfo)
{
    osgDB::Options* loadOptions = dynamic_cast<osgDB::Options*>(const_cast<osg::Referenced*>(options));
    if (!loadOptions)
    {
        loadOptions = Registry::instance()->getOptions();
    }


    if (!_acceptNewRequests) return;

    if(_requestPrison->checkIfInPrison(id))
    {
        return;
    }

    if (nodePath.empty())
    {
        return;
    }

    osg::Group* group = nodePath.back()->asGroup();
    if (!group)
    {
        return;
    }

    const double timestamp = framestamp?framestamp->getReferenceTime():0.0;
    const unsigned int frameNumber = framestamp?framestamp->getFrameNumber():static_cast<unsigned int>(_frameNumber);

    _fileRequestQueue->check(frameNumber);

    // search to see if filename already exist in the file loaded list.
    bool foundEntry = false;

    if (databaseRequestRef.valid())
    {
        DatabaseRequest* databaseRequest = dynamic_cast<DatabaseRequest*>(databaseRequestRef.get());
        bool requeue = false;
        if (databaseRequest)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> drLock(_dr_mutex);
            if (!(databaseRequest->valid()))
            {
                databaseRequest = 0;
                databaseRequestRef = NULL;
            }
            else
            {
                databaseRequest->_valid = true;
                databaseRequest->_frameNumberLastRequest = frameNumber;
                databaseRequest->_timestampLastRequest = timestamp;
                databaseRequest->_creationInfo = _childCreationInfo;

                foundEntry = true;

                if (databaseRequestRef->referenceCount() == 1u)
                {
                    databaseRequest->_frameNumberLastRequest = frameNumber;
                    databaseRequest->_timestampLastRequest = timestamp;
                    databaseRequest->_group = group;
                    databaseRequest->_loadOptions = loadOptions;
                    requeue = true;
                }

            }
        }
        if (requeue)
            _fileRequestQueue->add(databaseRequest);
    }

    if (!foundEntry)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_readMutex);

        if (!databaseRequestRef.valid() || databaseRequestRef->referenceCount()==1)
        {
            osg::ref_ptr<DatabaseRequest> databaseRequest = new DatabaseRequest;

            databaseRequestRef = databaseRequest.get();

            databaseRequest->_valid = true;
            databaseRequest->_id = id;
            databaseRequest->_frameNumberLastRequest = frameNumber;
            databaseRequest->_timestampLastRequest = timestamp;
            databaseRequest->_group = group;
            databaseRequest->_loadOptions = loadOptions;
            databaseRequest->_creationInfo = _childCreationInfo;

            _fileRequestQueue->addNoLock(databaseRequest.get());
        }
    }

    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);

        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;

            if (_databaseThreads.empty()) 
            {
                setUpThreads(osg::DisplaySettings::instance()->getNumOfDatabaseThreadsHint());
            }

            for(DatabaseThreadList::const_iterator dt_itr = _databaseThreads.begin();
                dt_itr != _databaseThreads.end();
                ++dt_itr)
            {
                (*dt_itr)->startThread();
            }
        }
    }

#ifdef WITH_REQUESTNODEFILE_TIMING
    totalTime += osg::Timer::instance()->delta_m(start_tick, osg::Timer::instance()->tick());
#endif
}

void DatabasePager::signalBeginFrame(const osg::FrameStamp* framestamp)
{
    if (framestamp)
    {
        _dataToCompileList->pruneOldRequestsAndCheckIfEmpty();

        _frameNumber.exchange(framestamp->getFrameNumber());

    }
}

void DatabasePager::signalEndFrame()
{
}

void DatabasePager::setDatabasePagerThreadPause(bool pause)
{
    if (_databasePagerThreadPaused == pause) return;
    
    _databasePagerThreadPaused = pause;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_readMutex);
        _fileRequestQueue->updateBlock();
    }
}


bool DatabasePager::requiresUpdateSceneGraph() const
{
    return !(_dataToMergeList->empty());
}

void DatabasePager::updateSceneGraph(const osg::FrameStamp& frameStamp)
{

#define UPDATE_TIMING 0
#if UPDATE_TIMING
    osg::ElapsedTime timer;
    double timeFor_removeExpiredSubgraphs, timeFor_addLoadedDataToSceneGraph;
#endif

    {
        removeExpiredSubgraphs(frameStamp);

#if UPDATE_TIMING
        timeFor_removeExpiredSubgraphs = timer.elapsedTime_m();
#endif

        addLoadedDataToSceneGraph(frameStamp);

#if UPDATE_TIMING
        timeFor_addLoadedDataToSceneGraph = timer.elapsedTime_m() - timeFor_removeExpiredSubgraphs;
#endif

    }

#if UPDATE_TIMING
    double elapsedTime = timer.elapsedTime_m();
    if (elapsedTime>0.4)
    {
        OSG_NOTICE<<"DatabasePager::updateSceneGraph() total time = "<<elapsedTime<<"ms"<<std::endl;
        OSG_NOTICE<<"   timeFor_removeExpiredSubgraphs    = "<<timeFor_removeExpiredSubgraphs<<"ms"<<std::endl;
        OSG_NOTICE<<"   timeFor_addLoadedDataToSceneGraph = "<<timeFor_addLoadedDataToSceneGraph<<"ms"<<std::endl;
        OSG_NOTICE<<"   _activePagedLODList.size()        = "<<_activePagedLODList->size()<<std::endl;
        OSG_NOTICE<<"   _inactivePagedLODList.size()      = "<<_inactivePagedLODList->size()<<std::endl;
        OSG_NOTICE<<"   total                             = "<<_activePagedLODList->size() + _inactivePagedLODList->size()<<std::endl;
    }
#endif
}


void DatabasePager::addLoadedDataToSceneGraph(const osg::FrameStamp &frameStamp)
{
    if(_dataToMergeList->empty())    return;

    const double timeStamp = frameStamp.getReferenceTime();
    const unsigned int frameNumber = frameStamp.getFrameNumber();

    RequestQueue::RequestList localFileLoadedList;

    // get the data from the _dataToMergeList, leaving it empty via a std::vector<>.swap.
    _dataToMergeList->swap(localFileLoadedList);

    // add the loaded data into the scene graph.
    for(RequestQueue::RequestList::iterator itr=localFileLoadedList.begin();
        itr!=localFileLoadedList.end();
        ++itr)
    {
        DatabaseRequest *pOneRequest = itr->get();
        if(!pOneRequest)    continue;

        if(frameNumber > (pOneRequest->_frameNumberLastRequest + 1u))
        {
            continue;
        }

        osg::Node *pLoadedNode = pOneRequest->_loadedModel;
        if(!pLoadedNode)
        {
            //osg::notify(osg::ALWAYS) << "model has been loaded successful, but compiled failed." << std::endl;
            continue;
        }

        // No need to take _dr_mutex. The pager threads are done with
        // the request; the cull traversal -- which might redispatch
        // the request -- can't run at the sametime as this update traversal.
        osg::ref_ptr<osg::Group> pGroup;
        if (!pOneRequest->_groupExpired && pOneRequest->_group.lock(pGroup))
        {
            if (osgDB::Registry::instance()->getSharedStateManager())
            {
                osgDB::Registry::instance()->getSharedStateManager()->share(pLoadedNode);
            }

            osg::PagedLOD *pPagedLod = dynamic_cast<osg::PagedLOD*>(pGroup.get());
            if(pPagedLod)
            {
                pPagedLod->setTimeStamp(pPagedLod->getNumChildren(), timeStamp);
                pPagedLod->setFrameNumber(pPagedLod->getNumChildren(), frameNumber);
                pPagedLod->getDatabaseRequest(pPagedLod->getNumChildren()) = 0;
            }

            pGroup->addChild(pLoadedNode);

            // Check if parent plod was already registered if not start visitor from parent
            if(pPagedLod && !_activePagedLODList->containsPagedLOD(pPagedLod))
            {
                registerPagedLODs(pPagedLod, frameNumber);
            } 
            else 
            {
                registerPagedLODs(pLoadedNode, frameNumber);
            }
        }

        // reset the loadedModel pointer
        pOneRequest->_loadedModel = 0;
    }
}



void DatabasePager::removeExpiredSubgraphs(const osg::FrameStamp& frameStamp)
{
    static double s_total_iter_stage_a = 0.0;
    static double s_total_time_stage_a = 0.0;
    static double s_total_max_stage_a = 0.0;
    
    static double s_total_iter_stage_b = 0.0;
    static double s_total_time_stage_b = 0.0;
    static double s_total_max_stage_b = 0.0;

    static double s_total_iter_stage_c = 0.0;
    static double s_total_time_stage_c = 0.0;
    static double s_total_max_stage_c = 0.0;

    if (frameStamp.getFrameNumber() == 0u)
    {
        // No need to remove anything on first frame.
        return;
    }


    const osg::Timer_t startTick = osg::Timer::instance()->tick();

    // numPagedLODs >= actual number of PagedLODs. There can be
    // invalid observer pointers in _activePagedLODList.
    const unsigned int nNumPagedLODs = _activePagedLODList->size();

    const osg::Timer_t end_a_Tick = osg::Timer::instance()->tick();
    const double time_a = osg::Timer::instance()->delta_m(startTick,end_a_Tick);

    s_total_iter_stage_a += 1.0;
    s_total_time_stage_a += time_a;
    if (s_total_max_stage_a<time_a) s_total_max_stage_a = time_a;

    if (nNumPagedLODs <= _targetMaximumNumberOfPageLOD)
    {
        // nothing to do
        return;
    }

    unsigned nNumToPrune = nNumPagedLODs - _targetMaximumNumberOfPageLOD;

    ObjectList childrenRemoved;

    const double dblExpiryTime = frameStamp.getReferenceTime() - 0.1;
    const unsigned int uExpiryFrame = frameStamp.getFrameNumber() - 1u;

    // First traverse inactive PagedLODs, as their children will
    // certainly have expired. Then traverse active nodes if we still
    // need to prune.
    if (nNumToPrune > 0)
    {
        _activePagedLODList->removeExpiredChildren(nNumToPrune, dblExpiryTime, uExpiryFrame, childrenRemoved, false);
    }

    nNumToPrune = 0u;
    if(_activePagedLODList->size() > _targetMaximumNumberOfPageLOD)
    {
        nNumToPrune = _activePagedLODList->size() - _targetMaximumNumberOfPageLOD;
    }
    if (nNumToPrune > 0)
    {
        _activePagedLODList->removeExpiredChildren(nNumToPrune, dblExpiryTime, uExpiryFrame, childrenRemoved, true);
    }

    const osg::Timer_t end_b_Tick = osg::Timer::instance()->tick();
    const double time_b = osg::Timer::instance()->delta_m(end_a_Tick,end_b_Tick);

    s_total_iter_stage_b += 1.0;
    s_total_time_stage_b += time_b;
    if (s_total_max_stage_b < time_b)
    {
        s_total_max_stage_b = time_b;
    }

    if (!childrenRemoved.empty())
    { 
        // pass the objects across to the database pager delete list
        if (_deleteRemovedSubgraphsInDatabaseThread)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_fileRequestQueue->_readMutex);
            // splice transfers the entire list in constant time.
            _fileRequestQueue->_childrenToDeleteList.splice(
                _fileRequestQueue->_childrenToDeleteList.end(),
                childrenRemoved);
            _fileRequestQueue->updateBlock();
        }
        else
        {
            childrenRemoved.clear();
        }
    }

    const osg::Timer_t end_c_Tick = osg::Timer::instance()->tick();
    const double time_c = osg::Timer::instance()->delta_m(end_b_Tick,end_c_Tick);

    s_total_iter_stage_c += 1.0;
    s_total_time_stage_c += time_c;
    if (s_total_max_stage_c < time_c)
    {
        s_total_max_stage_c = time_c;
    }
}

class DatabasePager::FindPagedLODsVisitor : public osg::NodeVisitor
{
public:

    FindPagedLODsVisitor(DatabasePager::PagedLODList& pagedLODList, unsigned int frameNumber):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _activePagedLODList(pagedLODList),
        _frameNumber(frameNumber)
    {
    }

    META_NodeVisitor("osgDB","FindPagedLODsVisitor")

    virtual void apply(osg::PagedLOD& plod)
    {
        plod.setFrameNumberOfLastTraversal(_frameNumber);

        osg::observer_ptr<osg::PagedLOD> obs_ptr(&plod);
        _activePagedLODList.insertPagedLOD(obs_ptr);

        traverse(plod);
    }

    DatabasePager::PagedLODList& _activePagedLODList;
    unsigned int _frameNumber;

protected:

    FindPagedLODsVisitor& operator = (const FindPagedLODsVisitor&) { return *this; }
};


void DatabasePager::registerPagedLODs(osg::Node* subgraph, unsigned int frameNumber)
{
    if (!subgraph) return;

    FindPagedLODsVisitor fplv(*_activePagedLODList, frameNumber);
    subgraph->accept(fplv);
}
