#ifndef f_VD2_KASUMI_UBERBLIT_V210_H
#define f_VD2_KASUMI_UBERBLIT_V210_H

#include <vd2/system/cpuaccel.h>
#include "uberblit_base.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32F -> V210
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_32F_To_V210 : public VDPixmapGenWindowBased {
public:
	void Init(IVDPixmapGen *srcR, uint32 srcindexR, IVDPixmapGen *srcG, uint32 srcindexG, IVDPixmapGen *srcB, uint32 srcindexB) {
		mpSrcR = srcR;
		mSrcIndexR = srcindexR;
		mpSrcG = srcG;
		mSrcIndexG = srcindexG;
		mpSrcB = srcB;
		mSrcIndexB = srcindexB;
		mWidth = srcG->GetWidth(srcindexG);
		mHeight = srcG->GetHeight(srcindexG);

		srcR->AddWindowRequest(0, 0);
		srcG->AddWindowRequest(0, 0);
		srcB->AddWindowRequest(0, 0);
	}

	void Start() {
		mpSrcR->Start();
		mpSrcG->Start();
		mpSrcB->Start();

		int qw = (mWidth + 47) / 48;
		StartWindow(qw * 128);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcG->GetType(mSrcIndexG) & ~kVDPixType_Mask) | kVDPixType_V210;
	}

protected:
	void Compute(void *dst0, sint32 y);

	IVDPixmapGen *mpSrcR;
	uint32 mSrcIndexR;
	IVDPixmapGen *mpSrcG;
	uint32 mSrcIndexG;
	IVDPixmapGen *mpSrcB;
	uint32 mSrcIndexB;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	V210 -> 32F
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_V210_To_32F : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start();
	const void *GetRow(sint32 y, uint32 index);

	sint32 GetWidth(int index) const;
	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 y);
};

#endif	// f_VD2_KASUMI_UBERBLIT_V210_H
