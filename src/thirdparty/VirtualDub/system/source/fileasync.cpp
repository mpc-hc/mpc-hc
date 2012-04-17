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
#include <windows.h>
#include <malloc.h>
#include <vd2/system/error.h>
#include <vd2/system/file.h>
#include <vd2/system/fileasync.h>
#include <vd2/system/thread.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/VDString.h>
#include <vd2/system/VDRingBuffer.h>
#include <vd2/system/w32assist.h>

///////////////////////////////////////////////////////////////////////////
//
//	VDFileAsync - Windows 9x implementation
//
///////////////////////////////////////////////////////////////////////////

class VDFileAsync9x : public IVDFileAsync, protected VDThread {
public:
	VDFileAsync9x(bool useFastMode, bool writeThrough);
	~VDFileAsync9x();

	void SetPreemptiveExtend(bool b) { mbPreemptiveExtend = b; }
	bool IsPreemptiveExtendActive() { return mbPreemptiveExtend; }

	bool IsOpen() { return mhFileSlow != INVALID_HANDLE_VALUE; }

	void Open(const wchar_t *pszFilename, uint32 count, uint32 bufferSize);
	void Open(VDFileHandle h, uint32 count, uint32 bufferSize);
	void Close();
	void FastWrite(const void *pData, uint32 bytes);
	void FastWriteEnd();
	void Write(sint64 pos, const void *pData, uint32 bytes);
	bool Extend(sint64 pos);
	void Truncate(sint64 pos);
	void SafeTruncateAndClose(sint64 pos);
	sint64 GetSize();
	sint64 GetFastWritePos() { return mClientFastPointer; }

protected:
	void WriteZero(sint64 pos, uint32 bytes);
	void Seek(sint64 pos);
	bool SeekNT(sint64 pos);
	void ThrowError();
	void ThreadRun();

	HANDLE		mhFileSlow;
	HANDLE		mhFileFast;
	uint32		mBlockSize;
	uint32		mBlockCount;
	uint32		mSectorSize;
	sint64		mClientSlowPointer;
	sint64		mClientFastPointer;

	const bool		mbUseFastMode;
	const bool		mbWriteThrough;

	volatile bool	mbPreemptiveExtend;

	enum {
		kStateNormal,
		kStateFlush,
		kStateAbort
	};
	VDAtomicInt	mState;

	VDSignal	mReadOccurred;
	VDSignal	mWriteOccurred;

	VDRingBuffer<char, VDFileUnbufferAllocator<char> >	mBuffer;

	VDStringA	mFilename;
	VDAtomicPtr<MyError>	mpError;
};

///////////////////////////////////////////////////////////////////////////

VDFileAsync9x::VDFileAsync9x(bool useFastMode, bool writeThrough)
	: mhFileSlow(INVALID_HANDLE_VALUE)
	, mhFileFast(INVALID_HANDLE_VALUE)
	, mClientSlowPointer(0)
	, mClientFastPointer(0)
	, mbUseFastMode(useFastMode)
	, mbWriteThrough(writeThrough)
	, mbPreemptiveExtend(false)
	, mpError(NULL)
{
}

VDFileAsync9x::~VDFileAsync9x() {
	Close();
}

void VDFileAsync9x::Open(const wchar_t *pszFilename, uint32 count, uint32 bufferSize) {
	try {
		mFilename = VDTextWToA(pszFilename);

		const DWORD slowFlags = mbWriteThrough ? FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH : FILE_ATTRIBUTE_NORMAL;

		mhFileSlow = CreateFile(mFilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, slowFlags, NULL);
		if (mhFileSlow == INVALID_HANDLE_VALUE)
			throw MyWin32Error("Unable to open file \"%s\" for write: %%s", GetLastError(), mFilename.c_str());

		if (mbUseFastMode)
			mhFileFast = CreateFile(mFilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);

		mSectorSize = 4096;		// guess for now... proper way would be GetVolumeMountPoint() followed by GetDiskFreeSpace().

		mBlockSize = bufferSize;
		mBlockCount = count;
		mBuffer.Init(count * bufferSize);

		mState = kStateNormal;
	} catch(const MyError&) {
		Close();
		throw;
	}

	ThreadStart();
}

