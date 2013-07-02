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

#ifndef f_VD2_SYSTEM_TIME_H
#define f_VD2_SYSTEM_TIME_H

#include <vd2/system/vdtypes.h>
#include <vd2/system/atomic.h>
#include <vd2/system/thread.h>
#include <vd2/system/win32/miniwindows.h>

class VDFunctionThunk;

// VDGetCurrentTick: Retrieve current process timer, in milliseconds.  Should only
// be used for sparsing updates/checks, and not for precision timing.  Approximate
// resolution is 55ms under Win9x and 10-15ms under WinNT. The advantage of this
// call is that it is usually extremely fast (just reading from the PEB).
uint32 VDGetCurrentTick();

// VDGetPreciseTick: Retrieves high-performance timer (QueryPerformanceCounter in
// Win32). This is very precise, often <1us, but often suffers from various bugs.
// that make it undesirable for high-accuracy requirements. On x64 Windows it
// can run at 1/2 speed when CPU throttling is enabled, and on some older buggy
// chipsets it can skip around occasionally.
uint64 VDGetPreciseTick();
uint64 VDGetPreciseTicksPerSecondI();
double VDGetPreciseTicksPerSecond();
double VDGetPreciseSecondsPerTick();

// VDGetAccurateTick: Reads a timer with good precision and accuracy, in
// milliseconds. On Win9x, it has 1ms precision; on WinNT, it may have anywhere
// from 1ms to 10-15ms, although 1ms can be forced with timeBeginPeriod().
uint32 VDGetAccurateTick();

// VDCallbackTimer is an abstraction of the Windows multimedia timer.  As such, it
// is rather expensive to instantiate, and should only be used for critical timing
// needs... such as multimedia.  Basically, there should only really be one or two
// of these running.  Win32 typically implements these as separate threads
// triggered off a timer, so despite the outdated documentation -- which still hasn't
// been updated from Windows 3.1 -- you can call almost any function from the
// callback.  Execution time in the callback delays other timers, however, so the
// callback should still execute as quickly as possible.

class VDINTERFACE IVDTimerCallback {
public:
	virtual void TimerCallback() = 0;
};

class VDCallbackTimer : private VDThread {
public:
	VDCallbackTimer();
	~VDCallbackTimer();

	bool Init(IVDTimerCallback *pCB, uint32 period_ms);
	bool Init2(IVDTimerCallback *pCB, uint32 period_100ns);
	bool Init3(IVDTimerCallback *pCB, uint32 period_100ns, uint32 accuracy_100ns, bool precise);
	void Shutdown();

	void SetRateDelta(int delta_100ns);
	void AdjustRate(int adjustment_100ns);

	bool IsTimerRunning() const;

private:
	void ThreadRun();

	IVDTimerCallback *mpCB;
	unsigned		mTimerAccuracy;
	uint32			mTimerPeriod;
	VDAtomicInt		mTimerPeriodDelta;
	VDAtomicInt		mTimerPeriodAdjustment;

	VDSignal		msigExit;

	volatile bool	mbExit;				// this doesn't really need to be atomic -- think about it
	bool			mbPrecise;
};


class VDLazyTimer {
public:
	VDLazyTimer();
	~VDLazyTimer();

	void SetOneShot(IVDTimerCallback *pCB, uint32 delay);
	void SetPeriodic(IVDTimerCallback *pCB, uint32 delay);
	void Stop();

protected:
	void StaticTimeCallback(VDZHWND hwnd, VDZUINT msg, VDZUINT_PTR id, VDZDWORD time);

	uint32				mTimerId;
	bool				mbPeriodic;
	VDFunctionThunk		*mpThunk;
	IVDTimerCallback	*mpCB;
};

#endif
