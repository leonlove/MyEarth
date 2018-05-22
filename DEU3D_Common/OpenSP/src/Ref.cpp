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
#include <stdlib.h>

#include <typeinfo>
#include <memory>
#include <set>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#include <Ref.h>
#include <Observer.h>
//#include <OpenSP\DeleteHandler.h>

namespace OpenSP
{

//DeleteHandler *Ref::ms_pDeleteHandler = NULL;

//class initOpenSP
//{
//public:
//    initOpenSP(void)
//    {
//        Ref::ms_pDeleteHandler = new DeleteHandler;
//    }
//}__creator;


void Ref::setThreadSafeReferenceCounting(bool enableThreadSafeReferenceCounting)
{
}

bool Ref::getThreadSafeReferenceCounting()
{
    return true;
}

#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
OpenThreads::Mutex& getNumObjectMutex()
{
    static OpenThreads::Mutex s_numObjectMutex;
    return s_numObjectMutex;
}
static int s_numObjects = 0;
#endif

Ref::Ref():
    _observerSet(0),
    _refCount(0)
{
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        printf("Object created, total num=%d\n",s_numObjects);
    }
#endif
}

Ref::Ref(bool threadSafeRefUnref):
    _observerSet(0),
    _refCount(0)
{
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        printf("Object created, total num=%d\n",s_numObjects);
    }
#endif
}

Ref::Ref(const Ref&):
    _observerSet(0),
    _refCount(0)
{
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        printf("Object created, total num=%d\n",s_numObjects);
    }
#endif
}

Ref::~Ref()
{
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        --s_numObjects;
        printf("Object created, total num=%d\n",s_numObjects);
    }
#endif

    // signal observers that we are being deleted.
    signalObserversAndDelete(true, false);

    // delete the ObserverSet
    if (_observerSet.get()) static_cast<ObserverSet*>(_observerSet.get())->unref();
}

bool Ref::getThreadSafeRefUnref() const
{
    return true;
}


ObserverSet* Ref::getOrCreateObserverSet() const
{
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());
    while (0 == observerSet)
    {
        ObserverSet* newObserverSet = new ObserverSet(this);
        newObserverSet->ref();

        if (!_observerSet.assign(newObserverSet, 0))
        {
            newObserverSet->unref();
        }

        observerSet = static_cast<ObserverSet*>(_observerSet.get());
    }
    return observerSet;
}

void Ref::addObserver(Observer* observer) const
{
    getOrCreateObserverSet()->addObserver(observer);
}

void Ref::removeObserver(Observer* observer) const
{
    getOrCreateObserverSet()->removeObserver(observer);
}

void Ref::signalObserversAndDelete(bool signalDelete, bool doDelete) const
{
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());

    if (observerSet && signalDelete)
    {
        observerSet->signalObjectDeleted(const_cast<Ref*>(this));
    }

    if (doDelete)
    {
        delete this;
        //ms_pDeleteHandler->signalObjectDeleted(const_cast<Ref*>(this));
    }
}


void Ref::setThreadSafeRefUnref(bool threadSafe)
{
}

int Ref::unref_nodelete() const
{
    return --_refCount;
}

}