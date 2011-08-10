//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include <vd2/system/halffloat.h>
#include <vd2/system/math.h>
#include "uberblit_v210.h"

///////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_32F_To_V210::Compute(void *dst0, sint32 y) {
	uint32 *dst = (uint32 *)dst0;
	const float *srcR = (const float *)mpSrcR->GetRow(y, mSrcIndexR);
	const float *srcG = (const float *)mpSrcG->GetRow(y, mSrcIndexG);
	const float *srcB = (const float *)mpSrcB->GetRow(y, mSrcIndexB);

	VDCPUCleanupExtensions();

	int w6 = mWidth / 6;
	for(sint32 i=0; i<w6; ++i) {
		float r0 = srcR[0];
		float r1 = srcR[1];
		float r2 = srcR[2];
		srcR += 3;

		float b0 = srcB[0];
		float b1 = srcB[1];
		float b2 = srcB[2];
		srcB += 3;

		float g0 = srcG[0];
		float g1 = srcG[1];
		float g2 = srcG[2];
		float g3 = srcG[3];
		float g4 = srcG[4];
		float g5 = srcG[5];
		srcG += 6;

		if (r0 < 0.0f) r0 = 0.0f; else if (r0 > 1.0f) r0 = 1.0f;
		if (r1 < 0.0f) r1 = 0.0f; else if (r1 > 1.0f) r1 = 1.0f;
		if (r2 < 0.0f) r2 = 0.0f; else if (r2 > 1.0f) r2 = 1.0f;
		if (g0 < 0.0f) g0 = 0.0f; else if (g0 > 1.0f) g0 = 1.0f;
		if (g1 < 0.0f) g1 = 0.0f; else if (g1 > 1.0f) g1 = 1.0f;
		if (g2 < 0.0f) g2 = 0.0f; else if (g2 > 1.0f) g2 = 1.0f;
		if (g3 < 0.0f) g3 = 0.0f; else if (g3 > 1.0f) g3 = 1.0f;
		if (g4 < 0.0f) g4 = 0.0f; else if (g4 > 1.0f) g4 = 1.0f;
		if (g5 < 0.0f) g5 = 0.0f; else if (g5 > 1.0f) g5 = 1.0f;
		if (b0 < 0.0f) b0 = 0.0f; else if (b0 > 1.0f) b0 = 1.0f;
		if (b1 < 0.0f) b1 = 0.0f; else if (b1 > 1.0f) b1 = 1.0f;
		if (b2 < 0.0f) b2 = 0.0f; else if (b2 > 1.0f) b2 = 1.0f;

		uint32 ir0 = (uint32)VDRoundToIntFast(r0 * 1024.0f);
		uint32 ir1 = (uint32)VDRoundToIntFast(r1 * 1024.0f);
		uint32 ir2 = (uint32)VDRoundToIntFast(r2 * 1024.0f);
		uint32 ib0 = (uint32)VDRoundToIntFast(b0 * 1024.0f);
		uint32 ib1 = (uint32)VDRoundToIntFast(b1 * 1024.0f);
		uint32 ib2 = (uint32)VDRoundToIntFast(b2 * 1024.0f);
		uint32 ig0 = (uint32)VDRoundToIntFast(g0 * 1024.0f);
		uint32 ig1 = (uint32)VDRoundToIntFast(g1 * 1024.0f);
		uint32 ig2 = (uint32)VDRoundToIntFast(g2 * 1024.0f);
		uint32 ig3 = (uint32)VDRoundToIntFast(g3 * 1024.0f);
		uint32 ig4 = (uint32)VDRoundToIntFast(g4 * 1024.0f);
		uint32 ig5 = (uint32)VDRoundToIntFast(g5 * 1024.0f);

		// dword 0: XX Cr0 Y0 Cb0
		// dword 1: XX Y2 Cb1 Y1
		// dword 2: XX Cb2 Y3 Cr1
		// dword 3: XX Y5 Cr2 Y4
		dst[0] = (ir0 << 20) + (ig0 << 10) + ib0;
		dst[1] = (ig2 << 20) + (ib1 << 10) + ig1;
		dst[2] = (ib2 << 20) + (ig3 << 10) + ir1;
		dst[3] = (ig5 << 20) + (ir2 << 10) + ig4;

		dst += 4;
	}

	int leftovers = mWidth - w6*6;
	if (leftovers) {
		float g0 = 0;
		float g1 = 0;
		float g2 = 0;
		float g3 = 0;
		float g4 = 0;
		float r0 = 0;
		float r1 = 0;
		float r2 = 0;
		float b0 = 0;
		float b1 = 0;
		float b2 = 0;

		switch(leftovers) {
			case 5:	r2 = srcR[2];
					b2 = srcB[2];
					g4 = srcG[4];
			case 4:	g3 = srcG[3];
			case 3:	r1 = srcR[1];
					b1 = srcB[1];
					g2 = srcG[2];
			case 2:	g1 = srcG[1];
			case 1:	r0 = srcR[0];
					b0 = srcB[0];
					g0 = srcG[0];
		}

		if (r0 < 0.0f) r0 = 0.0f; else if (r0 > 1.0f) r0 = 1.0f;
		if (r1 < 0.0f) r1 = 0.0f; else if (r1 > 1.0f) r1 = 1.0f;
		if (r2 < 0.0f) r2 = 0.0f; else if (r2 > 1.0f) r2 = 1.0f;
		if (g0 < 0.0f) g0 = 0.0f; else if (g0 > 1.0f) g0 = 1.0f;
		if (g1 < 0.0f) g1 = 0.0f; else if (g1 > 1.0f) g1 = 1.0f;
		if (g2 < 0.0f) g2 = 0.0f; else if (g2 > 1.0f) g2 = 1.0f;
		if (g3 < 0.0f) g3 = 0.0f; else if (g3 > 1.0f) g3 = 1.0f;
		if (g4 < 0.0f) g4 = 0.0f; else if (g4 > 1.0f) g4 = 1.0f;
		if (b0 < 0.0f) b0 = 0.0f; else if (b0 > 1.0f) b0 = 1.0f;
		if (b1 < 0.0f) b1 = 0.0f; else if (b1 > 1.0f) b1 = 1.0f;
		if (b2 < 0.0f) b2 = 0.0f; else if (b2 > 1.0f) b2 = 1.0f;

		uint32 ir0 = (uint32)VDRoundToIntFast(r0 * 1024.0f);
		uint32 ir1 = (uint32)VDRoundToIntFast(r1 * 1024.0f);
		uint32 ir2 = (uint32)VDRoundToIntFast(r2 * 1024.0f);
		uint32 ib0 = (uint32)VDRoundToIntFast(b0 * 1024.0f);
		uint32 ib1 = (uint32)VDRoundToIntFast(b1 * 1024.0f);
		uint32 ib2 = (uint32)VDRoundToIntFast(b2 * 1024.0f);
		uint32 ig0 = (uint32)VDRoundToIntFast(g0 * 1024.0f);
		uint32 ig1 = (uint32)VDRoundToIntFast(g1 * 1024.0f);
		uint32 ig2 = (uint32)VDRoundToIntFast(g2 * 1024.0f);
		uint32 ig3 = (uint32)VDRoundToIntFast(g3 * 1024.0f);
		uint32 ig4 = (uint32)VDRoundToIntFast(g4 * 1024.0f);

		// dword 0: XX Cr0 Y0 Cb0
		// dword 1: XX Y2 Cb1 Y1
		// dword 2: XX Cb2 Y3 Cr1
		// dword 3: XX Y5 Cr2 Y4
		dst[0] = (ir0 << 20) + (ig0 << 10) + ib0;
		dst[1] = (ig2 << 20) + (ib1 << 10) + ig1;
		dst[2] = (ib2 << 20) + (ig3 << 10) + ir1;
		dst[3] =               (ir2 << 10) + ig4;
		dst += 4;
	}

	// QuickTime defines the v210 format and requires zero padding in all unused samples.
	int w48up = (mWidth + 23) / 24;
	int w6up = (mWidth + 5) / 6;
	int zeropad = w48up * 16 - w6up * 4;
	memset(dst, 0, zeropad * 4);
}

