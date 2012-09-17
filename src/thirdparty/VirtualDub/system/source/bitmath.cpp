//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
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
#include <vd2/system/bitmath.h>

int VDCountBits(uint32 v) {
	v -= (v >> 1) & 0x55555555;
	v = ((v & 0xcccccccc) >> 2) + (v & 0x33333333);
	v = (v + (v >> 4)) & 0x0f0f0f0f;
	return (v * 0x01010101) >> 24;
}

#ifndef VD_COMPILER_MSVC_VC8

	int VDFindLowestSetBit(uint32 v) {
		for(int i=0; i<32; ++i) {
			if (v & 1)
				return i;
			v >>= 1;
		}

		return 32;
	}

	int VDFindHighestSetBit(uint32 v) {
		for(int i=31; i>=0; --i) {
			if ((sint32)v < 0)
				return i;
			v += v;
		}
		return -1;
	}

#endif

uint32 VDCeilToPow2(uint32 v) {
	v += v;
	--v;

	while(uint32 x = v & (v - 1))
		v = x;

	return v;
}
