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

#include <vd2/system/error.h>
#include <vd2/system/filesys.h>
#include <vd2/system/VDString.h>
#include <vd2/system/file.h>

namespace {
	bool IsWindowsNT() {
		static bool sbIsNT = (LONG)GetVersion()>=0;
		return sbIsNT;
	}

	bool IsHardDrivePath(const wchar_t *path) {
		const VDStringW rootPath(VDFileGetRootPath(path));

		UINT type = GetDriveTypeW(rootPath.c_str());

		return type == DRIVE_FIXED || type == DRIVE_UNKNOWN || type == DRIVE_REMOVABLE;
	}
};

///////////////////////////////////////////////////////////////////////////////
//
//	VDFile
//
///////////////////////////////////////////////////////////////////////////////

using namespace nsVDFile;

VDFile::VDFile(const char *pszFileName, uint32 flags)
	: mhFile(NULL)
{
	open_internal(pszFileName, NULL, flags, true);
}

VDFile::VDFile(const wchar_t *pwszFileName, uint32 flags)
	: mhFile(NULL)
{
	open_internal(NULL, pwszFileName, flags, true);
}

VDFile::VDFile(HANDLE h)
	: mhFile(h)
{
	LONG lo, hi = 0;

	lo = SetFilePointer(h, 0, &hi, FILE_CURRENT);

	mFilePosition = (uint32)lo + ((uint64)(uint32)hi << 32);
}

VDFile::~VDFile() {
	closeNT();
}

void VDFile::open(const char *pszFilename, uint32 flags) {
	open_internal(pszFilename, NULL, flags, true);
}

void VDFile::open(const wchar_t *pwszFilename, uint32 flags) {
	open_internal(NULL, pwszFilename, flags, true);
}

bool VDFile::openNT(const wchar_t *pwszFilename, uint32 flags) {
	return open_internal(NULL, pwszFilename, flags, false);
}

bool VDFile::open_internal(const char *pszFilename, const wchar_t *pwszFilename, uint32 flags, bool throwOnError) {
	close();

	mpFilename = _wcsdup(VDFileSplitPath(pszFilename ? VDTextAToW(pszFilename).c_str() : pwszFilename));
	if (!mpFilename) {
		if (!throwOnError)
			return false;
		throw MyMemoryError();
	}

	// At least one of the read/write flags must be set.
	VDASSERT(flags & (kRead | kWrite));

	DWORD dwDesiredAccess = 0;

	if (flags & kRead)  dwDesiredAccess  = GENERIC_READ;
	if (flags & kWrite) dwDesiredAccess |= GENERIC_WRITE;

	// Win32 docs are screwed here -- FILE_SHARE_xxx is the inverse of a deny flag.

	DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if (flags & kDenyRead)	dwShareMode = FILE_SHARE_WRITE;
	if (flags & kDenyWrite) dwShareMode &= ~FILE_SHARE_WRITE;

	// One of the creation flags must be set.
	VDASSERT(flags & kCreationMask);

	DWORD dwCreationDisposition;

	uint32 creationType = flags & kCreationMask;

	switch(creationType) {
	case kOpenExisting:		dwCreationDisposition = OPEN_EXISTING; break;
	case kOpenAlways:		dwCreationDisposition = OPEN_ALWAYS; break;
	case kCreateAlways:		dwCreationDisposition = CREATE_ALWAYS; break;
	case kCreateNew:		dwCreationDisposition = CREATE_NEW; break;
	case kTruncateExisting:	dwCreationDisposition = TRUNCATE_EXISTING; break;
	default:
		VDNEVERHERE;
		return false;
	}

	VDASSERT((flags & (kSequential | kRandomAccess)) != (kSequential | kRandomAccess));

	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL;

	if (flags & kSequential)	dwAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
	if (flags & kRandomAccess)	dwAttributes |= FILE_FLAG_RANDOM_ACCESS;
	if (flags & kWriteThrough)	dwAttributes |= FILE_FLAG_WRITE_THROUGH;
	if (flags & kUnbuffered)	dwAttributes |= FILE_FLAG_NO_BUFFERING;

	VDStringA tempFilenameA;
	VDStringW tempFilenameW;

	if (IsWindowsNT()) {
		if (pszFilename) {
			tempFilenameW = VDTextAToW(pszFilename);
			pwszFilename = tempFilenameW.c_str();
			pszFilename = NULL;
		}
	} else {
		if (pwszFilename) {
			tempFilenameA = VDTextWToA(pwszFilename);
			pszFilename = tempFilenameA.c_str();
			pwszFilename = NULL;
		}
	}

	if (pszFilename)
		mhFile = CreateFileA(pszFilename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwAttributes, NULL);
	else {
		if (!IsHardDrivePath(pwszFilename))
			flags &= ~FILE_FLAG_NO_BUFFERING;

		mhFile = CreateFileW(pwszFilename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwAttributes, NULL);
	}

	DWORD err = GetLastError();

	// If we failed and FILE_FLAG_NO_BUFFERING was set, strip it and try again.
	// VPC and Novell shares sometimes do this....
	if (mhFile == INVALID_HANDLE_VALUE && err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) {
		if (dwAttributes & FILE_FLAG_NO_BUFFERING) {
			dwAttributes &= ~FILE_FLAG_NO_BUFFERING;
			dwAttributes |= FILE_FLAG_WRITE_THROUGH;

			if (pszFilename)
				mhFile = CreateFileA(pszFilename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwAttributes, NULL);
			else
				mhFile = CreateFileW(pwszFilename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwAttributes, NULL);

			err = GetLastError();
		}
	}

	// INVALID_HANDLE_VALUE isn't NULL.  *sigh*

	if (mhFile == INVALID_HANDLE_VALUE) {
		mhFile = NULL;

		if (!throwOnError)
			return false;

		throw MyWin32Error("Cannot open file \"%ls\":\n%%s", err, mpFilename.get());
	}

	mFilePosition = 0;
	return true;
}

