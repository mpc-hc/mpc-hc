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