void VDFileAsync9x::Open(VDFileHandle h, uint32 count, uint32 bufferSize) {
	try {
		mFilename = "<anonymous pipe>";

		HANDLE hProcess = GetCurrentProcess();
		if (!DuplicateHandle(hProcess, h, hProcess, &mhFileSlow, 0, FALSE, DUPLICATE_SAME_ACCESS))
			throw MyWin32Error("Unable to open file \"%s\" for write: %%s", GetLastError(), mFilename.c_str());

		mSectorSize = 4096;		// guess for now... proper way would be GetVolumeMountPoint() followed by GetDiskFreeSpace().

		mBlockSize = bufferSize;
		mBlockCount = count;
		mBuffer.Init(count * bufferSize);

		mState = kStateNormal;
	} catch(const MyError&) {
		Close();
		throw;
	}

	ThreadStart();
}

void VDFileAsync9x::Close() {
	mState = kStateAbort;
	mWriteOccurred.signal();
	ThreadWait();

	if (mhFileSlow != INVALID_HANDLE_VALUE) {
		CloseHandle(mhFileSlow);
		mhFileSlow = INVALID_HANDLE_VALUE;
	}
	if (mhFileFast != INVALID_HANDLE_VALUE) {
		CloseHandle(mhFileFast);
		mhFileFast = INVALID_HANDLE_VALUE;
	}
}

void VDFileAsync9x::FastWrite(const void *pData, uint32 bytes) {
	if (mhFileFast == INVALID_HANDLE_VALUE) {
		if (pData)
			Write(mClientFastPointer, pData, bytes);
		else
			WriteZero(mClientFastPointer, bytes);
	} else {
		if (mpError)
			ThrowError();

		uint32 bytesLeft = bytes;
		while(bytesLeft) {
			int actual;
			void *p = mBuffer.LockWrite(bytesLeft, actual);

			if (!actual) {
				mReadOccurred.wait();
				if (mpError)
					ThrowError();
				continue;
			}

			if (pData) {
				memcpy(p, pData, actual);
				pData = (const char *)pData + actual;
			} else {
				memset(p, 0, actual);
			}
			mBuffer.UnlockWrite(actual);
			mWriteOccurred.signal();
			bytesLeft -= actual;
		}
	}

	mClientFastPointer += bytes;
}

void VDFileAsync9x::FastWriteEnd() {
	FastWrite(NULL, mSectorSize - 1);

	mState = kStateFlush;
	mWriteOccurred.signal();
	ThreadWait();

	if (mpError)
		ThrowError();
}

void VDFileAsync9x::Write(sint64 pos, const void *p, uint32 bytes) {
	Seek(pos);

	DWORD dwActual;
	if (!WriteFile(mhFileSlow, p, bytes, &dwActual, NULL) || dwActual != bytes) {
		mClientSlowPointer = -1;
		throw MyWin32Error("Write error occurred on file \"%s\": %%s\n", GetLastError(), mFilename.c_str());
	}

	mClientSlowPointer += bytes;
}

void VDFileAsync9x::WriteZero(sint64 pos, uint32 bytes) {
	uint32 bufsize = bytes > 2048 ? 2048 : bytes;
	void *p = _alloca(bufsize);
	memset(p, 0, bufsize);

	while(bytes > 0) {
		uint32 tc = bytes > 2048 ? 2048 : bytes;

		Write(pos, p, tc);
		pos += tc;
		bytes -= tc;
	}
}

bool VDFileAsync9x::Extend(sint64 pos) {
	return SeekNT(pos) && SetEndOfFile(mhFileSlow);
}

void VDFileAsync9x::Truncate(sint64 pos) {
	Seek(pos);
	if (!SetEndOfFile(mhFileSlow))
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());
}