bool VDFile::closeNT() {
	if (mhFile) {
		HANDLE h = mhFile;
		mhFile = NULL;
		if (!CloseHandle(h))
			return false;
	}

	return true;
}

void VDFile::close() {
	if (!closeNT())
		throw MyWin32Error("Cannot complete file \"%ls\": %%s", GetLastError(), mpFilename.get());
}

bool VDFile::truncateNT() {
	return 0 != SetEndOfFile(mhFile);
}

void VDFile::truncate() {
	if (!truncateNT())
		throw MyWin32Error("Cannot truncate file \"%ls\": %%s", GetLastError(), mpFilename.get());
}

bool VDFile::extendValidNT(sint64 pos) {
	if (GetVersion() & 0x80000000)
		return true;				// No need, Windows 95/98/ME do this automatically anyway.

	// The SetFileValidData() API is only available on XP and Server 2003.

	typedef BOOL (APIENTRY *tpSetFileValidData)(HANDLE hFile, LONGLONG ValidDataLength);		// Windows XP, Server 2003
	static tpSetFileValidData pSetFileValidData = (tpSetFileValidData)GetProcAddress(GetModuleHandle("kernel32"), "SetFileValidData");

	if (!pSetFileValidData) {
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return false;
	}

	return 0 != pSetFileValidData(mhFile, pos);
}

void VDFile::extendValid(sint64 pos) {
	if (!extendValidNT(pos))
		throw MyWin32Error("Cannot extend file \"%ls\": %%s", GetLastError(), mpFilename.get());
}

bool VDFile::enableExtendValid() {
	if (GetVersion() & 0x80000000)
		return true;				// Not Windows NT, no privileges involved

	// SetFileValidData() requires the SE_MANAGE_VOLUME_NAME privilege, so we must enable it
	// on the process token. We don't attempt to strip the privilege afterward as that would
	// introduce race conditions.
	bool bSuccessful = false;
	DWORD err = 0;

	SetLastError(0);

	HANDLE h;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &h)) {
		LUID luid;

		if (LookupPrivilegeValue(NULL, SE_MANAGE_VOLUME_NAME, &luid)) {
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if (AdjustTokenPrivileges(h, FALSE, &tp, 0, NULL, NULL))
				bSuccessful = true;
			else
				err = GetLastError();
		}

		CloseHandle(h);
	}

	if (!bSuccessful && err)
		SetLastError(err);

	return bSuccessful;
}

long VDFile::readData(void *buffer, long length) {
	DWORD dwActual;

	if (!ReadFile(mhFile, buffer, (DWORD)length, &dwActual, NULL))
		throw MyWin32Error("Cannot read from file \"%ls\": %%s", GetLastError(), mpFilename.get());

	mFilePosition += dwActual;

	return dwActual;
}

