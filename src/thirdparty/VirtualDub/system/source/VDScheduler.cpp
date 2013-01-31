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
#include <vd2/system/vdtypes.h>
#include <vd2/system/VDScheduler.h>
#include <vd2/system/thread.h>
#include <vd2/system/error.h>
#include <windows.h>

VDScheduler::VDScheduler()
	: mpErrorCB(NULL)
	, pWakeupSignal(NULL)
	, pParentSchedulerNode(NULL)
	, mbExitThreads(false)
{
}

VDScheduler::~VDScheduler() {
}

void VDScheduler::setSignal(VDSignal *pSignal) {
	pWakeupSignal = pSignal;
}

void VDScheduler::setSchedulerNode(VDSchedulerNode *pSchedulerNode) {
	pParentSchedulerNode = pSchedulerNode;
}

void VDScheduler::BeginShutdown() {
	mbExitThreads = true;
	Ping();
}

void VDScheduler::Repost(VDSchedulerNode *pNode, bool bReschedule) {
	vdsynchronized(csScheduler) {
		if (pNode->bCondemned) {
			tSuspendList::iterator it(listSuspends.begin()), itEnd(listSuspends.end());

			while(it!=itEnd) {
				VDSchedulerSuspendNode *pSuspendNode = *it;

				if (pSuspendNode->mpNode == pNode) {
					it = listSuspends.erase(it);
					pSuspendNode->mSignal.signal();
				} else
					++it;
			}
		} else {
			pNode->bRunning = false;
			if (bReschedule || pNode->bReschedule) {
				pNode->bReschedule = false;
				pNode->bReady = true;
				listReady.push_back(pNode);
			} else
				listWaiting.push_back(pNode);
		}
	}
}

bool VDScheduler::Run() {
	VDSchedulerNode *pNode = NULL;
	vdsynchronized(csScheduler) {
		if (!listReady.empty()) {
			pNode = listReady.front();
			listReady.pop_front();
			pNode->bRunning = true;
			pNode->bReady = false;
		}
	}

	if (!pNode)
		return false;

	bool bReschedule;
	try {
		bReschedule = pNode->Service();
	} catch(MyError& e) {
		Repost(pNode, false);

		vdsynchronized(csScheduler) {
			if (mpErrorCB) {
				if (!mpErrorCB->OnAsyncError(e))
					throw;
			}
		}

		return true;
	} catch(...) {
		Repost(pNode, false);
		throw;
	}

	Repost(pNode, bReschedule);

	return true;
}

bool VDScheduler::IdleWait() {
	if (mbExitThreads)
		return false;

	if (pWakeupSignal) {
#if 0
		while(WAIT_TIMEOUT == WaitForSingleObject(pWakeupSignal->getHandle(), 1000))
			DumpStatus();
#else
		pWakeupSignal->wait();
#endif
	}

	return true;
}

void VDScheduler::Ping() {
	if (pWakeupSignal)
		pWakeupSignal->signal();
}

void VDScheduler::Lock() {
	++csScheduler;
}

void VDScheduler::Unlock() {
	--csScheduler;
}

void VDScheduler::Reschedule(VDSchedulerNode *pNode) {
	VDCriticalSection::AutoLock lock(csScheduler);

	RescheduleFast(pNode);
}

void VDScheduler::RescheduleFast(VDSchedulerNode *pNode) {
	if (pNode->bReady)
		return;

	pNode->bReady = true;

	if (pNode->bRunning)
		pNode->bReschedule = true;
	else {
		if (pWakeupSignal)
			pWakeupSignal->signal();

		if (pParentSchedulerNode)
			pParentSchedulerNode->Reschedule();

		listWaiting.erase(pNode);
		listReady.push_back(pNode);
	}
}

