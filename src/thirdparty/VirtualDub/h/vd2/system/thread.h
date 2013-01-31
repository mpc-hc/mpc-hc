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

#ifndef f_VD2_SYSTEM_THREAD_H
#define f_VD2_SYSTEM_THREAD_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>
#include <vd2/system/atomic.h>

typedef void *VDThreadHandle;
typedef uint32 VDThreadID;
typedef uint32 VDThreadId;
typedef uint32 VDProcessId;

#if defined(__MINGW32__) || defined(__MINGW64__)
	struct _CRITICAL_SECTION;
	typedef _CRITICAL_SECTION VDCriticalSectionW32;
#else
	struct _RTL_CRITICAL_SECTION;
	typedef _RTL_CRITICAL_SECTION VDCriticalSectionW32;
#endif

extern "C" void __declspec(dllimport) __stdcall InitializeCriticalSection(VDCriticalSectionW32 *lpCriticalSection);
extern "C" void __declspec(dllimport) __stdcall LeaveCriticalSection(VDCriticalSectionW32 *lpCriticalSection);
extern "C" void __declspec(dllimport) __stdcall EnterCriticalSection(VDCriticalSectionW32 *lpCriticalSection);
extern "C" void __declspec(dllimport) __stdcall DeleteCriticalSection(VDCriticalSectionW32 *lpCriticalSection);
extern "C" unsigned long __declspec(dllimport) __stdcall WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);
extern "C" int __declspec(dllimport) __stdcall ReleaseSemaphore(void *hSemaphore, long lReleaseCount, long *lpPreviousCount);

VDThreadID VDGetCurrentThreadID();
VDProcessId VDGetCurrentProcessId();
uint32 VDGetLogicalProcessorCount();

void VDSetThreadDebugName(VDThreadID tid, const char *name);
void VDThreadSleep(int milliseconds);

///////////////////////////////////////////////////////////////////////////
//
//	VDThread
//
//	VDThread is a quick way to portably create threads -- to use it,
//	derive a subclass from it that implements the ThreadRun() function.
//
//	Win32 notes:
//
//	The thread startup code will attempt to notify the VC++ debugger of
//	the debug name of the thread.  Only the first 9 characters are used
//	by Visual C 6.0; Visual Studio .NET will accept a few dozen.
//
//	VDThread objects must not be WaitThread()ed or destructed from a
//	DllMain() function, TLS callback for an executable, or static
//	destructor unless the thread has been detached from the object.
//  The reason is that Win32 serializes calls to DllMain() functions.
//  If you attempt to do so, you will cause a deadlock when Win32
//  attempts to fire thread detach notifications.
//
///////////////////////////////////////////////////////////////////////////

class VDThread {
public:
	enum {
		kPriorityDefault = INT_MIN
	};

	VDThread(const char *pszDebugName = NULL);	// NOTE: pszDebugName must have static duration
	~VDThread() throw();

	// external functions

	bool ThreadStart();							// start thread
	void ThreadDetach();						// detach thread (wait() won't be called)
	void ThreadWait();							// wait for thread to finish
	void ThreadSetPriority(int priority);

	bool isThreadActive();

	bool isThreadAttached() const {				// NOTE: Will return true if thread started, even if thread has since exited
		return mhThread != 0;
	}

	VDThreadHandle getThreadHandle() const {	// get handle to thread (Win32: HANDLE)
		return mhThread;
	}

	VDThreadID getThreadID() const {			// get ID of thread (Win32: DWORD)
		return mThreadID;
	}

	void *ThreadLocation() const;				// retrieve current EIP of thread (use only for debug purposes -- may not return reliable information on syscall, etc.)

	// thread-local functions

	virtual void ThreadRun() = 0;				// thread, come to life
	void ThreadFinish();						// exit thread

private:
	static unsigned __stdcall StaticThreadStart(void *pThis);

	const char *mpszDebugName;
	VDThreadHandle	mhThread;
	VDThreadID		mThreadID;
	int				mThreadPriority;
};

///////////////////////////////////////////////////////////////////////////

