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
#include "uberblit_ycbcr_x86.h"

extern "C" void vdasm_pixblt_XRGB8888_to_YUV444Planar_scan_SSE2(void *dstY, void *dstCb, void *dstCr, const void *srcRGB, uint32 count, const void *coeffs);

void VDPixmapGenRGB32ToYCbCr601_SSE2::Compute(void *dst0, sint32 y) {
	uint8 *dstCb = (uint8 *)dst0;
	uint8 *dstY = dstCb + mWindowPitch;
	uint8 *dstCr = dstY + mWindowPitch;
	const uint8 *srcRGB = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	static const __declspec(align(16)) struct {
		sint16 rb_to_y[8];
		sint16 rb_to_cb[8];
		sint16 rb_to_cr[8];
		sint16 g_to_y[8];
		sint16 g_to_cb[8];
		sint16 g_to_cr[8];
		sint32 y_bias[4];
		sint32 c_bias[4];
	} kCoeffs={
	//	Cb = (28784*r - 24103*g -  4681*b + 8388608 + 32768) >> 16;
	//	Y  = (16829*r + 33039*g +  6416*b + 1048576 + 32768) >> 16;
	//	Cr = (-9714*r - 19071*g + 28784*b + 8388608 + 32768) >> 16;
		{   3208,  8414,   3208,  8414,   3208,  8414,   3208,  8414, },		// rb to y
		{  -2340, 14392,  -2340, 14392,  -2340, 14392,  -2340, 14392, },		// rb to cb
		{  16519,     0,  16519,     0,  16519,     0,  16519,     0, },		// g to y
		{ -12050,     0, -12050,     0, -12050,     0, -12050,     0, },		// g to cb
		{  14392, -4857,  14392, -4857,  14392, -4857,  14392, -4857, },		// rb to cr
		{  -9535,     0,  -9535,     0,  -9535,     0,  -9535,     0, },		// g to cr
		{ 0x084000, 0x084000, 0x084000, 0x084000, },	// y bias
		{ 0x404000, 0x404000, 0x404000, 0x404000, },	// c bias
	};

	vdasm_pixblt_XRGB8888_to_YUV444Planar_scan_SSE2(dstY, dstCb, dstCr, srcRGB, mWidth, &kCoeffs);
}