void VDScheduler::Add(VDSchedulerNode *pNode) {
	VDASSERT(pNode);

	pNode->mpScheduler = this;
	pNode->bRunning = false;
	pNode->bReschedule = false;
	pNode->bReady = true;
	pNode->bCondemned = false;

	vdsynchronized(csScheduler) {
		tNodeList::iterator it(listReady.begin()), itEnd(listReady.end());

		while(it != itEnd && (*it)->nPriority <= pNode->nPriority)
			++it;

		listReady.insert(it, pNode);
	}

	if (pWakeupSignal)
		pWakeupSignal->signal();

	if (pParentSchedulerNode)
		pParentSchedulerNode->Reschedule();
}

void VDScheduler::Remove(VDSchedulerNode *pNode) {
	VDASSERT(pNode);

	VDSchedulerSuspendNode suspendNode(pNode);
	bool running = false;

	vdsynchronized(csScheduler) {
		pNode->bCondemned = true;
		if (pNode->bRunning) {
			running = true;
			listSuspends.push_back(&suspendNode);
		} else if (pNode->bReady)
			listReady.erase(pNode);
		else
			listWaiting.erase(pNode);
	}

	if (running)
		suspendNode.mSignal.wait();
}

void VDScheduler::DumpStatus() {
	vdsynchronized(csScheduler) {
		VDDEBUG2("\n    Waiting nodes:\n");
		for(tNodeList::iterator it(listWaiting.begin()), itEnd(listWaiting.end()); it!=itEnd; ++it)
			(*it)->DumpStatus();
		VDDEBUG2("\n    Ready nodes:\n");
		for(tNodeList::iterator it2(listReady.begin()), it2End(listReady.end()); it2!=it2End; ++it2)
			(*it2)->DumpStatus();
	}
}

void VDSchedulerNode::DumpStatus() {
	VDDEBUG2("        anonymous %p\n", this);
}

///////////////////////////////////////////////////////////////////////////

VDSchedulerThread::VDSchedulerThread()
	: VDThread("Scheduler thread")
	, mpScheduler(NULL)
{
}

VDSchedulerThread::~VDSchedulerThread() {
}

bool VDSchedulerThread::Start(VDScheduler *pScheduler) {
	mpScheduler = pScheduler;
	return VDThread::ThreadStart();
}

void VDSchedulerThread::ThreadRun() {
	VDScheduler& scheduler = *mpScheduler;

	do {
		while(scheduler.Run())
			;
	} while(scheduler.IdleWait());

	scheduler.Ping();
}

///////////////////////////////////////////////////////////////////////////

VDSchedulerThreadPool::VDSchedulerThreadPool()
	: mpThreads(NULL)
	, mThreadCount(0)
	, mThreadPriority(VDThread::kPriorityDefault)
{
}

VDSchedulerThreadPool::~VDSchedulerThreadPool() {
	if (mpThreads) {
		for(uint32 i=0; i<mThreadCount; ++i) {
			mpThreads[i].ThreadWait();
		}

		delete[] mpThreads;
	}
}

void VDSchedulerThreadPool::SetPriority(int priority) {
	if (mThreadPriority != priority) {
		mThreadPriority = priority;

		if (mpThreads) {
			for(uint32 i=0; i<mThreadCount; ++i) {
				VDSchedulerThread& t = mpThreads[i];

				t.ThreadSetPriority(priority);
			}
		}
	}
}

bool VDSchedulerThreadPool::Start(VDScheduler *pScheduler) {
	return Start(pScheduler, VDGetLogicalProcessorCount());
}

bool VDSchedulerThreadPool::Start(VDScheduler *pScheduler, uint32 threadCount) {
	VDASSERT(!mpThreads);

	mpThreads = new VDSchedulerThread[threadCount];
	mThreadCount = threadCount;

	bool success = true;
	for(uint32 i=0; i<mThreadCount; ++i) {
		mpThreads[i].ThreadSetPriority(mThreadPriority);

		if (!mpThreads[i].Start(pScheduler)) {
			// We don't attempt to tear down scheduling threads here. The reason is
			// that those threads have already entered the scheduler, and it's very
			// difficult to extract only a specific thread. Usually it'll suffice
			// to shut down the entire scheduler instead.
			success = false;
			break;
		}
	}

	return false;
}
