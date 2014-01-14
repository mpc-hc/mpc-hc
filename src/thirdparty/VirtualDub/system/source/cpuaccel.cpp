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
#include <vd2/system/win32/intrin.h>
#include <vd2/system/cpuaccel.h>

static long g_lCPUExtensionsEnabled;
static long g_lCPUExtensionsAvailable;

extern "C" {
	bool FPU_enabled, MMX_enabled, SSE_enabled, ISSE_enabled, SSE2_enabled;
};

#if (!defined(VD_CPU_X86) && !defined(VD_CPU_AMD64)) || defined(__MINGW32__)
long CPUCheckForExtensions() {
	return 0;
}
#else

namespace {
#ifdef _M_IX86
	bool VDIsAVXSupportedByOS() {
		uint32 xfeature_enabled_mask;

		__asm {
			xor ecx, ecx
			__emit 0x0f		;xgetbv
			__emit 0x01
			__emit 0xd0
			mov dword ptr xfeature_enabled_mask, eax
		}

		return (xfeature_enabled_mask & 0x06) == 0x06;
	}
#else
	extern "C" bool VDIsAVXSupportedByOS();
#endif
}

// This code used to use IsProcessorFeaturePresent(), but this function is somewhat
// suboptimal in Win64 -- for one thing, it doesn't return true for MMX, at least
// on Vista 64.
long CPUCheckForExtensions() {
	// check for CPUID (x86 only)
#ifdef _M_IX86
	uint32 id;
	__asm {
		pushfd
		or		dword ptr [esp], 00200000h	;set the ID bit
		popfd
		pushfd					;flags -> EAX
		pop		dword ptr id
	}

	if (!(id & 0x00200000)) {
		// if we don't have CPUID, we probably won't want to try FPU optimizations
		// (80486).
		return 0;
	}
#endif

	// check for features register
	long flags = CPUF_SUPPORTS_FPU | CPUF_SUPPORTS_CPUID;

	int cpuInfo[4];
	__cpuid(cpuInfo, 0);
	if (cpuInfo[0] == 0)
		return flags;

	__cpuid(cpuInfo, 1);

	if (cpuInfo[3] & (1 << 23))
		flags |= CPUF_SUPPORTS_MMX;

	if (cpuInfo[3] & (1 << 25)) {
		// Check if SSE is actually supported.
		bool sseSupported = true;

#ifdef _M_IX86
		__try {
			__asm andps xmm0,xmm0
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
				sseSupported = false;
		}
#endif
		
		if (sseSupported) {
			flags |= CPUF_SUPPORTS_SSE | CPUF_SUPPORTS_INTEGER_SSE;

			if (cpuInfo[3] & (1 << 26))
				flags |= CPUF_SUPPORTS_SSE2;

			if (cpuInfo[2] & 0x00000001)
				flags |= CPUF_SUPPORTS_SSE3;

			if (cpuInfo[2] & 0x00000200)
				flags |= CPUF_SUPPORTS_SSSE3;

			if (cpuInfo[2] & 0x00080000)
				flags |= CPUF_SUPPORTS_SSE41;

			if (cpuInfo[2] & 0x00100000)
				flags |= CPUF_SUPPORTS_SSE42;

			// check OSXSAVE and AVX bits
			if ((cpuInfo[2] & ((1 << 27) | (1 << 28))) == ((1 << 27) | (1 << 28))) {
				if (VDIsAVXSupportedByOS())
					flags |= CPUF_SUPPORTS_AVX;
			}
		}
	}

	// check for 3DNow!, 3DNow! extensions
	__cpuid(cpuInfo, 0x80000000);
	if ((unsigned)cpuInfo[0] >= 0x80000001U) {
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
#if defined(VD_CPU_X86)
	if (ISSE_enabled)
		_mm_sfence();

	if (MMX_enabled)
		_mm_empty();
#elif defined(VD_CPU_AMD64)
	_mm_sfence();
#endif
}