void VDFileAsync9x::SafeTruncateAndClose(sint64 pos) {
	if (mhFileSlow != INVALID_HANDLE_VALUE) {
		FastWrite(NULL, mSectorSize - 1);

		mState = kStateFlush;
		mWriteOccurred.signal();
		ThreadWait();

		Extend(pos);
		Close();
	}
}

sint64 VDFileAsync9x::GetSize() {
	DWORD dwSizeHigh;
	DWORD dwSizeLow = GetFileSize(mhFileSlow, &dwSizeHigh);

	if (dwSizeLow == (DWORD)-1 && GetLastError() != NO_ERROR)
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());

	return dwSizeLow + ((sint64)dwSizeHigh << 32);
}

void VDFileAsync9x::Seek(sint64 pos) {
	if (!SeekNT(pos))
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());
}

bool VDFileAsync9x::SeekNT(sint64 pos) {
	if (mClientSlowPointer == pos)
		return true;

	LONG posHi = (LONG)(pos >> 32);
	DWORD result = SetFilePointer(mhFileSlow, (LONG)pos, &posHi, FILE_BEGIN);

	if (result == INVALID_SET_FILE_POINTER) {
		DWORD dwError = GetLastError();

		if (dwError != NO_ERROR) {
			mClientSlowPointer = -1;
			return false;
		}
	}

	mClientSlowPointer = pos;

	return true;
}

void VDFileAsync9x::ThrowError() {
	MyError *e = mpError.xchg(NULL);

	if (e) {
		if (mhFileFast != INVALID_HANDLE_VALUE) {
			CloseHandle(mhFileFast);
			mhFileFast = INVALID_HANDLE_VALUE;
		}

		MyError tmp;
		tmp.TransferFrom(*e);
		delete e;
		throw tmp;
	}
}

void VDFileAsync9x::ThreadRun() {
	bool	bPreemptiveExtend = mbPreemptiveExtend;
	sint64	currentSize;
	sint64	pos = 0;
	uint32	bufferSize = mBlockCount * mBlockSize;
	HANDLE  hFile = mhFileFast != INVALID_HANDLE_VALUE ? mhFileFast : mhFileSlow;

	try {
		if (bPreemptiveExtend && !VDGetFileSizeW32(hFile, currentSize))
			throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());

		for(;;) {
			int state = mState;

			if (state == kStateAbort)
				break;

			int actual;
			const void *p = mBuffer.LockRead(mBlockSize, actual);

			if ((uint32)actual < mBlockSize) {
				if (state == kStateNormal) {
					mWriteOccurred.wait();
					continue;
				}

				VDASSERT(state == kStateFlush);

				actual &= ~(mSectorSize-1);
				if (!actual)
					break;
			} else {
				if (bPreemptiveExtend) {
					sint64 checkpt = pos + mBlockSize + bufferSize;

					if (checkpt > currentSize) {
						currentSize += bufferSize;
						if (currentSize < checkpt)
							currentSize = checkpt;

						if (!VDSetFilePointerW32(hFile, currentSize, FILE_BEGIN)
							|| !SetEndOfFile(hFile))
							mbPreemptiveExtend = bPreemptiveExtend = false;

						if (!VDSetFilePointerW32(hFile, pos, FILE_BEGIN))
							throw MyWin32Error("Seek error occurred on file \"%s\": %%s\n", GetLastError(), mFilename.c_str());
					}
				}
			}

			DWORD dwActual;
			if (!WriteFile(hFile, p, actual, &dwActual, NULL) || dwActual != actual) {
				DWORD dwError = GetLastError();
				throw MyWin32Error("Write error occurred on file \"%s\": %%s\n", dwError, mFilename.c_str());
			}

			pos += actual;

			mBuffer.UnlockRead(actual);

			mReadOccurred.signal();
		}
	} catch(MyError& e) {
		MyError *p = new MyError;

		p->TransferFrom(e);
		delete mpError.xchg(p);
		mReadOccurred.signal();
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	VDFileAsync - Windows NT implementation
//
///////////////////////////////////////////////////////////////////////////

struct VDFileAsyncNTBuffer : public OVERLAPPED {
	bool	mbActive;
	bool	mbPending;
	uint32	mLength;

	VDFileAsyncNTBuffer() : mbActive(false), mbPending(false) { hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); }
	~VDFileAsyncNTBuffer() { if (hEvent) CloseHandle(hEvent); }
};

