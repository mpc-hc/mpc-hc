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
#include "uberblit_16f.h"

///////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_32F_To_16F::Start() {
	StartWindow(mWidth * sizeof(uint16));
}

uint32 VDPixmapGen_32F_To_16F::GetType(uint32 output) const {
	return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_16F_LE;
}

void VDPixmapGen_32F_To_16F::Compute(void *dst0, sint32 y) {
	uint16 *dst = (uint16 *)dst0;
	const float *src = (const float *)mpSrc->GetRow(y, mSrcIndex);
	uint32 w = mWidth;

	for(uint32 i=0; i<w; ++i)
		*dst++ = VDConvertFloatToHalf(src++);
}

///////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_16F_To_32F::Start() {
	StartWindow(mWidth * sizeof(float));
}

uint32 VDPixmapGen_16F_To_32F::GetType(uint32 output) const {
	return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_32F_LE;
}

void VDPixmapGen_16F_To_32F::Compute(void *dst0, sint32 y) {
	float *dst = (float *)dst0;
	const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);
	uint32 w = mWidth;

	for(uint32 i=0; i<w; ++i)
		VDConvertHalfToFloat(*src++, dst++);
}
