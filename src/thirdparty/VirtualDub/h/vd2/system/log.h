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

#ifndef f_VD2_SYSTEM_LOG_H
#define f_VD2_SYSTEM_LOG_H

#include <vd2/system/VDString.h>
#include <list>

class IVDLogger {
public:
	virtual void AddLogEntry(int severity, const wchar_t *s) = 0;
};

enum {
	kVDLogInfo, kVDLogMarker, kVDLogWarning, kVDLogError
};

void VDLog(int severity, const wchar_t *format);
void VDLog(int severity, const VDStringW& s);
void VDLogF(int severity, const wchar_t *format, ...);
void VDAttachLogger(IVDLogger *pLogger, bool bThisThreadOnly, bool bReplayLog);
void VDDetachLogger(IVDLogger *pLogger);

class VDAutoLogger : public IVDLogger {
public:
	struct Entry {
		int severity;
		VDStringW text;

		Entry(int sev, const wchar_t *s) : severity(sev), text(s) {}
	};

	typedef std::list<Entry>	tEntries;

	VDAutoLogger(int min_severity);
	~VDAutoLogger();

	void AddLogEntry(int severity, const wchar_t *s);

	const tEntries& GetEntries();

protected:
	tEntries mEntries;
	const int mMinSeverity;
	bool	mbAttached;
};

#endif
