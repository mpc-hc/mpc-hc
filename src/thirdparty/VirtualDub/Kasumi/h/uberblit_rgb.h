#ifndef f_VD2_KASUMI_UBERBLIT_RGB_H
#define f_VD2_KASUMI_UBERBLIT_RGB_H

#include <vd2/system/cpuaccel.h>
#include "uberblit_base.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	16-bit crossconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X1R5G5B5_To_R5G6B5 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_565_LE;
	}

protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];

			px += (px & 0x7fe0);
			px += (px & 0x400) >> 5;

			dst[i] = (uint16)px;
		}
	}
};

class VDPixmapGen_R5G6B5_To_X1R5G5B5 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];

			px &= 0xffdf;
			px -= (px & 0xffc0) >> 1;

			dst[i] = (uint16)px;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32-bit upconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X1R5G5B5_To_X8R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];
			uint32 px5 = ((px & 0x7c00) << 9) + ((px & 0x03e0) << 6) + ((px & 0x001f) << 3);

			dst[i] = px5 + ((px5 >> 5) & 0x070707);
		}
	}
};

class VDPixmapGen_R5G6B5_To_X8R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint16 *src = (const uint16 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];
			uint32 px_rb5 = ((px & 0xf800) << 8) + ((px & 0x001f) << 3);
			uint32 px_g6 = ((px & 0x07e0) << 5);

			dst[i] = px_rb5 + px_g6 + (((px_rb5 >> 5) + (px_g6 >> 6)) & 0x070307);
		}
	}
};

class VDPixmapGen_R8G8B8_To_A8R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = 255;
			dst += 4;
			src += 3;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32-bit downconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X8R8G8B8_To_X1R5G5B5 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_1555_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];

			dst[i] = ((px >> 9) & 0x7c00) + ((px >> 6) & 0x03e0) + ((px >> 3) & 0x001f);
		}
	}
};

class VDPixmapGen_X8R8G8B8_To_R5G6B5 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_565_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];

			dst[i] = ((px >> 8) & 0xf800) + ((px >> 5) & 0x07e0) + ((px >> 3) & 0x001f);
		}
	}
};

class VDPixmapGen_X8R8G8B8_To_R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 3);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];

			dst += 3;
			src += 4;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32-bit downconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_X8R8G8B8_To_X1R5G5B5_Dithered : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_1555_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		static const uint32 kDitherMatrix[4][4][2]={
			{ 0x00000000, 0x00000000, 0x04000400, 0x00040000, 0x01000100, 0x00010000, 0x05000500, 0x00050000 },
			{ 0x06000600, 0x00060000, 0x02000200, 0x00020000, 0x07000700, 0x00070000, 0x03000300, 0x00030000 },
			{ 0x01800180, 0x00018000, 0x05800580, 0x00058000, 0x00800080, 0x00008000, 0x04800480, 0x00048000 },
			{ 0x07800780, 0x00078000, 0x03800380, 0x00038000, 0x06800680, 0x00068000, 0x02800280, 0x00028000 },
		};

		const uint32 (*drow)[2] = kDitherMatrix[y & 3];

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];
			uint32 drg = drow[i & 3][0];
			uint32 db = drow[i & 3][1];
			uint32 rb = (px & 0xff00ff) * 249 + drg;
			uint32 g = (px & 0xff00) * 249 + db;

			dst[i] = ((rb >> 17) & 0x7c00) + ((g >> 14) & 0x03e0) + ((rb >> 11) & 0x001f);
		}
	}
};

class VDPixmapGen_X8R8G8B8_To_R5G6B5_Dithered : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 2);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_565_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint16 *dst = (uint16 *)dst0;
		const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		static const uint32 kDitherMatrix[4][4][2]={
			{ 0x00000000, 0x00000000, 0x04000400, 0x00020000, 0x01000100, 0x00008000, 0x05000500, 0x00028000 },
			{ 0x06000600, 0x00030000, 0x02000200, 0x00010000, 0x07000700, 0x00038000, 0x03000300, 0x00018000 },
			{ 0x01800180, 0x0000c000, 0x05800580, 0x0002c000, 0x00800080, 0x00004000, 0x04800480, 0x00024000 },
			{ 0x07800780, 0x0003c000, 0x03800380, 0x0001c000, 0x06800680, 0x00034000, 0x02800280, 0x00014000 },
		};

		const uint32 (*drow)[2] = kDitherMatrix[y & 3];

		for(sint32 i=0; i<w; ++i) {
			uint32 px = src[i];
			uint32 drg = drow[i & 3][0];
			uint32 db = drow[i & 3][1];
			uint32 rb = (px & 0xff00ff) * 249 + drg;
			uint32 g = (px & 0xff00) * 253 + db;

			dst[i] = ((rb >> 16) & 0xf800) + ((g >> 13) & 0x07e0) + ((rb >> 11) & 0x001f);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32F upconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_8_To_32F : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_32F_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dst = (float *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<w; ++i)
			*dst++ = (float)*src++ * (1.0f / 255.0f);
	}
};

