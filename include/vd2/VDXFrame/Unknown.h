//	VDXFrame - Helper library for VirtualDub plugins
//	Copyright (C) 2008 Avery Lee
//
//	The plugin headers in the VirtualDub plugin SDK are licensed differently
//	differently than VirtualDub and the Plugin SDK themselves.  This
//	particular file is thus licensed as follows (the "zlib" license):
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

#ifndef f_VD2_VDXFRAME_UNKNOWN_H
#define f_VD2_VDXFRAME_UNKNOWN_H

#include <vd2/plugin/vdplugin.h>

extern "C" long _InterlockedExchangeAdd(volatile long *p, long v);
#pragma intrinsic(_InterlockedExchangeAdd)

template<class T> class vdxunknown : public T {
public:
	vdxunknown() : mRefCount(0) {}
	vdxunknown(const vdxunknown<T>& src) : mRefCount(0) {}		// do not copy the refcount
	virtual ~vdxunknown() {}

	vdxunknown<T>& operator=(const vdxunknown<T>&) {}			// do not copy the refcount

	virtual int VDXAPIENTRY AddRef() {
		return _InterlockedExchangeAdd(&mRefCount, 1) + 1;
	}

	virtual int VDXAPIENTRY Release() {
		long rc = _InterlockedExchangeAdd(&mRefCount, -1) - 1;
		if (!mRefCount) {
			mRefCount = 1;
			delete this;
			return 0;
		}

		return rc;
	}

	virtual void *VDXAPIENTRY AsInterface(uint32 iid) {
		if (iid == T::kIID)
			return static_cast<T *>(this);

		if (iid == IVDXUnknown::kIID)
			return static_cast<IVDXUnknown *>(this);

		return NULL;
	}

protected:
	volatile long	mRefCount;
};

template<class T1, class T2> class vdxunknown2 : public T1, public T2 {
public:
	vdxunknown2() : mRefCount(0) {}
	vdxunknown2(const vdxunknown2& src) : mRefCount(0) {}		// do not copy the refcount
	virtual ~vdxunknown2() {}

	vdxunknown2& operator=(const vdxunknown2&) {}				// do not copy the refcount

	virtual int VDXAPIENTRY AddRef() {
		return _InterlockedExchangeAdd(&mRefCount, 1) + 1;
	}

	virtual int VDXAPIENTRY Release() {
		long rc = _InterlockedExchangeAdd(&mRefCount, -1) - 1;
		if (!mRefCount) {
			mRefCount = 1;
			delete this;
			return 0;
		}

		return rc;
	}

	virtual void *VDXAPIENTRY AsInterface(uint32 iid) {
		if (iid == T1::kIID)
			return static_cast<T1 *>(this);

		if (iid == T2::kIID)
			return static_cast<T2 *>(this);

		if (iid == IVDXUnknown::kIID)
			return static_cast<IVDXUnknown *>(static_cast<T1 *>(this));

		return NULL;
	}

protected:
	volatile long	mRefCount;
};

#endif
