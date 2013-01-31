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
#include <process.h>

#include <windows.h>

#include <vd2/system/vdtypes.h>
#include <vd2/system/thread.h>
#include <vd2/system/tls.h>
#include <vd2/system/protscope.h>
#include <vd2/system/bitmath.h>

namespace {
	//
	// This apparently came from one a talk by one of the Visual Studio
	// developers, i.e. I didn't write it.
	//
	#define MS_VC_EXCEPTION 0x406d1388

	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;        // must be 0x1000
		LPCSTR szName;       // pointer to name (in same addr space)
		DWORD dwThreadID;    // thread ID (-1 caller thread)
		DWORD dwFlags;       // reserved for future use, most be zero
	} THREADNAME_INFO;
}

VDThreadID VDGetCurrentThreadID() {
	return (VDThreadID)GetCurrentThreadId();
}

VDProcessId VDGetCurrentProcessId() {
	return (VDProcessId)GetCurrentProcessId();
}

uint32 VDGetLogicalProcessorCount() {
	DWORD_PTR processAffinityMask;
	DWORD_PTR systemAffinityMask;
	if (!::GetProcessAffinityMask(::GetCurrentProcess(), &processAffinityMask, &systemAffinityMask))
		return 1;

	// avoid unnecessary WTFs
	if (!processAffinityMask)
		return 1;

	// We use the process affinity mask as that's the number of logical processors we'll
	// actually be working with.
	return VDCountBits(processAffinityMask);
}

void VDSetThreadDebugName(VDThreadID tid, const char *name) {
	#ifdef VD_COMPILER_MSVC
		THREADNAME_INFO info;
		info.dwType		= 0x1000;
		info.szName		= name;
		info.dwThreadID	= tid;
		info.dwFlags	= 0;

		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
		} __except (EXCEPTION_CONTINUE_EXECUTION) {
		}
	#endif
}

void VDThreadSleep(int milliseconds) {
	if (milliseconds > 0)
		::Sleep(milliseconds);
}

///////////////////////////////////////////////////////////////////////////

VDThread::VDThread(const char *pszDebugName)
	: mpszDebugName(pszDebugName)
	, mhThread(0)
	, mThreadID(0)
	, mThreadPriority(INT_MIN)
{
}

VDThread::~VDThread() throw() {
	if (isThreadAttached())
		ThreadWait();
}

bool VDThread::ThreadStart() {
	VDASSERT(!isThreadAttached());

	if (!isThreadAttached()) {
		mhThread = (void *)_beginthreadex(NULL, 0, StaticThreadStart, this, 0, &mThreadID);

		if (mhThread && mThreadPriority != INT_MIN)
			::SetThreadPriority(mhThread, mThreadPriority);
	}

	return mhThread != 0;
}

void VDThread::ThreadDetach() {
	if (isThreadAttached()) {
		CloseHandle((HANDLE)mhThread);
		mhThread = NULL;
		mThreadID = 0;
	}
}

void VDThread::ThreadWait() {
	if (isThreadAttached()) {
		WaitForSingleObject((HANDLE)mhThread, INFINITE);
		ThreadDetach();
		mThreadID = 0;
	}
}

void VDThread::ThreadSetPriority(int priority) {
	if (mThreadPriority != priority) {
		mThreadPriority = priority;

		if (mhThread && priority != INT_MIN)
			::SetThreadPriority(mhThread, priority);
	}
}

bool VDThread::isThreadActive() {
	if (isThreadAttached()) {
		if (WAIT_TIMEOUT == WaitForSingleObject((HANDLE)mhThread, 0))
			return true;

		ThreadDetach();
		mThreadID = 0;
	}
	return false;
}

void VDThread::ThreadFinish() {
	_endthreadex(0);
}

void *VDThread::ThreadLocation() const {
	if (!isThreadAttached())
		return NULL;

	CONTEXT ctx;

	ctx.ContextFlags = CONTEXT_CONTROL;

	SuspendThread(mhThread);
	GetThreadContext(mhThread, &ctx);
	ResumeThread(mhThread);

#if defined(VD_CPU_AMD64)
	return (void *)ctx.Rip;
#elif defined(VD_CPU_X86)
	return (void *)ctx.Eip;
#elif defined(VD_CPU_ARM)
	return (void *)ctx.Pc;
#endif
}

///////////////////////////////////////////////////////////////////////////

unsigned __stdcall VDThread::StaticThreadStart(void *pThisAsVoid) {
	VDThread *pThis = static_cast<VDThread *>(pThisAsVoid);

	// We cannot use mThreadID here because it might already have been
	// invalidated by a detach in the main thread.
	if (pThis->mpszDebugName)
		VDSetThreadDebugName(GetCurrentThreadId(), pThis->mpszDebugName);

	VDInitThreadData(pThis->mpszDebugName);

	vdprotected1("running thread \"%.64s\"", const char *, pThis->mpszDebugName) {
		pThis->ThreadRun();
	}

	// NOTE: Do not put anything referencing this here, since our object
	//       may have been destroyed by the threaded code.

	VDDeinitThreadData();

	return 0;
}

///////////////////////////////////////////////////////////////////////////

void VDCriticalSection::StructCheck() {
	VDASSERTCT(sizeof(CritSec) == sizeof(CRITICAL_SECTION));
}

///////////////////////////////////////////////////////////////////////////

VDSignal::VDSignal() {
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

VDSignalPersistent::VDSignalPersistent() {
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

VDSignalBase::~VDSignalBase() {
	CloseHandle(hEvent);
}

void VDSignalBase::signal() {
	SetEvent(hEvent);
}

void VDSignalBase::wait() {
	WaitForSingleObject(hEvent, INFINITE);
}

bool VDSignalBase::check() {
	return WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0);
}

int VDSignalBase::wait(VDSignalBase *second) {
	HANDLE		hArray[16];
	DWORD		dwRet;

	hArray[0] = hEvent;
	hArray[1] = second->hEvent;

	dwRet = WaitForMultipleObjects(2, hArray, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
}

int VDSignalBase::wait(VDSignalBase *second, VDSignalBase *third) {
	HANDLE		hArray[3];
	DWORD		dwRet;

	hArray[0] = hEvent;
	hArray[1] = second->hEvent;
	hArray[2] = third->hEvent;

	dwRet = WaitForMultipleObjects(3, hArray, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
}

int VDSignalBase::waitMultiple(const VDSignalBase **signals, int count) {
	VDASSERT(count <= 16);

	HANDLE handles[16];
	int active = 0;

	for(int i=0; i<count; ++i) {
		HANDLE h = signals[i]->hEvent;

		if (h)
			handles[active++] = h;
	}

	if (!active)
		return -1;

	DWORD dwRet = WaitForMultipleObjects(active, handles, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
}

bool VDSignalBase::tryWait(uint32 timeoutMillisec) {
	return WAIT_OBJECT_0 == WaitForSingleObject(hEvent, timeoutMillisec);
}

void VDSignalPersistent::unsignal() {
	ResetEvent(hEvent);
}

VDSemaphore::VDSemaphore(int initial)
	: mKernelSema(CreateSemaphore(NULL, initial, 0x0fffffff, NULL))
{
}

VDSemaphore::~VDSemaphore() {
	if (mKernelSema)
		CloseHandle(mKernelSema);
}

void VDSemaphore::Reset(int count) {
	// reset semaphore to zero
	while(WAIT_OBJECT_0 == WaitForSingleObject(mKernelSema, 0))
		;

	if (count)
		ReleaseSemaphore(mKernelSema, count, NULL);
}
