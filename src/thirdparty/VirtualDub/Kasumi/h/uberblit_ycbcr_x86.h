#ifndef f_VD2_KASUMI_UBERBLIT_YCBCR_X86_H
#define f_VD2_KASUMI_UBERBLIT_YCBCR_X86_H

#include <vd2/system/cpuaccel.h>
#include "uberblit.h"
#include "uberblit_ycbcr.h"

extern "C" void __cdecl vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX(void *dst, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 count);

class VDPixmapGenYCbCr601ToRGB32_MMX : public VDPixmapGenYCbCr601ToRGB32 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *srcY = (const uint8 *)mpSrcY->GetRow(y, mSrcIndexY);
		const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(y, mSrcIndexCb);
		const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(y, mSrcIndexCr);

		vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX(dst, srcY, srcCb, srcCr, mWidth);
	}
};

class VDPixmapGenRGB32ToYCbCr601_SSE2 : public VDPixmapGenRGB32ToYCbCr601 {
protected:
	void Compute(void *dst0, sint32 y);
};

#endif
