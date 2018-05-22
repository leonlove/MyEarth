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

#ifndef OSP_REFERENCED
#define OSP_REFERENCED 1

#include "Export.h"

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>
#include <OpenThreads/Atomic>

#if !defined(_OPENTHREADS_ATOMIC_USE_MUTEX)
# define _OSG_REFERENCED_USE_ATOMIC_OPERATIONS
#endif

namespace OpenSP {

class Observer;
class ObserverSet;

/** Base class for providing reference counted objects.*/
class OSP_EXPORT Ref
{
    public:

        Ref(); 
        
        explicit Ref(bool threadSafeRefUnref); 

        Ref(const Ref&);

        inline Ref& operator = (const Ref&) { return *this; }

        /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
        virtual void setThreadSafeRefUnref(bool threadSafe);

        /** Get whether a mutex is used to ensure ref() and unref() are thread safe.*/
		bool getThreadSafeRefUnref() const;

        /** Increment the reference count by one, indicating that 
            this object has another pointer which is referencing it.*/
        inline int ref() const;
        
        /** Decrement the reference count by one, indicating that 
            a pointer to this object is referencing it.  If the
            reference count goes to zero, it is assumed that this object
            is no longer referenced and is automatically deleted.*/
        inline int unref() const;
        
        /** Decrement the reference count by one, indicating that 
            a pointer to this object is referencing it.  However, do
            not delete it, even if ref count goes to 0.  Warning, unref_nodelete() 
            should only be called if the user knows exactly who will
            be responsible for, one should prefer unref() over unref_nodelete() 
            as the latter can lead to memory leaks.*/
        int unref_nodelete() const;
        
        /** Return the number of pointers currently referencing this object. */
        inline int referenceCount() const { return _refCount; }


        /** Get the ObserverSet if one is attached, otherwise return NULL.*/
        ObserverSet* getObserverSet() const
        {
			return static_cast<ObserverSet*>(_observerSet.get());
        }

        /** Get the ObserverSet if one is attached, otherwise create an ObserverSet, attach it, then return this newly created ObserverSet.*/
        ObserverSet* getOrCreateObserverSet() const;

        /** Add a Observer that is observing this object, notify the Observer when this object gets deleted.*/
        void addObserver(Observer* observer) const;

        /** Remove Observer that is observing this object.*/
        void removeObserver(Observer* observer) const;

    public:

        /** Set whether reference counting should use a mutex for thread safe reference counting.*/
        static void setThreadSafeReferenceCounting(bool enableThreadSafeReferenceCounting);
        
        /** Get whether reference counting is active.*/
        static bool getThreadSafeReferenceCounting();

       

        virtual void signalObserversAndDelete(bool signalDelete, bool doDelete) const;

        mutable OpenThreads::AtomicPtr  _observerSet;

        mutable OpenThreads::Atomic     _refCount;

    protected:    
        virtual ~Ref();
};

inline int Ref::ref() const
{
    return ++_refCount;
}

inline int Ref::unref() const
{
    int newRef = --_refCount;
    bool needDelete = (newRef == 0);

    if (needDelete)
    {
        signalObserversAndDelete(true,true);
    }
    return newRef;
}

}

#endif