void VDFile::read(void *buffer, long length) {
	if (length != readData(buffer, length))
		throw MyWin32Error("Cannot read from file \"%ls\": Premature end of file.", GetLastError(), mpFilename.get());
}

long VDFile::writeData(const void *buffer, long length) {
	DWORD dwActual;
	bool success = false;

	if (!WriteFile(mhFile, buffer, (DWORD)length, &dwActual, NULL) || dwActual != (DWORD)length)
		goto found_error;

	mFilePosition += dwActual;

	return dwActual;

found_error:
	throw MyWin32Error("Cannot write to file \"%ls\": %%s", GetLastError(), mpFilename.get());
}

void VDFile::write(const void *buffer, long length) {
	if (length != writeData(buffer, length))
		throw MyWin32Error("Cannot write to file \"%ls\": Unable to write all data.", GetLastError(), mpFilename.get());
}

bool VDFile::seekNT(sint64 newPos, eSeekMode mode) {
	DWORD dwMode;

	switch(mode) {
	case kSeekStart:
		dwMode = FILE_BEGIN;
		break;
	case kSeekCur:
		dwMode = FILE_CURRENT;
		break;
	case kSeekEnd:
		dwMode = FILE_END;
		break;
	default:
		VDNEVERHERE;
		return false;
	}

	union {
		sint64 pos;
		LONG l[2];
	} u = { newPos };

	u.l[0] = SetFilePointer(mhFile, u.l[0], &u.l[1], dwMode);

	if (u.l[0] == -1 && GetLastError() != NO_ERROR)
		return false;

	mFilePosition = u.pos;
	return true;
}

void VDFile::seek(sint64 newPos, eSeekMode mode) {
	if (!seekNT(newPos, mode))
		throw MyWin32Error("Cannot seek within file \"%ls\": %%s", GetLastError(), mpFilename.get());
}

bool VDFile::skipNT(sint64 delta) {
	if (!delta)
		return true;

	char buf[1024];

	if (delta <= sizeof buf) {
		return (long)delta == readData(buf, (long)delta);
	} else
		return seekNT(delta, kSeekCur);
}

void VDFile::skip(sint64 delta) {
	if (!delta)
		return;

	char buf[1024];

	if (delta > 0 && delta <= sizeof buf) {
		if ((long)delta != readData(buf, (long)delta))
			throw MyWin32Error("Cannot seek within file \"%ls\": %%s", GetLastError(), mpFilename.get());
	} else
		seek(delta, kSeekCur);
}

sint64 VDFile::size() {
	union {
		uint64 siz;
		DWORD l[2];
	} u;

	u.l[0] = GetFileSize(mhFile, &u.l[1]);

	DWORD err;

	if (u.l[0] == (DWORD)-1L && (err = GetLastError()) != NO_ERROR)
		throw MyWin32Error("Cannot retrieve size of file \"%ls\": %%s", GetLastError(), mpFilename.get());

	return (sint64)u.siz;
}

sint64 VDFile::tell() {
	return mFilePosition;
}

bool VDFile::isOpen() {
	return mhFile != 0;
}

VDFileHandle VDFile::getRawHandle() {
	return mhFile;
}

void *VDFile::AllocUnbuffer(size_t nBytes) {
	return VirtualAlloc(NULL, nBytes, MEM_COMMIT, PAGE_READWRITE);
}

void VDFile::FreeUnbuffer(void *p) {
	VirtualFree(p, 0, MEM_RELEASE);
}

///////////////////////////////////////////////////////////////////////////////

VDFileStream::~VDFileStream() {
}

const wchar_t *VDFileStream::GetNameForError() {
	return getFilenameForError();
}

sint64 VDFileStream::Pos() {
	return tell();
}

void VDFileStream::Read(void *buffer, sint32 bytes) {
	read(buffer, bytes);
}

sint32 VDFileStream::ReadData(void *buffer, sint32 bytes) {
	return readData(buffer, bytes);
}

void VDFileStream::Write(const void *buffer, sint32 bytes) {
	write(buffer, bytes);
}

sint64 VDFileStream::Length() {
	return size();
}

void VDFileStream::Seek(sint64 offset) {
	seek(offset);
}

///////////////////////////////////////////////////////////////////////////////

VDMemoryStream::VDMemoryStream(const void *pSrc, uint32 len) 
	: mpSrc((const char *)pSrc)
	, mPos(0)
	, mLength(len)
{
}

const wchar_t *VDMemoryStream::GetNameForError() {
	return L"memory stream";
}