class VDPixmapGen_X8R8G8B8_To_X32B32G32R32F : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 16);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_32Fx4_LE;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dst = (float *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<w; ++i) {
			dst[0] = (float)src[2] * (1.0f / 255.0f);
			dst[1] = (float)src[1] * (1.0f / 255.0f);
			dst[2] = (float)src[0] * (1.0f / 255.0f);
			dst[3] = 1.0f;
			dst += 4;
			src += 4;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	32F downconverters
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGen_32F_To_8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const float *src = (const float *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<w; ++i) {
			float b = *src++;

			uint32 ib = VDClampedRoundFixedToUint8Fast(b);

			dst[i] = (uint8)ib;
		}
	}
};

class VDPixmapGen_32F_To_8_Dithered : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const float *src = (const float *)mpSrc->GetRow(y, mSrcIndex);
		VDCPUCleanupExtensions();

		sint32 w = mWidth;

#define X(v) ((v) - 0x49400000)

		static const sint32 kDitherMatrix[4][4]={
			{ X( 0), X( 8), X( 2), X(10), },
			{ X(12), X( 4), X(14), X( 6), },
			{ X( 3), X(11), X( 1), X( 9), },
			{ X(15), X( 7), X(13), X( 5), },
		};

#undef X

		const sint32 *pDitherRow = kDitherMatrix[y & 3];

		for(sint32 i=0; i<w; ++i) {
			float b = *src++;

			sint32 addend = pDitherRow[i & 3];
			union {
				float f;
				sint32 i;
			}	cb = {b * 255.0f + 786432.0f};

			sint32 vb = ((sint32)cb.i + addend) >> 4;

			if ((uint32)vb >= 0x100)
				vb = (uint8)(~vb >> 31);

			dst[i] = (uint8)vb;
		}
	}
};

class VDPixmapGen_X32B32G32R32F_To_X8R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const float *src = (const float *)mpSrc->GetRow(y, mSrcIndex);

		VDCPUCleanupExtensions();

		sint32 w = mWidth;

		for(sint32 i=0; i<w; ++i) {
			float r = src[0];
			float g = src[1];
			float b = src[2];
			src += 4;

			uint32 ir = VDClampedRoundFixedToUint8Fast(r) << 16;
			uint32 ig = VDClampedRoundFixedToUint8Fast(g) << 8;
			uint32 ib = VDClampedRoundFixedToUint8Fast(b);

			dst[i] = ir + ig + ib;
		}
	}
};

class VDPixmapGen_X32B32G32R32F_To_X8R8G8B8_Dithered : public VDPixmapGenWindowBasedOneSourceSimple {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const float *src = (const float *)mpSrc->GetRow(y, mSrcIndex);

		VDCPUCleanupExtensions();

		sint32 w = mWidth;

#define X(v) ((v) - 0x49400000)

		static const sint32 kDitherMatrix[4][4]={
			{ X( 0), X( 8), X( 2), X(10), },
			{ X(12), X( 4), X(14), X( 6), },
			{ X( 3), X(11), X( 1), X( 9), },
			{ X(15), X( 7), X(13), X( 5), },
		};

#undef X

		const sint32 *pDitherRow = kDitherMatrix[y & 3];

		for(sint32 i=0; i<w; ++i) {
			float r = src[0];
			float g = src[1];
			float b = src[2];
			src += 4;

			sint32 addend = pDitherRow[i & 3];
			union {
				float f;
				sint32 i;
			}	cr = {r * 255.0f + 786432.0f},
				cg = {g * 255.0f + 786432.0f},
				cb = {b * 255.0f + 786432.0f};

			sint32 vr = ((sint32)cr.i + addend) >> 4;
			sint32 vg = ((sint32)cg.i + addend) >> 4;
			sint32 vb = ((sint32)cb.i + addend) >> 4;

			if ((uint32)vr >= 0x100)
				vr = (uint8)(~vr >> 31);

			if ((uint32)vg >= 0x100)
				vg = (uint8)(~vg >> 31);

			if ((uint32)vb >= 0x100)
				vb = (uint8)(~vb >> 31);

			dst[i] = (vr << 16) + (vg << 8) + vb;
		}
	}
};

#endif
