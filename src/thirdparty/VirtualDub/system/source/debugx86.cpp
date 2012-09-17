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
#include <vd2/system/seh.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/debugx86.h>

bool VDIsValidCallX86(const char *buf, int len) {
	// Permissible CALL sequences that we care about:
	//
	//	E8 xx xx xx xx			CALL near relative
	//	FF (group 2)			CALL near absolute indirect
	//
	// Minimum sequence is 2 bytes (call eax).
	// Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).

	if (len >= 5 && buf[-5] == (char)0xE8)
		return true;

	// FF 14 xx					CALL [reg32+reg32*scale]

	if (len >= 3 && buf[-3] == (char)0xFF && buf[-2]==0x14)
		return true;

	// FF 15 xx xx xx xx		CALL disp32

	if (len >= 6 && buf[-6] == (char)0xFF && buf[-5]==0x15)
		return true;

	// FF 00-3F(!14/15)			CALL [reg32]

	if (len >= 2 && buf[-2] == (char)0xFF && (unsigned char)buf[-1] < 0x40)
		return true;

	// FF D0-D7					CALL reg32

	if (len >= 2 && buf[-2] == (char)0xFF && (buf[-1]&0xF8) == 0xD0)
		return true;

	// FF 50-57 xx				CALL [reg32+reg32*scale+disp8]

	if (len >= 3 && buf[-3] == (char)0xFF && (buf[-2]&0xF8) == 0x50)
		return true;

	// FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]

	if (len >= 7 && buf[-7] == (char)0xFF && (buf[-6]&0xF8) == 0x90)
		return true;

	return false;
}

VDInstructionTypeX86 VDGetInstructionTypeX86(const void *p) {
	struct local {
		static bool RangeHitTest(const uint8 *range, uint8 c) {
			while(*range) {
				if (c>=range[0] && c<=range[1])
					return true;
				range += 2;
			}

			return false;
		}
	};

	VDInstructionTypeX86 type = kX86InstUnknown;

	vd_seh_guard_try {
		unsigned char buf[8];

		memcpy(buf, p, 8);

		if (buf[0] == 0x0f && buf[1] == 0x0f)
			type = kX86Inst3DNow;			// Conveniently, all 3DNow! instructions begin 0F 0F
		else if ((buf[0] == 0xdb || buf[0] == 0xdf) && (buf[1]>=0xe8 && buf[1]<=0xf7))
			type = kX86InstP6;				// DB/DF E8-F7: FCOMI/FCOMIP/FUCOMI/FUCOMIP (P6)
		else if ((buf[0]&0xfe)==0xda && (buf[1]&0xe0)==0xc0)
			type = kX86InstP6;				// DA/DB C0-DF: FCMOVcc (P6)
		else if (buf[0] == 0x0f && (buf[1]&0xf0)==0x40)
			type = kX86InstP6;				// 0F 40-4F: CMOVcc (P6)
		else {
			const unsigned char *s = buf;
			bool bWide = false;
			bool bRepF2 = false;
			bool bRepF3 = false;

			// At this point we're down to MMX, SSE, SSE2 -- which makes things simpler
			// as we must see F2 0F, F3 0F, or 0F next.  MMX ops use 0F exclusively,
			// some SSE ops use F2, and a few SSE2 ones use F3.  If we see 66 on an
			// MMX or SSE op it's automatically SSE2 as it's either a 128-bit MMX op
			// or a double-precision version of an SSE one.

			if (*s == 0x66) {		// 66h override used by SSE2 and is supposed to be ahead of F2/F3 in encodings
				++s;
				bWide = true;
			}

			if (*s == 0xf2) {
				++s;
				bRepF2 = true;
			}

			if (*s == 0xf3) {
				++s;
				bRepF3 = true;
			}

			if (*s++ == 0x0f) {
				// SSE - 1x, 28-2F, 5x, C2, AE
				// MMX2 - 70, C4-C6, D7, DA, DE, E0, E3, E4, E7, EA, EE, F6, F7
				// MMX - 6x, 7x, Dx, Ex, and Fx except for MMX2
				// SSE2 - C3, SSE ops with 66 or F2, MMX/MMX2 ops with 66/F2/F3

				static const uint8 sse_ranges[]={0x10,0x1f,0x28,0x2f,0x50,0x5f,0xc2,0xc2,0xae,0xae,0};
				static const uint8 sse2_ranges[]={0xc3,0xc3,0};
				static const uint8 mmx2_ranges[]={0x70,0x70,0xc4,0xc6,0xd7,0xd7,0xda,0xda,0xde,0xde,0xe0,0xe0,0xe3,0xe4,0xe7,0xe7,0xea,0xea,0xee,0xee,0xf6,0xf7,0};
				static const uint8 mmx_ranges[]={0x60,0x7f,0xd0,0xff,0};

				if (local::RangeHitTest(sse_ranges, *s))
					type = (bWide||bRepF2) ? kX86InstSSE2 : kX86InstSSE;
				else if (local::RangeHitTest(sse2_ranges, *s))
					type = kX86InstSSE2;
				else if (local::RangeHitTest(mmx2_ranges, *s))
					type = (bWide||bRepF2||bRepF3) ? kX86InstSSE2 : kX86InstMMX2;
				else if (local::RangeHitTest(mmx_ranges, *s))
					type = (bWide||bRepF2||bRepF3) ? kX86InstSSE2 : kX86InstMMX;
			}
		}
	} vd_seh_guard_except {
	}

	return type;
}