class VDCriticalSection {
private:
	struct CritSec {				// This is a clone of CRITICAL_SECTION.
		void	*DebugInfo;
		sint32	LockCount;
		sint32	RecursionCount;
		void	*OwningThread;
		void	*LockSemaphore;
		uint32	SpinCount;
	} csect;

	VDCriticalSection(const VDCriticalSection&);
	const VDCriticalSection& operator=(const VDCriticalSection&);
	static void StructCheck();
public:
	class AutoLock {
	private:
		VDCriticalSection& cs;
	public:
		AutoLock(VDCriticalSection& csect) : cs(csect) { cs.Lock(); }
		~AutoLock() { cs.Unlock(); }

		inline operator bool() const { return false; }
	};

	VDCriticalSection() {
		InitializeCriticalSection((VDCriticalSectionW32 *)&csect);
	}

	~VDCriticalSection() {
		DeleteCriticalSection((VDCriticalSectionW32 *)&csect);
	}

	void operator++() {
		EnterCriticalSection((VDCriticalSectionW32 *)&csect);
	}

	void operator--() {
		LeaveCriticalSection((VDCriticalSectionW32 *)&csect);
	}

	void Lock() {
		EnterCriticalSection((VDCriticalSectionW32 *)&csect);
	}

	void Unlock() {
		LeaveCriticalSection((VDCriticalSectionW32 *)&csect);
	}
};

// 'vdsynchronized' keyword
//
// The vdsynchronized(lock) keyword emulates Java's 'synchronized' keyword, which
// protects the following statement or block from race conditions by obtaining a
// lock during its execution:
//
//		vdsynchronized(list_lock) {
//			mList.pop_back();
//			if (mList.empty())
//				return false;
//		}
//
// The construct is exception safe and will release the lock even if a return,
// continue, break, or thrown exception exits the block.  However, hardware
// exceptions (access violations) may not work due to synchronous model
// exception handling.
//
// There are two Visual C++ bugs we need to work around here (both are in VC6 and VC7).
//
// 1) Declaring an object with a non-trivial destructor in a switch() condition
//    causes a C1001 INTERNAL COMPILER ERROR.
//
// 2) Using __LINE__ in a macro expanded in a function with Edit and Continue (/ZI)
//    breaks the preprocessor (KB article Q199057).  Shame, too, because without it
//    all the autolocks look the same.

#define vdsynchronized2(lock) if(VDCriticalSection::AutoLock vd__lock=(lock))VDNEVERHERE;else
#define vdsynchronized1(lock) vdsynchronized2(lock)
#define vdsynchronized(lock) vdsynchronized1(lock)

///////////////////////////////////////////////////////////////////////////

class VDSignalBase {
protected:
	void *hEvent;

public:
	~VDSignalBase();

	void signal();
	bool check();
	void wait();
	int wait(VDSignalBase *second);
	int wait(VDSignalBase *second, VDSignalBase *third);
	static int waitMultiple(const VDSignalBase **signals, int count);

	bool tryWait(uint32 timeoutMillisec);

	void *getHandle() { return hEvent; }

	void operator()() { signal(); }
};

class VDSignal : public VDSignalBase {
	VDSignal(const VDSignal&);
	VDSignal& operator=(const VDSignal&);
public:
	VDSignal();
};

class VDSignalPersistent : public VDSignalBase {
	VDSignalPersistent(const VDSignalPersistent&);
	VDSignalPersistent& operator=(const VDSignalPersistent&);
public:
	VDSignalPersistent();

	void unsignal();
};

///////////////////////////////////////////////////////////////////////////

class VDSemaphore {
public:
	VDSemaphore(int initial);
	~VDSemaphore();

	void *GetHandle() const {
		return mKernelSema;
	}

	void Reset(int count);

	void Wait() {
		WaitForSingleObject(mKernelSema, 0xFFFFFFFFU);
	}

	bool Wait(int timeout) {
		return 0 == WaitForSingleObject(mKernelSema, timeout);
	}

	bool TryWait() {
		return 0 == WaitForSingleObject(mKernelSema, 0);
	}

	void Post() {
		ReleaseSemaphore(mKernelSema, 1, NULL);
	}

private:
	void *mKernelSema;
};

#endif
