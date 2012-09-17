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

#include "stdafx.h"
#include <vd2/system/time.h>
#include <vd2/system/profile.h>

///////////////////////////////////////////////////////////////////////////

VDRTProfiler *g_pCentralProfiler;

void VDInitProfilingSystem() {
	if (!g_pCentralProfiler)
		g_pCentralProfiler = new VDRTProfiler;
}

void VDDeinitProfilingSystem() {
	delete g_pCentralProfiler;
	g_pCentralProfiler = 0;
}

VDRTProfiler *VDGetRTProfiler() {
	return g_pCentralProfiler;
}

///////////////////////////////////////////////////////////////////////////

VDRTProfiler::VDRTProfiler()
	: mbEnableCollection(false)
{
	mPerfFreq = VDGetPreciseTicksPerSecondI();
}

VDRTProfiler::~VDRTProfiler() {
}

void VDRTProfiler::BeginCollection() {
	mbEnableCollection = true;
}

void VDRTProfiler::EndCollection() {
	mbEnableCollection = false;
}

void VDRTProfiler::Swap() {
	vdsynchronized(mLock) {
		mSnapshotTime = VDGetPreciseTick();

		// update channels
		uint32 channelCount = mChannelArray.size();
		mChannelArrayToPaint.resize(channelCount);

		for(uint32 i=0; i<channelCount; ++i) {
			Channel& src = mChannelArray[i];
			Channel& dst = mChannelArrayToPaint[i];

			dst.mpName = src.mpName;

			dst.mEventList.clear();
			dst.mEventList.swap(src.mEventList);
			if (src.mbEventPending) {
				src.mEventList.push_back(dst.mEventList.back());
				src.mEventList.back().mEndTime = mSnapshotTime;
			}
		}

		// update counters
		Counters::iterator itC(mCounterArray.begin()), itCEnd(mCounterArray.end());
		for(; itC != itCEnd; ++itC) {
			Counter& ctr = *itC;

			ctr.mDataLast = ctr.mData;

			switch(ctr.mType) {
				case kCounterTypeUint32:
					ctr.mData.u32 = *(const uint32 *)ctr.mpData;
					break;
				case kCounterTypeDouble:
					ctr.mData.d = *(const double *)ctr.mpData;
					break;
			}
		}

		mCounterArrayToPaint = mCounterArray;
	}
}

int VDRTProfiler::AllocChannel(const char *name) {
	uint32 i;

	vdsynchronized(mLock) {
		const uint32 nChannels = mChannelArray.size();

		for(i=0; i<nChannels; ++i)
			if (!mChannelArray[i].mpName)
				break;

		if (mChannelArray.size() <= i)
			mChannelArray.resize(i + 1);

		mChannelArray[i].mpName = name;
		mChannelArray[i].mbEventPending = false;
	}

	return (int)i;
}

void VDRTProfiler::FreeChannel(int ch) {
	vdsynchronized(mLock) {
		mChannelArray[ch].mpName = 0;
		mChannelArray[ch].mEventList.clear();
	}
}

void VDRTProfiler::BeginEvent(int channel, uint32 color, const char *name) {
	if (mbEnableCollection) {
		LARGE_INTEGER tim;
		QueryPerformanceCounter(&tim);
		vdsynchronized(mLock) {
			Channel& chan = mChannelArray[channel];

			if (!chan.mbEventPending) {
				chan.mbEventPending = true;
				chan.mEventList.push_back(Event());
				Event& ev = chan.mEventList.back();
				ev.mpName = name;
				ev.mColor = color;
				ev.mStartTime = tim.QuadPart;
				ev.mEndTime = tim.QuadPart;
			}
		}
	}
}

void VDRTProfiler::EndEvent(int channel) {
	if (mbEnableCollection) {
		uint64 t = VDGetPreciseTick();

		vdsynchronized(mLock) {
			Channel& chan = mChannelArray[channel];

			if (chan.mbEventPending) {
				chan.mEventList.back().mEndTime = t;
				chan.mbEventPending = false;
			}
		}
	}
}

void VDRTProfiler::RegisterCounterU32(const char *name, const uint32 *val) {
	RegisterCounter(name, val, kCounterTypeUint32);
}

void VDRTProfiler::RegisterCounterD(const char *name, const double *val) {
	RegisterCounter(name, val, kCounterTypeDouble);
}

struct VDRTProfiler::CounterByNamePred {
	bool operator()(const char *name1, const char *name2) const {
		return strcmp(name1, name2) < 0;
	}

	bool operator()(const char *name1, const Counter& ctr) const {
		return strcmp(name1, ctr.mpName) < 0;
	}

	bool operator()(const Counter& ctr, const char *name2) const {
		return strcmp(ctr.mpName, name2) < 0;
	}

	bool operator()(const Counter& ctr1, const Counter& ctr2) const {
		return strcmp(ctr1.mpName, ctr2.mpName) < 0;
	}
};

void VDRTProfiler::RegisterCounter(const char *name, const void *val, CounterType type) {
	VDASSERT(val);

	vdsynchronized(mLock) {
		Counters::iterator itBegin(mCounterArray.end());
		Counters::iterator itEnd(mCounterArray.end());
		Counters::iterator it(std::upper_bound(itBegin, itEnd, name, CounterByNamePred()));

		it = mCounterArray.insert(it, Counter());
		Counter& ctr = *it;

		memset(&ctr.mData, 0, sizeof ctr.mData);
		memset(&ctr.mDataLast, 0, sizeof ctr.mDataLast);
		ctr.mpData = val;
		ctr.mpName = name;
		ctr.mType = type;
	}
}

void VDRTProfiler::UnregisterCounter(void *p) {
	vdsynchronized(mLock) {
		Counters::iterator it(mCounterArray.begin()), itEnd(mCounterArray.end());
		for(; it!=itEnd; ++it) {
			const Counter& counter = *it;
			if (counter.mpData == p) {
				mCounterArray.erase(it);
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

IVDEventProfiler *g_pVDEventProfiler;
