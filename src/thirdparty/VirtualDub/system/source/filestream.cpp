//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2012 Avery Lee, All Rights Reserved.
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
#include <vd2/system/file.h>

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

void VDTextOutputStream::Write(const char *s) {
	PutData(s, strlen(s));
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

void VDTextOutputStream::Format(const char *format, ...) {
	va_list val;

	va_start(val, format);

	int rv = -1;
	if (mLevel < kBufSize-4)
		rv = _vsnprintf(mBuf+mLevel, kBufSize-mLevel, format, val);

	if (rv >= 0)
		mLevel += rv;
	else
		Format2(format, val);

	va_end(val);
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
		Format2(format, val);

	PutData("\r\n", 2);
	va_end(val);
}

void VDTextOutputStream::Format2(const char *format, va_list val) {
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
