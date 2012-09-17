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

#include <windows.h>
#include <vd2/system/win32/intrin.h>

#include <vd2/system/vdtypes.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/system/debug.h>
#include <vd2/system/thread.h>

#ifdef _DEBUG

class VDSafeMessageBoxThreadW32 : public VDThread {
public:
	VDSafeMessageBoxThreadW32(HWND hwndParent, const char *pszText, const char *pszCaption, DWORD dwFlags)
		: mhwndParent(hwndParent)
		, mpszText(pszText)
		, mpszCaption(pszCaption)
		, mdwFlags(dwFlags)
	{
	}

	DWORD GetResult() const { return mdwResult; }

protected:
	void ThreadRun() {
		mdwResult = MessageBox(mhwndParent, mpszText, mpszCaption, mdwFlags);
	}

	HWND mhwndParent;
	const char *const mpszText;
	const char *const mpszCaption;
	const DWORD mdwFlags;
	DWORD mdwResult;
};

UINT VDSafeMessageBoxW32(HWND hwndParent, const char *pszText, const char *pszCaption, DWORD dwFlags) {
	VDSafeMessageBoxThreadW32 mbox(hwndParent, pszText, pszCaption, dwFlags);

	mbox.ThreadStart();
	mbox.ThreadWait();
	return mbox.GetResult();
}

VDAssertResult VDAssert(const char *exp, const char *file, int line) {
	DWORD dwOldError = GetLastError();
	char szText[1024];

	VDDEBUG("%s(%d): Assert failed: %s\n", file, line, exp);

	wsprintf(szText,
		"Assert failed in module %s, line %d:\n"
		"\n"
		"\t%s\n"
		"\n"
		"Break into debugger?", file, line, exp);

	UINT result = VDSafeMessageBoxW32(NULL, szText, "Assert failure", MB_ABORTRETRYIGNORE|MB_ICONWARNING|MB_TASKMODAL);

	SetLastError(dwOldError);

	switch(result) {
	case IDABORT:
		::Sleep(250);				// Pause for a moment so the VC6 debugger doesn't freeze.
		return kVDAssertBreak;
	case IDRETRY:
		return kVDAssertContinue;
	default:
		VDNEVERHERE;
	case IDIGNORE:
		return kVDAssertIgnore;
	}
}

VDAssertResult VDAssertPtr(const char *exp, const char *file, int line) {
	DWORD dwOldError = GetLastError();
	char szText[1024];

	VDDEBUG("%s(%d): Assert failed: %s is not a valid pointer\n", file, line, exp);

	wsprintf(szText,
		"Assert failed in module %s, line %d:\n"
		"\n"
		"\t(%s) not a valid pointer\n"
		"\n"
		"Break into debugger?", file, line, exp);

	UINT result = VDSafeMessageBoxW32(NULL, szText, "Assert failure", MB_ABORTRETRYIGNORE|MB_ICONWARNING|MB_TASKMODAL);

	SetLastError(dwOldError);

	switch(result) {
	case IDABORT:
		return kVDAssertBreak;
	case IDRETRY:
		return kVDAssertContinue;
	default:
		VDNEVERHERE;
	case IDIGNORE:
		return kVDAssertIgnore;
	}
}

#endif

void VDProtectedAutoScopeICLWorkaround() {}

void VDDebugPrint(const char *format, ...) {
	char buf[4096];

	va_list val;
	va_start(val, format);
	_vsnprintf(buf, sizeof buf, format, val);
	va_end(val);
	Sleep(0);
	OutputDebugString(buf);
}

///////////////////////////////////////////////////////////////////////////

namespace {
	IVDExternalCallTrap *g_pExCallTrap;
}

void VDSetExternalCallTrap(IVDExternalCallTrap *trap) {
	g_pExCallTrap = trap;
}

#if defined(WIN32) && defined(_M_IX86) && defined(__MSC_VER)
	namespace {
		bool IsFPUStateOK(unsigned& ctlword) {
			ctlword = 0;

			__asm mov eax, ctlword
			__asm fnstcw [eax]

			ctlword &= 0x0f3f;

			return ctlword == 0x023f;
		}

		void ResetFPUState() {
			static const unsigned ctlword = 0x027f;

			__asm fnclex
			__asm fldcw ctlword
		}

		bool IsSSEStateOK(uint32& ctlword) {
			ctlword = _mm_getcsr();

			// Intel C/C++ flips FTZ and DAZ. :(
			return (ctlword & 0x7f80) == 0x1f80;
		}

		void ResetSSEState() {
			_mm_setcsr(0x1f80);
		}
	}

	bool IsMMXState() {
		char	buf[28];
		unsigned short tagword;

		__asm fnstenv buf		// this resets the FPU control word somehow!?

		tagword = *(unsigned short *)(buf + 8);

		return (tagword != 0xffff);
	}
	void ClearMMXState() {
		if (MMX_enabled)
			__asm emms
		else {
			__asm {
				ffree st(0)
				ffree st(1)
				ffree st(2)
				ffree st(3)
				ffree st(4)
				ffree st(5)
				ffree st(6)
				ffree st(7)
			}
		}
	}

	void VDClearEvilCPUStates() {
		ResetFPUState();
		ClearMMXState();
	}

	void VDPreCheckExternalCodeCall(const char *file, int line) {
		unsigned fpucw;
		uint32 mxcsr;
		bool bFPUStateBad = !IsFPUStateOK(fpucw);
		bool bSSEStateBad = SSE_enabled && !IsSSEStateOK(mxcsr);
		bool bMMXStateBad = IsMMXState();

		if (bMMXStateBad || bFPUStateBad || bSSEStateBad) {
			ClearMMXState();
			ResetFPUState();
			if (SSE_enabled)
				ResetSSEState();
		}

		if (g_pExCallTrap) {
			if (bMMXStateBad)
				g_pExCallTrap->OnMMXTrap(NULL, file, line);

			if (bFPUStateBad)
				g_pExCallTrap->OnFPUTrap(NULL, file, line, fpucw);

			if (bSSEStateBad)
				g_pExCallTrap->OnSSETrap(NULL, file, line, mxcsr);
		}
	}

	void VDPostCheckExternalCodeCall(const wchar_t *mpContext, const char *mpFile, int mLine) {
		unsigned fpucw;
		uint32 mxcsr;
		bool bFPUStateBad = !IsFPUStateOK(fpucw);
		bool bSSEStateBad = SSE_enabled && !IsSSEStateOK(mxcsr);
		bool bMMXStateBad = IsMMXState();
		bool bBadState = bMMXStateBad || bFPUStateBad || bSSEStateBad;

		if (bBadState) {
			ClearMMXState();
			ResetFPUState();
			if (SSE_enabled)
				ResetSSEState();
		}

		if (g_pExCallTrap) {
			if (bMMXStateBad)
				g_pExCallTrap->OnMMXTrap(mpContext, mpFile, mLine);

			if (bFPUStateBad)
				g_pExCallTrap->OnFPUTrap(mpContext, mpFile, mLine, fpucw);

			if (bSSEStateBad)
				g_pExCallTrap->OnSSETrap(mpContext, mpFile, mLine, mxcsr);
		}
	}

#else

	bool IsMMXState() {
		return false;
	}

	void ClearMMXState() {
	}

	void VDClearEvilCPUStates() {
	}

	void VDPreCheckExternalCodeCall(const char *file, int line) {
	}

	void VDPostCheckExternalCodeCall(const wchar_t *mpContext, const char *mpFile, int mLine) {
	}

#endif