class VDFileAsyncNT : public IVDFileAsync, private VDThread {
public:
	VDFileAsyncNT();
	~VDFileAsyncNT();

	void SetPreemptiveExtend(bool b) { mbPreemptiveExtend = b; }
	bool IsPreemptiveExtendActive() { return mbPreemptiveExtend; }

	bool IsOpen() { return mhFileSlow != INVALID_HANDLE_VALUE; }

	void Open(const wchar_t *pszFilename, uint32 count, uint32 bufferSize);
	void Open(VDFileHandle h, uint32 count, uint32 bufferSize);
	void Close();
	void FastWrite(const void *pData, uint32 bytes);
	void FastWriteEnd();
	void Write(sint64 pos, const void *pData, uint32 bytes);
	bool Extend(sint64 pos);
	void Truncate(sint64 pos);
	void SafeTruncateAndClose(sint64 pos);
	sint64 GetSize();
	sint64 GetFastWritePos() { return mClientFastPointer; }

protected:
	void WriteZero(sint64 pos, uint32 bytes);
	void Seek(sint64 pos);
	bool SeekNT(sint64 pos);
	void ThrowError();
	void ThreadRun();

	HANDLE		mhFileSlow;
	HANDLE		mhFileFast;
	uint32		mBlockSize;
	uint32		mBlockCount;
	uint32		mBufferSize;
	uint32		mSectorSize;

	enum {
		kStateNormal,
		kStateFlush,
		kStateAbort
	};
	VDAtomicInt	mState;

	VDSignal	mReadOccurred;
	VDSignal	mWriteOccurred;

	uint32		mWriteOffset;
	VDAtomicInt	mBufferLevel;
	sint64		mClientSlowPointer;
	sint64		mClientFastPointer;
	sint64		mFastPointer;

	volatile bool	mbPreemptiveExtend;

	vdautoarrayptr<VDFileAsyncNTBuffer>	mpBlocks;

	vdblock<char, VDFileUnbufferAllocator<char> >	mBuffer;

	VDAtomicPtr<MyError>	mpError;
	VDStringA	mFilename;
};

VDFileAsyncNT::VDFileAsyncNT()
	: mhFileSlow(INVALID_HANDLE_VALUE)
	, mhFileFast(INVALID_HANDLE_VALUE)
	, mFastPointer(0)
	, mClientSlowPointer(0)
	, mClientFastPointer(0)
	, mbPreemptiveExtend(false)
	, mpError(NULL)
{
}

VDFileAsyncNT::~VDFileAsyncNT() {
	Close();
}

