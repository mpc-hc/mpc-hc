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

#ifndef f_VD2_KASUMI_UBERBLIT_INTERLACE_H
#define f_VD2_KASUMI_UBERBLIT_INTERLACE_H

#include "uberblit_base.h"

class VDPixmapGen_SplitFields : public IVDPixmapGen {
public:
	void AddWindowRequest(int minDY, int maxDY) {
		mpSrc->AddWindowRequest(minDY*2, maxDY*2+1);
	}

	void Start() {
		mpSrc->Start();
	}

	sint32 GetWidth(int) const { return mWidth; }
	sint32 GetHeight(int idx) const { return mHeight[idx]; }

	bool IsStateful() const {
		return false;
	}

	uint32 GetType(uint32 output) const {
		return mpSrc->GetType(mSrcIndex);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return mpSrc->GetRow(y+y+index, mSrcIndex);
	}

	void ProcessRow(void *dst, sint32 y) {
		memcpy(dst, GetRow(y, 0), mBpr);
	}

	void Init(IVDPixmapGen *src, uint32 srcindex, uint32 bpr) {
		mpSrc = src;
		mSrcIndex = srcindex;
		mBpr = bpr;
		mWidth = src->GetWidth(srcindex);

		uint32 h = src->GetHeight(srcindex);
		mHeight[0] = (h + 1) >> 1;
		mHeight[1] = h >> 1;
	}

protected:
	IVDPixmapGen *mpSrc;
	uint32 mSrcIndex;
	sint32 mWidth;
	sint32 mHeight[2];
	uint32 mBpr;
};

class VDPixmapGen_MergeFields : public IVDPixmapGen {
public:
	void AddWindowRequest(int minDY, int maxDY) {
		mpSrc[0]->AddWindowRequest(minDY >> 1, maxDY >> 1);
		mpSrc[1]->AddWindowRequest(minDY >> 1, maxDY >> 1);
	}

	void Start() {
		mpSrc[0]->Start();
		mpSrc[1]->Start();
	}

	sint32 GetWidth(int) const { return mWidth; }
	sint32 GetHeight(int) const { return mHeight; }

	bool IsStateful() const {
		return false;
	}

	uint32 GetType(uint32 output) const {
		return mpSrc[0]->GetType(mSrcIndex[0]);
	}

	const void *GetRow(sint32 y, uint32 index) {
		int srcIndex = y & 1;
		return mpSrc[srcIndex]->GetRow(y >> 1, mSrcIndex[srcIndex]);
	}

	void ProcessRow(void *dst, sint32 y) {
		memcpy(dst, GetRow(y, 0), mBpr);
	}

	void Init(IVDPixmapGen *src1, uint32 srcindex1, IVDPixmapGen *src2, uint32 srcindex2, uint32 w, uint32 h, uint32 bpr) {
		mpSrc[0] = src1;
		mpSrc[1] = src2;
		mSrcIndex[0] = srcindex1;
		mSrcIndex[1] = srcindex2;

		mWidth = w;
		mHeight = h;
		mBpr = bpr;
	}

protected:
	IVDPixmapGen *mpSrc[2];
	uint32 mSrcIndex[2];
	sint32 mWidth;
	sint32 mHeight;
	uint32 mBpr;
};

#endif
