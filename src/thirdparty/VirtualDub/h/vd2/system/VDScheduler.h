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

#ifndef f_VD2_SYSTEM_VDSCHEDULER_H
#define f_VD2_SYSTEM_VDSCHEDULER_H

#include <vd2/system/vdstl.h>
#include <vd2/system/thread.h>
#include <vd2/system/error.h>

class VDSchedulerNode;
class VDSchedulerSuspendNode;
class VDSignal;
class IVDAsyncErrorCallback;

class VDScheduler {
public:
	VDScheduler();
	~VDScheduler();

	void setSignal(VDSignal *);
	VDSignal *getSignal() { return pWakeupSignal; }
	void setSchedulerNode(VDSchedulerNode *pSchedulerNode);

	IVDAsyncErrorCallback *getErrorCallback() const { return mpErrorCB; }
	void setErrorCallback(IVDAsyncErrorCallback *pCB) { mpErrorCB = pCB; }

	bool isShuttingDown() const { return mbExitThreads; }

	void BeginShutdown();							///< Start signaling scheduling threads to exit.

	bool Run();
	bool IdleWait();								///< Wait because no nodes are ready. Returns false if a thread should exit immediately.
	void Ping();									///< Restart a scheduler thread.  This is required when a scheduler thread leaves.
	void Lock();
	void Unlock();
	void Reschedule(VDSchedulerNode *);				///< Move node to Ready if Waiting.
	void RescheduleFast(VDSchedulerNode *);			///< Same as Reschedule(), but assumes the scheduler is already locked.
	void Add(VDSchedulerNode *pNode);				///< Add node to scheduler.
	void Remove(VDSchedulerNode *pNode);			///< Remove node from scheduler.
	void DumpStatus();

protected:
	void Repost(VDSchedulerNode *, bool);

	VDCriticalSection csScheduler;
	IVDAsyncErrorCallback	*mpErrorCB;
	VDSignal *pWakeupSignal;
	volatile bool	mbExitThreads;
	VDSchedulerNode *pParentSchedulerNode;

	typedef vdlist<VDSchedulerNode> tNodeList;
	tNodeList listWaiting, listReady;

	typedef vdlist<VDSchedulerSuspendNode> tSuspendList;
	tSuspendList listSuspends;
};

class VDSchedulerNode : public vdlist<VDSchedulerNode>::node {
friend class VDScheduler;
public:
	int nPriority;

	VDSchedulerNode() : nPriority(0), mpScheduler(NULL) {}

	virtual bool Service()=0;

	virtual void DumpStatus();

	void Reschedule() { mpScheduler->Reschedule(this); }
	void RemoveFromScheduler() { mpScheduler->Remove(this); }

protected:
	VDScheduler *mpScheduler;
	volatile bool bRunning;
	volatile bool bReschedule;
	volatile bool bReady;
	volatile bool bCondemned;
};

class VDSchedulerSuspendNode : public vdlist<VDSchedulerSuspendNode>::node {
public:
	VDSchedulerSuspendNode(VDSchedulerNode *pNode) : mpNode(pNode) {}

	VDSchedulerNode *mpNode;
	VDSignal mSignal;
};

class VDSchedulerThread : public VDThread {
	VDSchedulerThread(const VDSchedulerThread&);
	VDSchedulerThread& operator=(const VDSchedulerThread&);
public:
	VDSchedulerThread();
	~VDSchedulerThread();

	bool Start(VDScheduler *pScheduler);

protected:
	void ThreadRun();

	VDScheduler *mpScheduler;
	uint32 mAffinity;
};

class VDSchedulerThreadPool {
	VDSchedulerThreadPool(const VDSchedulerThreadPool&);
	VDSchedulerThreadPool& operator=(const VDSchedulerThreadPool&);
public:
	VDSchedulerThreadPool();
	~VDSchedulerThreadPool();

	uint32 GetThreadCount() const { return mThreadCount; }

	void SetPriority(int priority);

	bool Start(VDScheduler *pScheduler);
	bool Start(VDScheduler *pScheduler, uint32 threadCount);

protected:
	VDSchedulerThread *mpThreads;
	uint32 mThreadCount;
	int mThreadPriority;
};

#endif
