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

#ifndef f_VD2_ERROR_H
#define f_VD2_ERROR_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>

class MyError;

///////////////////////////////////////////////////////////////////////////
//	IVDAsyncErrorCallback
//
class IVDAsyncErrorCallback {
public:
	virtual bool OnAsyncError(MyError& e) = 0;
};

///////////////////////////////////////////////////////////////////////////
//	MyError
//
class MyError {
private:
	const MyError& operator=(const MyError&);		// protect against accidents

protected:
	char *buf;

public:
	MyError();
	MyError(const MyError& err);
	MyError(const char *f, ...);
	~MyError();
	void clear();
	void assign(const MyError& e);
	void assign(const char *s);
	void setf(const char *f, ...);
	void vsetf(const char *f, va_list val);
	void post(struct HWND__ *hWndParent, const char *title) const;
	char *gets() const {
		return buf;
	}
	char *c_str() const {
		return buf;
	}
	bool empty() const { return !buf; }
	void discard();
	void swap(MyError& err);
	void TransferFrom(MyError& err);
};

class MyICError : public MyError {
public:
	MyICError(const char *s, uint32 icErr);
	MyICError(uint32 icErr, const char *format, ...);
};

class MyMMIOError : public MyError {
public:
	MyMMIOError(const char *s, uint32 icErr);
};

class MyAVIError : public MyError {
public:
	MyAVIError(const char *s, uint32 aviErr);
};

class MyMemoryError : public MyError {
public:
	MyMemoryError();
	MyMemoryError(size_t attemptedSize);
};

class MyWin32Error : public MyError {
public:
	MyWin32Error(const char *format, uint32 err, ...);

	uint32 GetWin32Error() const { return mWin32Error; }

protected:
	const uint32 mWin32Error;
};

class MyCrashError : public MyError {
public:
	MyCrashError(const char *format, uint32 dwExceptionCode);
};

class MyUserAbortError : public MyError {
public:
	MyUserAbortError();
};

class MyInternalError : public MyError {
public:
	MyInternalError(const char *format, ...);
};

#endif
