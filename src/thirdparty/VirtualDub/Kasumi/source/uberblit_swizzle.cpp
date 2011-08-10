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
#include "uberblit_swizzle.h"

void VDPixmapGen_Swap8In16::Init(IVDPixmapGen *gen, int srcIndex, uint32 w, uint32 h, uint32 bpr) {
	InitSource(gen, srcIndex);
	mRowLength = bpr;
	SetOutputSize(w, h);
	gen->AddWindowRequest(0, 0);
}

void VDPixmapGen_Swap8In16::Start() {
	StartWindow(mRowLength);
}

uint32 VDPixmapGen_Swap8In16::GetType(uint32 index) const {
	return mpSrc->GetType(mSrcIndex);
}

void VDPixmapGen_Swap8In16::Compute(void *dst0, sint32 y) {
	const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
	uint8 *dst = (uint8 *)dst0;
	sint32 w = mRowLength;

	uint32 n4 = w >> 2;

	for(uint32 i=0; i<n4; ++i) {
		uint32 p = *(uint32 *)src;
		src += 4;

		uint32 r = ((p & 0xff00ff00) >> 8) + ((p & 0x00ff00ff) << 8);

		*(uint32 *)dst = r;
		dst += 4;
	}

	if (w & 2) {
		dst[0] = src[1];
		dst[1] = src[0];
		dst += 2;
		src += 2;
	}

	if (w & 1) {
		*dst = *src;
	}
}

/////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_B8x2_To_B8R8::Init(IVDPixmapGen *srcCb, uint32 srcindexCb, IVDPixmapGen *srcCr, uint32 srcindexCr) {
	mpSrcCb = srcCb;
	mSrcIndexCb = srcindexCb;
	mpSrcCr = srcCr;
	mSrcIndexCr = srcindexCr;
	mWidth = srcCb->GetWidth(srcindexCb);
	mHeight = srcCb->GetHeight(srcindexCb);

	srcCb->AddWindowRequest(0, 0);
	srcCr->AddWindowRequest(0, 0);
}

void VDPixmapGen_B8x2_To_B8R8::Start() {
	mpSrcCb->Start();
	mpSrcCr->Start();

	StartWindow(mWidth * 2);
}

uint32 VDPixmapGen_B8x2_To_B8R8::GetType(uint32 output) const {
	return (mpSrcCb->GetType(mSrcIndexCb) & ~kVDPixType_Mask) | kVDPixType_B8R8;
}

void VDPixmapGen_B8x2_To_B8R8::Compute(void *dst0, sint32 y) {
	uint8 *VDRESTRICT dst = (uint8 *)dst0;
	const uint8 *VDRESTRICT srcCb = (const uint8 *)mpSrcCb->GetRow(y, mSrcIndexCb);
	const uint8 *VDRESTRICT srcCr = (const uint8 *)mpSrcCr->GetRow(y, mSrcIndexCr);

	sint32 w = mWidth;
	for(sint32 x=0; x<w; ++x) {
		uint8 cb = srcCb[0];
		uint8 cr = srcCr[0];

		dst[0] = cb;
		dst[1] = cr;

		++srcCb;
		++srcCr;
		dst += 2;
	}
}
