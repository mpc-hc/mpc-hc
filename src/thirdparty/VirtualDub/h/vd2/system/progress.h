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

#ifndef f_SYSTEM_PROGRESS_H
#define f_SYSTEM_PROGRESS_H

#include <vd2/system/error.h>

class VDAtomicInt;
class VDSignalPersistent;

class IProgress {
public:
	virtual void Error(const char *)=0;
	virtual void Warning(const char *)=0;
	virtual bool Query(const char *query, bool fDefault)=0;
	virtual void ProgressStart(const char *text, const char *caption, const char *progtext, long lMax)=0;
	virtual void ProgressAdvance(long)=0;
	virtual void ProgressEnd()=0;
	virtual void Output(const char *text)=0;
	virtual VDAtomicInt *ProgressGetAbortFlag()=0;
	virtual VDSignalPersistent *ProgressGetAbortSignal()=0;
};


void ProgressSetHandler(IProgress *pp);
IProgress *ProgressGetHandler();

bool ProgressCheckAbort();
void ProgressSetAbort(bool bNewValue);
VDSignalPersistent *ProgressGetAbortSignal();
void ProgressError(const class MyError&);
void ProgressWarning(const char *format, ...);
void ProgressOutput(const char *format, ...);
bool ProgressQuery(bool fDefault, const char *format, ...);
void ProgressStart(long lMax, const char *caption, const char *progresstext, const char *format, ...);
void ProgressAdvance(long lNewValue);
void ProgressEnd();


class VDProgress {
public:
	VDProgress(long lMax, const char *caption, const char *progresstext, const char *format, ...) {
		ProgressStart(lMax, caption, progresstext, format);
	}

	~VDProgress() {
		ProgressEnd();
	}

	void advance(long v) {
		ProgressAdvance(v);
	}
};

class VDProgressAbortable {
public:
	VDProgressAbortable(long lMax, const char *caption, const char *progresstext, const char *format, ...) {
		ProgressStart(lMax, caption, progresstext, format);
		ProgressSetAbort(false);
	}

	~VDProgressAbortable() {
		ProgressEnd();
	}

	void advance(long v) {
		if (ProgressCheckAbort())
			throw MyUserAbortError();
		ProgressAdvance(v);
	}
};

#endif