sint64 VDMemoryStream::Pos() {
	return mPos;
}

void VDMemoryStream::Read(void *buffer, sint32 bytes) {
	if (bytes != ReadData(buffer, bytes))
		throw MyError("Attempt to read beyond stream.");
}

sint32 VDMemoryStream::ReadData(void *buffer, sint32 bytes) {
	if (bytes <= 0)
		return 0;

	if (bytes + mPos > mLength)
		bytes = mLength - mPos;

	if (bytes > 0) {
		memcpy(buffer, mpSrc+mPos, bytes);
		mPos += bytes;
	}

	return bytes;
}

void VDMemoryStream::Write(const void *buffer, sint32 bytes) {
	throw MyError("Memory streams are read-only.");
}

sint64 VDMemoryStream::Length() {
	return mLength;
}

void VDMemoryStream::Seek(sint64 offset) {
	if (offset < 0 || offset > mLength)
		throw MyError("Invalid seek position");

	mPos = (uint32)offset;
}

///////////////////////////////////////////////////////////////////////////////

VDBufferedStream::VDBufferedStream(IVDRandomAccessStream *pSrc, uint32 bufferSize)
	: mpSrc(pSrc)
	, mBuffer(bufferSize)
	, mBasePosition(0)
	, mBufferOffset(0)
	, mBufferValidSize(0)
{
}

VDBufferedStream::~VDBufferedStream() {
}

const wchar_t *VDBufferedStream::GetNameForError() {
	return mpSrc->GetNameForError();
}

sint64 VDBufferedStream::Pos() {
	return mBasePosition + mBufferOffset;
}

void VDBufferedStream::Read(void *buffer, sint32 bytes) {
	if (bytes != ReadData(buffer, bytes))
		throw MyError("Cannot read %d bytes at location %08llx from %ls", bytes, mBasePosition + mBufferOffset, mpSrc->GetNameForError());
}

sint32 VDBufferedStream::ReadData(void *buffer, sint32 bytes) {
	if (bytes <= 0)
		return 0;

	uint32 actual = 0;
	for(;;) {
		uint32 tc = mBufferValidSize - mBufferOffset;

		if (tc > (uint32)bytes)
			tc = (uint32)bytes;

		if (tc) {
			if (buffer) {
				memcpy(buffer, mBuffer.data() + mBufferOffset, tc);
				buffer = (char *)buffer + tc;
			}

			mBufferOffset += tc;
			bytes -= tc;
			actual += tc;

			if (!bytes)
				break;
		}

		// At this point, the buffer is empty.
		if (mBufferValidSize) {
			VDASSERT(mBufferOffset >= mBufferValidSize);

			mBasePosition += mBufferValidSize;
			mBufferOffset = 0;
			mBufferValidSize = 0;
		}

		// If the remaining read is large, issue it directly to the underlying stream.
		if (buffer && (uint32)bytes >= mBuffer.size() * 2) {
			sint32 localActual = mpSrc->ReadData(buffer, bytes);
			mBasePosition += localActual;
			actual += localActual;
			break;
		}

		// Refill the buffer.
		mBufferValidSize = mpSrc->ReadData(mBuffer.data(), mBuffer.size());
		mBufferOffset = 0;
		if (!mBufferValidSize)
			break;
	}

	return actual;
}

void VDBufferedStream::Write(const void *buffer, sint32 bytes) {
	throw MyError("Buffered streams are read-only.");
}

sint64 VDBufferedStream::Length() {
	return mpSrc->Length();
}

void VDBufferedStream::Seek(sint64 offset) {
	// check if an in-buffer skip is possible
	sint64 relativeOffset = offset - mBasePosition;
	if (relativeOffset >= 0 && relativeOffset <= (sint64)mBufferValidSize) {
		mBufferOffset = (uint32)relativeOffset;
		return;
	}

	// flush buffer
	mBufferOffset = 0;
	mBufferValidSize = 0;

	// issue seek
	mpSrc->Seek(offset);
	mBasePosition = offset;
}

void VDBufferedStream::Skip(sint64 size) {
	sint64 targetPos = mBasePosition + mBufferOffset + size;
	sint64 bufferEnd = mBasePosition + mBufferValidSize;

	// check if we can do a buffered skip
	if (targetPos >= bufferEnd && targetPos < bufferEnd + (sint64)mBuffer.size()) {
		Read(NULL, (sint32)size);
		return;
	}

	// issue a seek
	Seek(targetPos);
}

