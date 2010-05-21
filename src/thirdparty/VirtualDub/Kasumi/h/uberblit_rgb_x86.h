#ifndef f_VD2_KASUMI_UBERBLIT_RGB_X86_H
#define f_VD2_KASUMI_UBERBLIT_RGB_X86_H

#include <vd2/system/cpuaccel.h>
#include "uberblit_base.h"

extern "C" void vdasm_pixblt_XRGB1555_to_XRGB8888_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_RGB565_to_XRGB8888_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_RGB565_to_XRGB1555_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_XRGB1555_to_RGB565_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_XRGB8888_to_XRGB1555_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_XRGB8888_to_RGB565_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_XRGB8888_to_RGB888_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
extern "C" void vdasm_pixblt_RGB888_to_XRGB8888_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	16-bit crossconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X1R5G5B5_To_R5G6B5_MMX : public VDPixmapGen_X1R5G5B5_To_R5G6B5 {
protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_XRGB1555_to_RGB565_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

class VDPixmapGen_R5G6B5_To_X1R5G5B5_MMX : public VDPixmapGen_R5G6B5_To_X1R5G5B5 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_RGB565_to_XRGB1555_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32-bit upconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X1R5G5B5_To_X8R8G8B8_MMX : public VDPixmapGen_X1R5G5B5_To_X8R8G8B8 {
protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_XRGB1555_to_XRGB8888_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

class VDPixmapGen_R5G6B5_To_X8R8G8B8_MMX : public VDPixmapGen_R5G6B5_To_X8R8G8B8 {
protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_RGB565_to_XRGB8888_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

class VDPixmapGen_R8G8B8_To_X8R8G8B8_MMX : public VDPixmapGen_R8G8B8_To_A8R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_RGB888_to_XRGB8888_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32-bit downconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X8R8G8B8_To_X1R5G5B5_MMX : public VDPixmapGen_X8R8G8B8_To_X1R5G5B5 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_XRGB8888_to_XRGB1555_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

class VDPixmapGen_X8R8G8B8_To_R5G6B5_MMX : public VDPixmapGen_X8R8G8B8_To_R5G6B5 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_XRGB8888_to_RGB565_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

class VDPixmapGen_X8R8G8B8_To_R8G8B8_MMX : public VDPixmapGen_X8R8G8B8_To_R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

		vdasm_pixblt_XRGB8888_to_RGB888_MMX(dst, 0, src, 0, mWidth, 1);
	}
};

#endif
