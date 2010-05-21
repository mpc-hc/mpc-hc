#include "uberblit_resample_special_x86.h"
#include "blt_spanutils.h"
#include "blt_spanutils_x86.h"

void VDPixmapGenResampleRow_x2_p0_lin_u8_ISSE::Compute(void *dst0, sint32 y) {
	const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	nsVDPixmapSpanUtils::horiz_expand2x_coaligned_ISSE((uint8 *)dst0, src, mWidth);
}

void VDPixmapGenResampleRow_x4_p0_lin_u8_MMX::Compute(void *dst0, sint32 y) {
	const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	nsVDPixmapSpanUtils::horiz_expand4x_coaligned_MMX((uint8 *)dst0, src, mWidth);
}

void VDPixmapGenResampleCol_d2_pnqrtr_lin_u8_ISSE::Compute(void *dst0, sint32 y) {
	sint32 y2 = (y - 1) >> 1;
	const uint8 *src[2] = {
		(const uint8 *)mpSrc->GetRow(y2, mSrcIndex),
		(const uint8 *)mpSrc->GetRow(y2+1, mSrcIndex),
	};

	nsVDPixmapSpanUtils::vert_expand2x_centered_ISSE((uint8 *)dst0, src, mWidth, ~y << 7);
}

void VDPixmapGenResampleCol_d4_pn38_lin_u8_ISSE::Compute(void *dst0, sint32 y) {
	sint32 y2 = (y - 2) >> 2;
	const uint8 *src[2] = {
		(const uint8 *)mpSrc->GetRow(y2, mSrcIndex),
		(const uint8 *)mpSrc->GetRow(y2+1, mSrcIndex),
	};

	nsVDPixmapSpanUtils::vert_expand4x_centered_ISSE((uint8 *)dst0, src, mWidth, (y - 2) << 6);
}