///////////////////////////////////////////////////////////////////////////////

VDTextStream::VDTextStream(IVDStream *pSrc)
	: mpSrc(pSrc)
	, mBufferPos(0)
	, mBufferLimit(0)
	, mState(kFetchLine)
	, mFileBuffer(kFileBufferSize)
{
}

VDTextStream::~VDTextStream() {
}

const char *VDTextStream::GetNextLine() {
	if (!mpSrc)
		return NULL;

	mLineBuffer.clear();

	for(;;) {
		if (mBufferPos >= mBufferLimit) {
			mBufferPos = 0;
			mBufferLimit = mpSrc->ReadData(mFileBuffer.data(), mFileBuffer.size());

			if (!mBufferLimit) {
				mpSrc = NULL;

				if (mLineBuffer.empty())
					return NULL;

				mLineBuffer.push_back(0);

				return mLineBuffer.data();
			}
		}

		switch(mState) {

			case kEatNextIfCR:
				mState = kFetchLine;
				if (mFileBuffer[mBufferPos] == '\r')
					++mBufferPos;
				continue;

			case kEatNextIfLF:
				mState = kFetchLine;
				if (mFileBuffer[mBufferPos] == '\n')
					++mBufferPos;
				continue;

			case kFetchLine:
				uint32 base = mBufferPos;

				do {
					const char c = mFileBuffer[mBufferPos++];

					if (c == '\r') {
						mState = kEatNextIfLF;
						mLineBuffer.insert(mLineBuffer.end(), mFileBuffer.begin() + base, mFileBuffer.begin() + (mBufferPos-1));
						mLineBuffer.push_back(0);
						return mLineBuffer.data();
					}
					if (c == '\n') {
						mState = kEatNextIfCR;
						mLineBuffer.insert(mLineBuffer.end(), mFileBuffer.begin() + base, mFileBuffer.begin() + (mBufferPos-1));
						mLineBuffer.push_back(0);
						return mLineBuffer.data();
					}
				} while(mBufferPos < mBufferLimit);
				mLineBuffer.insert(mLineBuffer.end(), mFileBuffer.begin() + base, mFileBuffer.begin() + mBufferLimit);
				break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

VDTextInputFile::VDTextInputFile(const wchar_t *filename, uint32 flags)
	: mFileStream(filename, flags | nsVDFile::kRead)
	, mTextStream(&mFileStream)
{
}

VDTextInputFile::~VDTextInputFile() {
}

///////////////////////////////////////////////////////////////////////////////

VDTextOutputStream::VDTextOutputStream(IVDStream *stream)
	: mpDst(stream)
	, mLevel(0)
{
}

VDTextOutputStream::~VDTextOutputStream() {
	try { 
		Flush();
	} catch(const MyError&) {
		// ignore errors in destructor
	}
}

void VDTextOutputStream::Flush() {
	if (mLevel) {
		mpDst->Write(mBuf, mLevel);
		mLevel = 0;
	}
}

void VDTextOutputStream::Write(const char *s, int len) {
	PutData(s, len);
}

void VDTextOutputStream::PutLine() {
	PutData("\r\n", 2);
}

void VDTextOutputStream::PutLine(const char *s) {
	PutData(s, strlen(s));
	PutData("\r\n", 2);
}

void VDTextOutputStream::FormatLine(const char *format, ...) {
	va_list val;

	va_start(val, format);

	int rv = -1;
	if (mLevel < kBufSize-4)
		rv = _vsnprintf(mBuf+mLevel, kBufSize-mLevel, format, val);

	if (rv >= 0)
		mLevel += rv;
	else
		FormatLine2(format, val);

	PutData("\r\n", 2);
	va_end(val);
}

void VDTextOutputStream::FormatLine2(const char *format, va_list val) {
	char buf[3072];

	int rv = _vsnprintf(buf, 3072, format, val);
	if (rv > 0)
		PutData(buf, rv);
}

void VDTextOutputStream::PutData(const char *s, int len) {
	while(len > 0) {
		int left = kBufSize - mLevel;
		if (!left) {
			mpDst->Write(mBuf, kBufSize);
			mLevel = 0;
			left = kBufSize;
		}

		int tc = len;

		if (tc > left)
			tc = left;

		memcpy(mBuf + mLevel, s, tc);

		s += tc;
		len -= tc;
		mLevel += tc;
	}
}
