//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2006 Avery Lee, All Rights Reserved.
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

#include "stdafx.h"
#include <vd2/system/vdtypes.h>
#include <vd2/system/event.h>

///////////////////////////////////////////////////////////////////////////////

VDDelegate::VDDelegate() {
	mpPrev = mpNext = this;
}

VDDelegate::~VDDelegate() {
	VDDelegateNode *next = mpNext;
	VDDelegateNode *prev = mpPrev;
	prev->mpNext = next;
	next->mpPrev = prev;
}

///////////////////////////////////////////////////////////////////////////////

VDEventBase::VDEventBase() {
	mAnchor.mpPrev = mAnchor.mpNext = &mAnchor;
}

VDEventBase::~VDEventBase() {
	while(mAnchor.mpPrev != &mAnchor)
		Remove(static_cast<VDDelegate&>(*mAnchor.mpPrev));
}

void VDEventBase::Add(VDDelegate& dbase) {
	VDDelegateNode *next = mAnchor.mpNext;

	VDASSERT(dbase.mpPrev == &dbase);

	mAnchor.mpNext = &dbase;
	dbase.mpPrev = &mAnchor;
	dbase.mpNext = next;
	next->mpPrev = &dbase;
}

void VDEventBase::Remove(VDDelegate& dbase) {
	VDASSERT(dbase.mpPrev != &dbase);

	VDDelegateNode *next = dbase.mpNext;
	VDDelegateNode *prev = dbase.mpPrev;
	prev->mpNext = next;
	next->mpPrev = prev;
	dbase.mpPrev = dbase.mpNext = &dbase;
}

void VDEventBase::Raise(void *src, const void *info) {
	// We allow the specific case of removing the delegate that's being removed.
	VDDelegateNode *node = mAnchor.mpNext;
	
	while(node != &mAnchor) {
		VDDelegateNode *next = node->mpNext;

		VDDelegate& dbase = static_cast<VDDelegate&>(*node);

		dbase.mpCallback(src, info, dbase);

		node = next;
	}
}
