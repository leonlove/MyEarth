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

#ifndef OSP_SP
#define OSP_SP 1

#define OSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION

namespace OpenSP {

template<typename T> class op;

/** Smart pointer for handling Ref counted objects.*/
template<class T>
class sp
{
    public:
		typedef T element_type;

		sp() : _ptr(0) {}
		sp(T* ptr) : _ptr(ptr) { if (_ptr) _ptr->ref(); }
		sp(const sp& rp) : _ptr(rp._ptr) { if (_ptr) _ptr->ref(); }
		template<class Other> sp(const sp<Other>& rp) : _ptr(rp._ptr) { if (_ptr) _ptr->ref(); }
		sp(op<T>& optr) : _ptr(0) { optr.lock(*this); }
		~sp()
		{
			if (_ptr) _ptr->unref();
			_ptr = 0; 
		}

        sp& operator = (const sp& rp)
        {
            assign(rp);
            return *this;
        }

        template<class Other> sp& operator = (const sp<Other>& rp)
        {
            assign(rp);
            return *this;
        }

        inline sp& operator = (T* ptr)
        {
            if (_ptr==ptr) return *this;
            T* tmp_ptr = _ptr;
            _ptr = ptr;
            if (_ptr) _ptr->ref();
            // unref second to prevent any deletion of any object which might
            // be Ref by the other object. i.e rp is child of the
            // original _ptr.
            if (tmp_ptr) tmp_ptr->unref();
            return *this;
        }

#ifdef OSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION
        // implicit output conversion
        operator T*() const { return _ptr; }
#else
        // comparison operators for sp.
        bool operator == (const sp& rp) const { return (_ptr==rp._ptr); }
        bool operator == (const T* ptr) const { return (_ptr==ptr); }
        friend bool operator == (const T* ptr, const sp& rp) { return (ptr==rp._ptr); }

        bool operator != (const sp& rp) const { return (_ptr!=rp._ptr); }
        bool operator != (const T* ptr) const { return (_ptr!=ptr); }
        friend bool operator != (const T* ptr, const sp& rp) { return (ptr!=rp._ptr); }
 
        bool operator < (const sp& rp) const { return (_ptr<rp._ptr); }


    // follows is an implmentation of the "safe bool idiom", details can be found at:
    //   http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Safe_bool
    //   http://lists.boost.org/Archives/boost/2003/09/52856.php

    private:
        typedef T* sp::*unspecified_bool_type;

    public:
        // safe bool conversion
        operator unspecified_bool_type() const { return valid()? &sp::_ptr : 0; }
#endif

        T& operator*() const { return *_ptr; }
        T* operator->() const { return _ptr; }
        T* get() const { return _ptr; }

        bool operator!() const   { return _ptr==0; } // not required
        bool valid() const       { return _ptr!=0; }

        T* release() { T* tmp=_ptr; if (_ptr) _ptr->unref_nodelete(); _ptr=0; return tmp; }

        void swap(sp& rp) { T* tmp=_ptr; _ptr=rp._ptr; rp._ptr=tmp; }

//    private:

        template<class Other> void assign(const sp<Other>& rp)
        {
            if (_ptr==rp._ptr) return;
            T* tmp_ptr = _ptr;
            _ptr = rp._ptr;
            if (_ptr) _ptr->ref();
            // unref second to prevent any deletion of any object which might
            // be Ref by the other object. i.e rp is child of the
            // original _ptr.
            if (tmp_ptr) tmp_ptr->unref();
        }

        template<class Other> friend class sp;

        T* _ptr;
};

template<class T> inline
void swap(sp<T>& rp1, sp<T>& rp2) { rp1.swap(rp2); }

template<class T> inline
T* get_pointer(const sp<T>& rp) { return rp.get(); }

template<class T, class Y> inline
sp<T> static_pointer_cast(const sp<Y>& rp) { return static_cast<T*>(rp.get()); }

template<class T, class Y> inline
sp<T> dynamic_pointer_cast(const sp<Y>& rp) { return dynamic_cast<T*>(rp.get()); }

template<class T, class Y> inline
sp<T> const_pointer_cast(const sp<Y>& rp) { return const_cast<T*>(rp.get()); }

}

#endif