void VDFileAsyncNT::Open(const wchar_t *pszFilename, uint32 count, uint32 bufferSize) {
	try {
		mFilename = VDTextWToA(pszFilename);

		mhFileSlow = CreateFileW(pszFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (mhFileSlow == INVALID_HANDLE_VALUE)
			throw MyWin32Error("Unable to open file \"%s\" for write: %%s", GetLastError(), mFilename.c_str());

		mhFileFast = CreateFileW(pszFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
		if (mhFileFast == INVALID_HANDLE_VALUE)
			mhFileFast = CreateFileW(pszFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED, NULL);

		mSectorSize = 4096;		// guess for now... proper way would be GetVolumeMountPoint() followed by GetDiskFreeSpace().

		mBlockSize = bufferSize;
		mBlockCount = count;
		mBufferSize = mBlockSize * mBlockCount;

		mWriteOffset = 0;
		mBufferLevel = 0;

		mState = kStateNormal;

		if (mhFileFast != INVALID_HANDLE_VALUE) {
			mpBlocks = new VDFileAsyncNTBuffer[count];
			mBuffer.resize(count * bufferSize);
			ThreadStart();
		}
	} catch(const MyError&) {
		Close();
		throw;
	}
}

void VDFileAsyncNT::Open(VDFileHandle h, uint32 count, uint32 bufferSize) {
	try {
		mFilename = "<anonymous pipe>";

		HANDLE hProcess = GetCurrentProcess();
		if (!DuplicateHandle(hProcess, h, hProcess, &mhFileSlow, 0, FALSE, DUPLICATE_SAME_ACCESS))
			throw MyWin32Error("Unable to open file \"%s\" for write: %%s", GetLastError(), mFilename.c_str());

		mSectorSize = 4096;		// guess for now... proper way would be GetVolumeMountPoint() followed by GetDiskFreeSpace().

		mBlockSize = bufferSize;
		mBlockCount = count;
		mBufferSize = mBlockSize * mBlockCount;

		mWriteOffset = 0;
		mBufferLevel = 0;

		mState = kStateNormal;

		if (mhFileFast != INVALID_HANDLE_VALUE) {
			mpBlocks = new VDFileAsyncNTBuffer[count];
			mBuffer.resize(count * bufferSize);
			ThreadStart();
		}
	} catch(const MyError&) {
		Close();
		throw;
	}
}

void VDFileAsyncNT::Close() {
	mState = kStateAbort;
	mWriteOccurred.signal();
	ThreadWait();

	if (mpError) {
		delete mpError;
		mpError = NULL;
	}

	if (mhFileSlow != INVALID_HANDLE_VALUE) {
		CloseHandle(mhFileSlow);
		mhFileSlow = INVALID_HANDLE_VALUE;
	}
	if (mhFileFast != INVALID_HANDLE_VALUE) {
		CloseHandle(mhFileFast);
		mhFileFast = INVALID_HANDLE_VALUE;
	}

	mpBlocks = NULL;
}

void VDFileAsyncNT::FastWrite(const void *pData, uint32 bytes) {
	if (mhFileFast == INVALID_HANDLE_VALUE) {
		if (pData)
			Write(mClientFastPointer, pData, bytes);
		else
			WriteZero(mClientFastPointer, bytes);
	} else {
		if (mpError)
			ThrowError();

		uint32 bytesLeft = bytes;
		while(bytesLeft) {
			uint32 actual = mBufferSize - mBufferLevel;

			if (actual > bytesLeft)
				actual = bytesLeft;

			if (mWriteOffset + actual > mBufferSize)
				actual = mBufferSize - mWriteOffset;

			if (!actual) {
				mReadOccurred.wait();
				if (mpError)
					ThrowError();
				continue;
			}

			if (pData) {
				memcpy(&mBuffer[mWriteOffset], pData, actual);
				pData = (const char *)pData + actual;
			} else {
				memset(&mBuffer[mWriteOffset], 0, actual);
			}

			uint32 oldWriteOffset = mWriteOffset;
			mWriteOffset += actual;
			if (mWriteOffset >= mBufferSize)
				mWriteOffset = 0;
			mBufferLevel += actual;

			// only bother signaling if the write offset crossed a block boundary
			if (oldWriteOffset % mBlockSize + actual >= mBlockSize) {
				mWriteOccurred.signal();
				if (mpError)
					ThrowError();
			}

			bytesLeft -= actual;
		}
	}
		
	mClientFastPointer += bytes;
}

void VDFileAsyncNT::FastWriteEnd() {
	if (mhFileFast != INVALID_HANDLE_VALUE) {
		FastWrite(NULL, mSectorSize - 1);
		mState = kStateFlush;
		mWriteOccurred.signal();
		ThreadWait();
	}

	if (mpError)
		ThrowError();
}

void VDFileAsyncNT::Write(sint64 pos, const void *p, uint32 bytes) {
	Seek(pos);

	DWORD dwActual;
	if (!WriteFile(mhFileSlow, p, bytes, &dwActual, NULL) || (mClientSlowPointer += dwActual),(dwActual != bytes))
		throw MyWin32Error("Write error occurred on file \"%s\": %%s", GetLastError(), mFilename.c_str());
}

void VDFileAsyncNT::WriteZero(sint64 pos, uint32 bytes) {
	uint32 bufsize = bytes > 2048 ? 2048 : bytes;
	void *p = _alloca(bufsize);
	memset(p, 0, bufsize);

	while(bytes > 0) {
		uint32 tc = bytes > 2048 ? 2048 : bytes;

		Write(pos, p, tc);
		pos += tc;
		bytes -= tc;
	}
}

bool VDFileAsyncNT::Extend(sint64 pos) {
	return SeekNT(pos) && SetEndOfFile(mhFileSlow);
}

void VDFileAsyncNT::Truncate(sint64 pos) {
	Seek(pos);
	if (!SetEndOfFile(mhFileSlow))
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());
}

void VDFileAsyncNT::SafeTruncateAndClose(sint64 pos) {
	if (isThreadAttached()) {
		mState = kStateAbort;
		mWriteOccurred.signal();
		ThreadWait();

		if (mpError) {
			delete mpError;
			mpError = NULL;
		}
	}

	if (mhFileSlow != INVALID_HANDLE_VALUE) {
		Extend(pos);
		Close();
	}
}

sint64 VDFileAsyncNT::GetSize() {
	DWORD dwSizeHigh;
	DWORD dwSizeLow = GetFileSize(mhFileSlow, &dwSizeHigh);

	if (dwSizeLow == (DWORD)-1 && GetLastError() != NO_ERROR)
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());

	return dwSizeLow + ((sint64)dwSizeHigh << 32);
}

