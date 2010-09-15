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
#include <wtypes.h>
#include <winnt.h>
#include <intrin.h>
#include <vd2/system/cpuaccel.h>

static long g_lCPUExtensionsEnabled;
static long g_lCPUExtensionsAvailable;

extern "C" {
	bool FPU_enabled, MMX_enabled, SSE_enabled, ISSE_enabled, SSE2_enabled;
};


#ifdef _M_AMD64

	long CPUCheckForExtensions() {
		long flags = CPUF_SUPPORTS_FPU;

		// This code used to use IsProcessorFeaturePresent(), but this function is somewhat
		// suboptimal in Win64 -- for one thing, it doesn't return true for MMX, at least
		// on Vista 64.

		// check for SSE3, SSSE3, SSE4.1
		int cpuInfo[4];
		__cpuid(cpuInfo, 1);

		if (cpuInfo[3] & (1 << 23))
			flags |= CPUF_SUPPORTS_MMX;

		if (cpuInfo[3] & (1 << 25))
			flags |= CPUF_SUPPORTS_SSE | CPUF_SUPPORTS_INTEGER_SSE;

		if (cpuInfo[3] & (1 << 26))
			flags |= CPUF_SUPPORTS_SSE2;

		if (cpuInfo[2] & 0x00000001)
			flags |= CPUF_SUPPORTS_SSE3;

		if (cpuInfo[2] & 0x00000200)
			flags |= CPUF_SUPPORTS_SSSE3;

		if (cpuInfo[2] & 0x00080000)
			flags |= CPUF_SUPPORTS_SSE41;

		// check for 3DNow!, 3DNow! extensions
		__cpuid(cpuInfo, 0x80000000);
		if (cpuInfo[0] >= 0x80000001) {
			__cpuid(cpuInfo, 0x80000001);

			if (cpuInfo[3] & (1 << 31))
				flags |= CPUF_SUPPORTS_3DNOW;

			if (cpuInfo[3] & (1 << 30))
				flags |= CPUF_SUPPORTS_3DNOW_EXT;

			if (cpuInfo[3] & (1 << 22))
				flags |= CPUF_SUPPORTS_INTEGER_SSE;
		}

		return flags;
	}

#else

	// This is ridiculous.

	static long CPUCheckForSSESupport() {
		__try {
	//		__asm andps xmm0,xmm0

			__asm _emit 0x0f
			__asm _emit 0x54
			__asm _emit 0xc0

		} __except(EXCEPTION_EXECUTE_HANDLER) {
			if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
				g_lCPUExtensionsAvailable &= ~(CPUF_SUPPORTS_SSE|CPUF_SUPPORTS_SSE2|CPUF_SUPPORTS_SSE3|CPUF_SUPPORTS_SSSE3);
		}

		return g_lCPUExtensionsAvailable;
	}

	long __declspec(naked) CPUCheckForExtensions() {
		__asm {
			push	ebp
			push	edi
			push	esi
			push	ebx

			xor		ebp,ebp			;cpu flags - if we don't have CPUID, we probably
									;won't want to try FPU optimizations.

			;check for CPUID.

			pushfd					;flags -> EAX
			pop		eax
			or		eax,00200000h	;set the ID bit
			push	eax				;EAX -> flags
			popfd
			pushfd					;flags -> EAX
			pop		eax
			and		eax,00200000h	;ID bit set?
			jz		done			;nope...

			;CPUID exists, check for features register.

			mov		ebp,00000003h
			xor		eax,eax
			cpuid
			or		eax,eax
			jz		done			;no features register?!?

			;features register exists, look for MMX, SSE, SSE2.

			mov		eax,1
			cpuid
			mov		ebx,edx
			and		ebx,00800000h	;MMX is bit 23 of EDX
			shr		ebx,21
			or		ebp,ebx			;set bit 2 if MMX exists

			mov		ebx,edx
			and		edx,02000000h	;SSE is bit 25 of EDX
			shr		edx,25
			neg		edx
			and		edx,00000018h	;set bits 3 and 4 if SSE exists
			or		ebp,edx

			and		ebx,04000000h	;SSE2 is bit 26 of EDX
			shr		ebx,21
			and		ebx,00000020h	;set bit 5
			or		ebp,ebx

			test	ecx, 1			;SSE3 is bit 0 of ECX
			jz		no_sse3
			or		ebp, 100h
no_sse3:

			test	ecx, 200h		;SSSE3 is bit 9 of ECX
			jz		no_ssse3
			or		ebp, 200h
no_ssse3:

			test	ecx, 80000h		;SSE4_1 is bit 19 of ECX
			jz		no_sse4_1
			or		ebp, 400h
no_sse4_1:

			;check for vendor feature register (K6/Athlon).

			mov		eax,80000000h
			cpuid
			mov		ecx,80000001h
			cmp		eax,ecx
			jb		done

			;vendor feature register exists, look for 3DNow! and Athlon extensions

			mov		eax,ecx
			cpuid

			mov		eax,edx
			and		edx,80000000h	;3DNow! is bit 31
			shr		edx,25
			or		ebp,edx			;set bit 6

			mov		edx,eax
			and		eax,40000000h	;3DNow!2 is bit 30
			shr		eax,23
			or		ebp,eax			;set bit 7

			and		edx,00400000h	;AMD MMX extensions (integer SSE) is bit 22
			shr		edx,19
			or		ebp,edx

	done:
			mov		eax,ebp
			mov		g_lCPUExtensionsAvailable, ebp

			;Full SSE and SSE-2 require OS support for the xmm* registers.

			test	eax,00000030h
			jz		nocheck
			call	CPUCheckForSSESupport
	nocheck:
			pop		ebx
			pop		esi
			pop		edi
			pop		ebp
			ret
		}
	}

#endif

long CPUEnableExtensions(long lEnableFlags) {
	g_lCPUExtensionsEnabled = lEnableFlags;

	MMX_enabled = !!(g_lCPUExtensionsEnabled & CPUF_SUPPORTS_MMX);
	FPU_enabled = !!(g_lCPUExtensionsEnabled & CPUF_SUPPORTS_FPU);
	SSE_enabled = !!(g_lCPUExtensionsEnabled & CPUF_SUPPORTS_SSE);
	ISSE_enabled = !!(g_lCPUExtensionsEnabled & CPUF_SUPPORTS_INTEGER_SSE);
	SSE2_enabled = !!(g_lCPUExtensionsEnabled & CPUF_SUPPORTS_SSE2);

	return g_lCPUExtensionsEnabled;
}

long CPUGetAvailableExtensions() {
	return g_lCPUExtensionsAvailable;
}

long CPUGetEnabledExtensions() {
	return g_lCPUExtensionsEnabled;
}

void VDCPUCleanupExtensions() {
#ifndef _M_AMD64
	if (ISSE_enabled)
		__asm sfence
	if (MMX_enabled)
		__asm emms
#else
	_mm_sfence();
#endif
}