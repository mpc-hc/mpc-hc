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
#include <list>
#include <utility>
#include <vd2/system/log.h>
#include <vd2/system/thread.h>
#include <vd2/system/VDString.h>

namespace {
	wchar_t		g_log[16384];			// 32K log
	int			g_logHead, g_logTail;
	VDCriticalSection	g_csLog;

	typedef std::list<std::pair<IVDLogger *, VDThreadID> > tVDLoggers;
	tVDLoggers g_loggers;
}

void VDLog(int severity, const VDStringW& s) {
	VDLog(severity, s.c_str());
}

void VDLog(int severity, const wchar_t *s) {
	int strSize = wcslen(s) + 1;

	if (strSize >= 16384) {
		VDASSERT(false);
		return;
	}

	vdsynchronized(g_csLog) {
		for(;;) {
			int currentSize = (g_logTail - g_logHead) & 16383;

			if (currentSize + strSize < 16384)	// NOTE: This means that the last byte in the ring buffer can never be used.
				break;

			while(g_log[g_logHead++ & 16383])
				;

			g_logHead &= 16383;
		}

		const wchar_t *ps = s;

		g_log[g_logTail++] = severity;

		for(int i=1; i<strSize; ++i)
			g_log[g_logTail++ & 16383] = *ps++;

		g_log[g_logTail++ & 16383] = 0;

		g_logTail &= 16383;

		VDThreadID currentThread = VDGetCurrentThreadID();
		for(tVDLoggers::const_iterator it(g_loggers.begin()), itEnd(g_loggers.end()); it!=itEnd; ++it) {
			if (!(*it).second || currentThread == (*it).second)
				(*it).first->AddLogEntry(severity, s);
		}
	}
}

void VDLogF(int severity, const wchar_t *format, ...) {
	va_list val;
	va_start(val, format);
	VDStringW s;
	s.append_vsprintf(format, val);
	va_end(val);

	VDLog(severity, s);
}

void VDAttachLogger(IVDLogger *pLogger, bool bThisThreadOnly, bool bReplayLog) {
	vdsynchronized(g_csLog) {
		g_loggers.push_back(tVDLoggers::value_type(pLogger, bThisThreadOnly ? VDGetCurrentThreadID() : 0));

		if (bReplayLog) {
			int idx = g_logHead;

			while(idx != g_logTail) {
				int severity = g_log[idx++];
				int headidx = idx;

				idx &= 16383;

				for(;;) {
					wchar_t c = g_log[idx];

					idx = (idx+1) & 16383;

					if (!c)
						break;
				}

				if (idx > headidx) {
					pLogger->AddLogEntry(severity, VDStringW(g_log + headidx, idx-headidx-1).c_str());
				} else {
					VDStringW t(idx+16383-headidx);

					std::copy(g_log + headidx, g_log + 16384, const_cast<wchar_t *>(t.data()));
					std::copy(g_log, g_log + idx - 1, const_cast<wchar_t *>(t.data() + (16384 - headidx)));
					pLogger->AddLogEntry(severity, t.c_str());
				}
			}
		}
	}
}

void VDDetachLogger(IVDLogger *pLogger) {
	vdsynchronized(g_csLog) {
		for(tVDLoggers::iterator it(g_loggers.begin()), itEnd(g_loggers.end()); it!=itEnd; ++it) {
			if (pLogger == (*it).first) {
				g_loggers.erase(it);
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	autologger
//
///////////////////////////////////////////////////////////////////////////

VDAutoLogger::VDAutoLogger(int min_severity)
	: mbAttached(true)
	, mMinSeverity(min_severity)
{
	VDAttachLogger(this, false, false);
}

VDAutoLogger::~VDAutoLogger() {
	if (mbAttached)
		VDDetachLogger(this);
}

void VDAutoLogger::AddLogEntry(int severity, const wchar_t *s) {
	if (severity >= mMinSeverity)
		mEntries.push_back(Entry(severity, s));
}

const VDAutoLogger::tEntries& VDAutoLogger::GetEntries() {
	if (mbAttached) {
		VDDetachLogger(this);
		mbAttached = false;
	}

	return mEntries;
}

