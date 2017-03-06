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
#include <stdio.h>
#include <stdarg.h>

#include <vd2/system/vdtypes.h>
#include <vd2/system/Error.h>
#include <vd2/system/log.h>

MyError::MyError() {
	buf = NULL;
}

MyError::MyError(const MyError& err) {
	buf = _strdup(err.buf);
}

MyError::MyError(const char *f, ...)
	: buf(NULL)
{
	va_list val;

	va_start(val, f);
	vsetf(f, val);
	va_end(val);
}

MyError::~MyError() {
	free(buf);
}

void MyError::clear() {
	if (buf)			// we do this check because debug free() always does a heapchk even if buf==NULL
		free(buf);
	buf = NULL;
}

void MyError::assign(const MyError& e) {
	if (buf)
		free(buf);
	buf = _strdup(e.buf);
}

void MyError::assign(const char *s) {
	if (buf)
		free(buf);
	buf = _strdup(s);
}

void MyError::setf(const char *f, ...) {
	va_list val;

	va_start(val, f);
	vsetf(f,val);
	va_end(val);
}

void MyError::vsetf(const char *f, va_list val) {
	for(int size = 1024; size <= 32768; size += size) {
		free(buf);
		buf = NULL;

		buf = (char *)malloc(size);
		if (!buf)
			return;

		if ((unsigned)_vsnprintf(buf, size, f, val) < (unsigned)size)
			return;
	}

	free(buf);
	buf = NULL;
}

void MyError::post(HWND hWndParent, const char *title) const {
	if (!buf || !*buf)
		return;

	VDDEBUG("*** %s: %s\n", title, buf);
	//VDLog(kVDLogError, VDswprintf(L"Error: %hs", 1, &buf));

	MessageBox(hWndParent, buf, title, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

void MyError::discard() {
	free(buf);
	buf = NULL;
}

void MyError::swap(MyError& err) {
	char *s = err.buf;
	err.buf = buf;
	buf = s;
}

void MyError::TransferFrom(MyError& err) {
	if (buf)
		free(buf);

	buf = err.buf;
	err.buf = NULL;
}

MyMemoryError::MyMemoryError() {
	setf("Out of memory");
}

MyMemoryError::MyMemoryError(size_t requestedSize) {
	setf("Out of memory (unable to allocate %llu bytes)", (unsigned long long)requestedSize);
}

MyUserAbortError::MyUserAbortError() {
	buf = _strdup("");
}

MyInternalError::MyInternalError(const char *format, ...) {
	char buf[1024];
	va_list val;

	va_start(val, format);
	_vsnprintf(buf, (sizeof buf) - 1, format, val);
	buf[1023] = 0;
	va_end(val);

	setf("Internal error: %s", buf);
}