///////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_V210_To_32F::Start() {
	StartWindow(((mWidth + 5) / 6) * 6 * sizeof(float), 3);
}

const void *VDPixmapGen_V210_To_32F::GetRow(sint32 y, uint32 index) {
	return (const uint8 *)VDPixmapGenWindowBasedOneSource::GetRow(y, index) + mWindowPitch * index;
}

sint32 VDPixmapGen_V210_To_32F::GetWidth(int index) const {
	return index == 1 ? mWidth : (mWidth + 1) >> 1;
}

uint32 VDPixmapGen_V210_To_32F::GetType(uint32 output) const {
	return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_32F_LE;
}

void VDPixmapGen_V210_To_32F::Compute(void *dst0, sint32 y) {
	float *dstR = (float *)dst0;
	float *dstG = (float *)((char *)dstR + mWindowPitch);
	float *dstB = (float *)((char *)dstG + mWindowPitch);
	const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
	uint32 w = (mWidth + 5) / 6;

	VDCPUCleanupExtensions();

	// dword 0: XX Cr0 Y0 Cb0
	// dword 1: XX Y2 Cb1 Y1
	// dword 2: XX Cb2 Y3 Cr1
	// dword 3: XX Y5 Cr2 Y4

	for(uint32 i=0; i<w; ++i) {
		const uint32 w0 = src[0];
		const uint32 w1 = src[1];
		const uint32 w2 = src[2];
		const uint32 w3 = src[3];
		src += 4;

		dstB[0] = (float)( w0        & 0x3ff) / 1023.0f;
		dstG[0] = (float)((w0 >> 10) & 0x3ff) / 1023.0f;
		dstR[0] = (float)((w0 >> 20) & 0x3ff) / 1023.0f;
		dstG[1] = (float)( w1        & 0x3ff) / 1023.0f;
		dstB[1] = (float)((w1 >> 10) & 0x3ff) / 1023.0f;
		dstG[2] = (float)((w1 >> 20) & 0x3ff) / 1023.0f;
		dstR[1] = (float)( w2        & 0x3ff) / 1023.0f;
		dstG[3] = (float)((w2 >> 10) & 0x3ff) / 1023.0f;
		dstB[2] = (float)((w2 >> 20) & 0x3ff) / 1023.0f;
		dstG[4] = (float)( w3        & 0x3ff) / 1023.0f;
		dstR[2] = (float)((w3 >> 10) & 0x3ff) / 1023.0f;
		dstG[5] = (float)((w3 >> 20) & 0x3ff) / 1023.0f;

		dstR += 3;
		dstG += 6;
		dstB += 3;
	}
}