void VDFileAsyncNT::Seek(sint64 pos) {
	if (!SeekNT(pos))
		throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());
}

bool VDFileAsyncNT::SeekNT(sint64 pos) {
	if (mClientSlowPointer == pos)
		return true;

	LONG posHi = (LONG)(pos >> 32);
	DWORD result = SetFilePointer(mhFileSlow, (LONG)pos, &posHi, FILE_BEGIN);

	if (result == INVALID_SET_FILE_POINTER) {
		DWORD dwError = GetLastError();

		if (dwError != NO_ERROR)
			return false;
	}

	mClientSlowPointer = pos;
	return true;
}

void VDFileAsyncNT::ThrowError() {
	MyError *e = mpError.xchg(NULL);

	if (e) {
		if (mhFileFast != INVALID_HANDLE_VALUE) {
			CloseHandle(mhFileFast);
			mhFileFast = INVALID_HANDLE_VALUE;
		}

		MyError tmp;
		tmp.TransferFrom(*e);
		delete e;
		throw tmp;
	}
}

void VDFileAsyncNT::ThreadRun() {
	int requestHead = 0;
	int requestTail = 0;
	int requestCount = mBlockCount;
	uint32 pendingLevel = 0;
	uint32 readOffset = 0;
	bool	bPreemptiveExtend = mbPreemptiveExtend;
	sint64	currentSize;

	try {
		if (!VDGetFileSizeW32(mhFileFast, currentSize))
			throw MyWin32Error("I/O error on file \"%s\": %%s", GetLastError(), mFilename.c_str());

		for(;;) {
			int state = mState;

			if (state == kStateAbort) {
				typedef BOOL (WINAPI *tpCancelIo)(HANDLE);
				static const tpCancelIo pCancelIo = (tpCancelIo)GetProcAddress(GetModuleHandle("kernel32"), "CancelIo");
				pCancelIo(mhFileFast);

				// Wait for any pending blocks to complete.
				for(int i=0; i<requestCount; ++i) {
					VDFileAsyncNTBuffer& buf = mpBlocks[i];

					if (buf.mbActive) {
						WaitForSingleObject(buf.hEvent, INFINITE);
						buf.mbActive = false;
					}
				}
				break;
			}

			uint32 actual = mBufferLevel - pendingLevel;
			VDASSERT((int)actual >= 0);
			if (readOffset + actual > mBufferSize)
				actual = mBufferSize - readOffset;

			if (actual < mBlockSize) {
				if (state == kStateNormal || actual < mSectorSize) {
					// check for blocks that have completed
					bool blocksCompleted = false;
					for(;;) {
						VDFileAsyncNTBuffer& buf = mpBlocks[requestTail];

						if (!buf.mbActive) {
							if (state == kStateFlush)
								goto all_done;

							if (!blocksCompleted) {
								// wait for further writes
								mWriteOccurred.wait();
							}
							break;
						}

						if (buf.mbPending) {
							HANDLE h[2] = {buf.hEvent, mWriteOccurred.getHandle()};
							DWORD waitResult = WaitForMultipleObjects(2, h, FALSE, INFINITE);

							if (waitResult == WAIT_OBJECT_0+1)	// write pending
								break;

							DWORD dwActual;
							if (!GetOverlappedResult(mhFileFast, &buf, &dwActual, TRUE))
								throw MyWin32Error("Write error occurred on file \"%s\": %%s", GetLastError(), mFilename.c_str());
						}

						buf.mbActive = false;

						blocksCompleted = true;

						if (++requestTail >= requestCount)
							requestTail = 0;

						mBufferLevel -= buf.mLength;
						pendingLevel -= buf.mLength;
						VDASSERT((int)mBufferLevel >= 0);
						VDASSERT((int)pendingLevel >= 0);

						mReadOccurred.signal();

					}

					continue;
				}

				VDASSERT(state == kStateFlush);

				actual &= ~(mSectorSize-1);

				VDASSERT(actual > 0);
			} else {
				actual = mBlockSize;

				if (bPreemptiveExtend) {
					sint64 checkpt = mFastPointer + mBlockSize + mBufferSize;

					if (checkpt > currentSize) {
						currentSize += mBufferSize;
						if (currentSize < checkpt)
							currentSize = checkpt;

						if (!VDSetFilePointerW32(mhFileFast, currentSize, FILE_BEGIN)
							|| !SetEndOfFile(mhFileFast))
							mbPreemptiveExtend = bPreemptiveExtend = false;
					}
				}
			}

			// Issue a write to OS
			VDFileAsyncNTBuffer& buf = mpBlocks[requestHead];

			VDASSERT(!buf.mbActive);

			DWORD dwActual;

			buf.Offset = (DWORD)mFastPointer;
			buf.OffsetHigh = (DWORD)((uint64)mFastPointer >> 32);
			buf.Internal = 0;
			buf.InternalHigh = 0;
			buf.mLength = actual;
			buf.mbPending = false;

			if (!WriteFile(mhFileFast, &mBuffer[readOffset], actual, &dwActual, &buf)) {
				if (GetLastError() != ERROR_IO_PENDING)
					throw MyWin32Error("Write error occurred on file \"%s\": %%s", GetLastError(), mFilename.c_str());

				buf.mbPending = true;
			}

			buf.mbActive = true;

			pendingLevel += actual;
			VDASSERT(pendingLevel <= (uint32)mBufferLevel);

			readOffset += actual;
			VDASSERT(readOffset <= mBufferSize);
			if (readOffset >= mBufferSize)
				readOffset = 0;

			mFastPointer += actual;

			if (++requestHead >= requestCount)
				requestHead = 0;
		}
all_done:
		;

	} catch(MyError& e) {
		MyError *p = new MyError;

		p->TransferFrom(e);
		delete mpError.xchg(p);
		mReadOccurred.signal();
	}
}

///////////////////////////////////////////////////////////////////////////

IVDFileAsync *VDCreateFileAsync(IVDFileAsync::Mode mode) {
	switch(mode) {

		case IVDFileAsync::kModeAsynchronous:
			if (VDIsWindowsNT())
				return new VDFileAsyncNT;
			// Can't do async I/O. Fall-through to 9x method.
		case IVDFileAsync::kModeThreaded:
			return new VDFileAsync9x(true, true);

		default:
			return new VDFileAsync9x(false, true);

		case IVDFileAsync::kModeBuffered:
			return new VDFileAsync9x(false, false);
	}
}
