//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_SYSTEM_UNKNOWN_H
#define f_VD2_SYSTEM_UNKNOWN_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>

///////////////////////////////////////////////////////////////////////////
//	IVDUnknown
///	Base interface for runtime type discovery.
class IVDUnknown {
public:
	/// Attempt to cast to another type. Returns NULL if interface is unsupported.
	virtual void *AsInterface(uint32 id) = 0;

	inline const void *AsInterface(uint32 id) const {
		return const_cast<IVDUnknown *>(this)->AsInterface(id);
	}
};

///////////////////////////////////////////////////////////////////////////
//	IVDUnknown
///	Base interface for runtime type discovery with reference counting.
class IVDRefUnknown : public IVDUnknown {
public:
	virtual int AddRef() = 0;	///< Add strong reference to object. Returns new reference count (debug builds only).
	virtual int Release() = 0;	///< Remove strong refence from object, and destroy it if the refcount drops to zero. Returns zero if object was destroyed.
};

template<class T>
inline uint32 vdpoly_id_from_ptr(T *p) {
	return T::kTypeID;
}

///////////////////////////////////////////////////////////////////////////
//	vdpoly_cast
///	Performs a runtime polymorphic cast on an IUnknown-based object.
///
///	\param	pUnk	Pointer to cast. May be NULL.
///
///	Attempts to cast a pointer to a different type using the
///	\c AsInterface() method. The destination type must support the
/// \c kTypeID convention for returning the type ID.
/// 
template<class T>
T vdpoly_cast(IVDUnknown *pUnk) {
	return pUnk ? (T)pUnk->AsInterface(vdpoly_id_from_ptr(T(NULL))) : NULL;
}

#endif
