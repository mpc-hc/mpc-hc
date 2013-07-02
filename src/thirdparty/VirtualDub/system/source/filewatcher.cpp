#include "stdafx.h"
#include <windows.h>
#include <vd2/system/filesys.h>
#include <vd2/system/filewatcher.h>
#include <vd2/system/thunk.h>
#include <vd2/system/w32assist.h>

VDFileWatcher::VDFileWatcher()
	: mChangeHandle(INVALID_HANDLE_VALUE)
	, mLastWriteTime(0)
	, mbWatchDir(false)
	, mbRepeatRequested(false)
	, mbThunksInited(false)
	, mpThunk(NULL)
	, mTimerId(0)
{
}

VDFileWatcher::~VDFileWatcher() {
	Shutdown();
}

bool VDFileWatcher::IsActive() const {
	return mChangeHandle != INVALID_HANDLE_VALUE;
}

void VDFileWatcher::Init(const wchar_t *file, IVDFileWatcherCallback *callback) {
	Shutdown();

	const wchar_t *pathEnd = VDFileSplitPath(file);

	VDStringW basePath(file, pathEnd);

	if (basePath.empty())
		basePath = L".";

	if (VDIsWindowsNT())
		mChangeHandle = FindFirstChangeNotificationW(basePath.c_str(), FALSE, FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
	else
		mChangeHandle = FindFirstChangeNotificationA(VDTextWToA(basePath).c_str(), FALSE, FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);

	if (mChangeHandle == INVALID_HANDLE_VALUE)
		throw MyError("Unable to monitor file: %ls", file);

	mPath = file;
	mLastWriteTime = VDFileGetLastWriteTime(mPath.c_str());
	mpCB = callback;
	mbRepeatRequested = false;
	mbWatchDir = false;

	if (callback) {
		if (!mbThunksInited)
			mbThunksInited = VDInitThunkAllocator();

		if (mbThunksInited) {
			mpThunk = VDCreateFunctionThunkFromMethod(this, &VDFileWatcher::StaticTimerCallback, true);

			if (mpThunk) {
				mTimerId = SetTimer(NULL, 0, 1000, (TIMERPROC)mpThunk);
			}
		}
	}
}

void VDFileWatcher::InitDir(const wchar_t *path, bool subdirs, IVDFileWatcherCallback *callback) {
	Shutdown();

	const DWORD flags
		= FILE_NOTIFY_CHANGE_SIZE
		| FILE_NOTIFY_CHANGE_ATTRIBUTES
		| FILE_NOTIFY_CHANGE_LAST_WRITE
		| FILE_NOTIFY_CHANGE_FILE_NAME
		| FILE_NOTIFY_CHANGE_CREATION;
	if (VDIsWindowsNT())
		mChangeHandle = FindFirstChangeNotificationW(path, subdirs, flags);
	else
		mChangeHandle = FindFirstChangeNotificationA(VDTextWToA(path).c_str(), subdirs, flags);

	if (mChangeHandle == INVALID_HANDLE_VALUE)
		throw MyError("Unable to monitor path: %ls", path);

	mPath = path;
	mpCB = callback;
	mbRepeatRequested = false;
	mbWatchDir = true;

	if (callback) {
		if (!mbThunksInited)
			mbThunksInited = VDInitThunkAllocator();

		if (mbThunksInited) {
			mpThunk = VDCreateFunctionThunkFromMethod(this, &VDFileWatcher::StaticTimerCallback, true);

			if (mpThunk) {
				mTimerId = SetTimer(NULL, 0, 1000, (TIMERPROC)mpThunk);
			}
		}
	}
}

void VDFileWatcher::Shutdown() {
	if (mChangeHandle != INVALID_HANDLE_VALUE) {
		FindCloseChangeNotification(mChangeHandle);
		mChangeHandle = INVALID_HANDLE_VALUE;
	}

	if (mTimerId) {
		KillTimer(NULL, mTimerId);
		mTimerId = 0;
	}

	if (mpThunk) {
		VDDestroyFunctionThunk(mpThunk);
		mpThunk = NULL;
	}

	if (mbThunksInited) {
		mbThunksInited = false;

		VDShutdownThunkAllocator();
	}
}

bool VDFileWatcher::Wait(uint32 delay) {
	if (mChangeHandle == INVALID_HANDLE_VALUE)
		return false;

	if (WAIT_OBJECT_0 != WaitForSingleObject(mChangeHandle, delay))
		return false;

	FindNextChangeNotification(mChangeHandle);

	if (!mbWatchDir) {
		uint64 t = VDFileGetLastWriteTime(mPath.c_str());

		if (mLastWriteTime == t)
			return false;

		mLastWriteTime = t;
	}
	return true;
}

void VDFileWatcher::StaticTimerCallback(void *, unsigned, unsigned, unsigned long) {
	if (mbRepeatRequested) {
		if (mpCB)
			mbRepeatRequested = !mpCB->OnFileUpdated(mPath.c_str());
		else
			mbRepeatRequested = false;
		return;
	}

	if (Wait(0)) {
		if (mpCB)
			mbRepeatRequested = !mpCB->OnFileUpdated(mPath.c_str());
	}
}
