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
#include <new>

#include <windows.h>
#include <mmsystem.h>

#include <vd2/system/time.h>
#include <vd2/system/thread.h>
#include <vd2/system/thunk.h>

/* MPC-HC comment out: we use winmm.lib in the project file
#ifdef _MSC_VER
	#pragma comment(lib, "winmm")
#endif
*/

uint32 VDGetCurrentTick() {
	return (uint32)GetTickCount();
}

uint64 VDGetPreciseTick() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

namespace {
	uint64 VDGetPreciseTicksPerSecondNowI() {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return freq.QuadPart;
	}

	double VDGetPreciseTicksPerSecondNow() {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (double)freq.QuadPart;
	}
}

uint64 VDGetPreciseTicksPerSecondI() {
	static uint64 ticksPerSecond = VDGetPreciseTicksPerSecondNowI();

	return ticksPerSecond;
}

double VDGetPreciseTicksPerSecond() {
	static double ticksPerSecond = VDGetPreciseTicksPerSecondNow();

	return ticksPerSecond;
}

double VDGetPreciseSecondsPerTick() {
	static double secondsPerTick = 1.0 / VDGetPreciseTicksPerSecondNow();

	return secondsPerTick;
}

uint32 VDGetAccurateTick() {
	return timeGetTime();
}

///////////////////////////////////////////////////////////////////////////////
VDCallbackTimer::VDCallbackTimer()
	: mTimerAccuracy(0)
{
}

VDCallbackTimer::~VDCallbackTimer() {
	Shutdown();
}

bool VDCallbackTimer::Init(IVDTimerCallback *pCB, uint32 period_ms) {
	return Init2(pCB, period_ms * 10000);
}

bool VDCallbackTimer::Init2(IVDTimerCallback *pCB, uint32 period_100ns) {
	return Init3(pCB, period_100ns, period_100ns >> 1, true);
}

bool VDCallbackTimer::Init3(IVDTimerCallback *pCB, uint32 period_100ns, uint32 accuracy_100ns, bool precise) {
	Shutdown();

	mpCB = pCB;
	mbExit = false;
	mbPrecise = precise;

	UINT accuracy = accuracy_100ns / 10000;
	if (accuracy > 10)
		accuracy = 10;

	TIMECAPS tc;
	if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof tc)) {
		if (accuracy < tc.wPeriodMin)
			accuracy = tc.wPeriodMin;
		if (accuracy > tc.wPeriodMax)
			accuracy = tc.wPeriodMax;
	}

	if (TIMERR_NOERROR == timeBeginPeriod(accuracy)) {
		mTimerAccuracy = accuracy;
		mTimerPeriod = period_100ns;
		mTimerPeriodAdjustment = 0;
		mTimerPeriodDelta = 0;

		if (ThreadStart())
			return true;
	}

	Shutdown();

	return false;
}

void VDCallbackTimer::Shutdown() {
	if (isThreadActive()) {
		mbExit = true;
		msigExit.signal();
		ThreadWait();
	}

	if (mTimerAccuracy) {
		timeEndPeriod(mTimerAccuracy);
		mTimerAccuracy = 0;
	}
}

void VDCallbackTimer::SetRateDelta(int delta_100ns) {
	mTimerPeriodDelta = delta_100ns;
}

void VDCallbackTimer::AdjustRate(int adjustment_100ns) {
	mTimerPeriodAdjustment += adjustment_100ns;
}

bool VDCallbackTimer::IsTimerRunning() const {
	return const_cast<VDCallbackTimer *>(this)->isThreadActive();
}

void VDCallbackTimer::ThreadRun() {
	uint32 timerPeriod = mTimerPeriod;
	uint32 periodHi = timerPeriod / 10000;
	uint32 periodLo = timerPeriod % 10000;
	uint32 nextTimeHi = VDGetAccurateTick() + periodHi;
	uint32 nextTimeLo = periodLo;

	uint32 maxDelay = mTimerPeriod / 2000;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	HANDLE hExit = msigExit.getHandle();

	if (!mbPrecise) {
		while(!mbExit) {
			DWORD res = ::WaitForSingleObject(hExit, periodHi);

			if (res != WAIT_TIMEOUT)
				break;

			mpCB->TimerCallback();
		}
	} else {
		while(!mbExit) {
			uint32 currentTime = VDGetAccurateTick();
			sint32 delta = nextTimeHi - currentTime;

			if (delta > 0) {
				// safety guard against the clock going nuts
				DWORD res;
				if ((uint32)delta > maxDelay)
					res = ::WaitForSingleObject(hExit, maxDelay);
				else
					res = ::WaitForSingleObject(hExit, nextTimeHi - currentTime);

				if (res != WAIT_TIMEOUT)
					break;
			}

			if ((uint32)abs(delta) > maxDelay) {
				nextTimeHi = currentTime + periodHi;
				nextTimeLo = periodLo;
			} else {
				nextTimeLo += periodLo;
				nextTimeHi += periodHi;
				if (nextTimeLo >= 10000) {
					nextTimeLo -= 10000;
					++nextTimeHi;
				}
			}

			mpCB->TimerCallback();

			int adjust = mTimerPeriodAdjustment.xchg(0);
			int perdelta = mTimerPeriodDelta;

			if (adjust || perdelta) {
				timerPeriod += adjust;
				periodHi = (timerPeriod+perdelta) / 10000;
				periodLo = (timerPeriod+perdelta) % 10000;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

VDLazyTimer::VDLazyTimer()
	: mTimerId(0)
	, mpCB(NULL)
	, mbPeriodic(false)
{
	if (!VDInitThunkAllocator())
		throw MyError("Unable to initialize thunk allocator.");

	mpThunk = VDCreateFunctionThunkFromMethod(this, &VDLazyTimer::StaticTimeCallback, true);
	if (!mpThunk) {
		VDShutdownThunkAllocator();
		throw MyError("Unable to create timer thunk.");
	}
}

VDLazyTimer::~VDLazyTimer() {
	Stop();

	VDDestroyFunctionThunk(mpThunk);
	VDShutdownThunkAllocator();
}

void VDLazyTimer::SetOneShot(IVDTimerCallback *pCB, uint32 delay) {
	Stop();

	mbPeriodic = false;
	mpCB = pCB;
	mTimerId = SetTimer(NULL, 0, delay, (TIMERPROC)mpThunk);
}

void VDLazyTimer::SetPeriodic(IVDTimerCallback *pCB, uint32 delay) {
	Stop();

	mbPeriodic = true;
	mpCB = pCB;
	mTimerId = SetTimer(NULL, 0, delay, (TIMERPROC)mpThunk);
}

void VDLazyTimer::Stop() {
	if (mTimerId) {
		KillTimer(NULL, mTimerId);
		mTimerId = 0;
	}
}

void VDLazyTimer::StaticTimeCallback(VDZHWND hwnd, VDZUINT msg, VDZUINT_PTR id, VDZDWORD time) {
	if (!mbPeriodic)
		Stop();

	if (mpCB)
		mpCB->TimerCallback();
}
