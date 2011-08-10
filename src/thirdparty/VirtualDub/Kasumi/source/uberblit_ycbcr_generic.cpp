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
#include <vd2/system/vectors.h>
#include "uberblit_ycbcr_generic.h"

extern const VDPixmapGenYCbCrBasis g_VDPixmapGenYCbCrBasis_601 = {
	0.299f,
	0.114f,
	{
		0.0f,   -0.3441363f,   1.772f,
		1.402f, -0.7141363f,   0.0f,
	}
};

extern const VDPixmapGenYCbCrBasis g_VDPixmapGenYCbCrBasis_709 = {
	0.2126f,
	0.0722f,
	{
		0.0f,     -0.1873243f,    1.8556f,
		1.5748f,  -0.4681243f,    0.0f,
	}
};

////////////////////////////////////////////////////////////////////////////

VDPixmapGenYCbCrToRGB32Generic::VDPixmapGenYCbCrToRGB32Generic(const VDPixmapGenYCbCrBasis& basis, bool studioRGB) {
	float scale;
	float bias;

	if (studioRGB) {
		scale = 65536.0f * (219.0f / 255.0f);
		bias = 65536.0f * (16.0f / 255.0f + 0.5f);
	} else {
		scale = 65536.0f;
		bias = 32768.0f;
	}

	float scale255 = scale * 255.0f;

	mCoY = VDRoundToInt32(scale);
	mCoRCr = VDRoundToInt32(basis.mToRGB[1][0] * scale);
	mCoGCr = VDRoundToInt32(basis.mToRGB[1][1] * scale);
	mCoGCb = VDRoundToInt32(basis.mToRGB[0][1] * scale);
	mCoBCb = VDRoundToInt32(basis.mToRGB[0][2] * scale);
	mBiasR = VDRoundToInt32(bias) - 128*mCoRCr;
	mBiasG = VDRoundToInt32(bias) - 128*(mCoGCr + mCoGCb);
	mBiasB = VDRoundToInt32(bias) - 128*mCoBCb;
}

uint32 VDPixmapGenYCbCrToRGB32Generic::GetType(uint32 output) const {
	return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8888 | kVDPixSpace_BGR;
}

void VDPixmapGenYCbCrToRGB32Generic::Compute(void *dst0, sint32 y) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *srcY = (const uint8 *)mpSrcY->GetRow(y, mSrcIndexY);
	const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(y, mSrcIndexCb);
	const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(y, mSrcIndexCr);

	const sint32 coY = mCoY;
	const sint32 coRCr = mCoRCr;
	const sint32 coGCr = mCoGCr;
	const sint32 coGCb = mCoGCb;
	const sint32 coBCb = mCoBCb;
	const sint32 biasR = mBiasR;
	const sint32 biasG = mBiasG;
	const sint32 biasB = mBiasB;

	const sint32 w = mWidth;
	for(sint32 i=0; i<w; ++i) {
		sint32 y = srcY[i];
		sint32 cb = srcCb[i];
		sint32 cr = srcCr[i];

		y *= coY;

		sint32 r = biasR + y + coRCr * cr;
		sint32 g = biasG + y + coGCr * cr + coGCb * cb;
		sint32 b = biasB + y + coBCb * cb;

		// clip low
		r &= ~r >> 31;
		g &= ~g >> 31;
		b &= ~b >> 31;

		// clip high
		sint32 clipR = 0xffffff - r;
		sint32 clipG = 0xffffff - g;
		sint32 clipB = 0xffffff - b;
		r |= clipR >> 31;
		g |= clipG >> 31;
		b |= clipB >> 31;

		dst[0] = (uint8)(b >> 16);
		dst[1] = (uint8)(g >> 16);
		dst[2] = (uint8)(r >> 16);
		dst[3] = 0xff;

		dst += 4;
	}
}

////////////////////////////////////////////////////////////////////////////

VDPixmapGenYCbCrToRGB32FGeneric::VDPixmapGenYCbCrToRGB32FGeneric(const VDPixmapGenYCbCrBasis& basis) {
	mCoRCr = basis.mToRGB[1][0];
	mCoGCr = basis.mToRGB[1][1];
	mCoGCb = basis.mToRGB[0][1];
	mCoBCb = basis.mToRGB[0][2];
}

uint32 VDPixmapGenYCbCrToRGB32FGeneric::GetType(uint32 output) const {
	return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32Fx4_LE | kVDPixSpace_BGR;
}

void VDPixmapGenYCbCrToRGB32FGeneric::Compute(void *dst0, sint32 y) {
	float *dst = (float *)dst0;
	const float *srcY = (const float *)mpSrcY->GetRow(y, mSrcIndexY);
	const float *srcCb = (const float *)mpSrcCb->GetRow(y, mSrcIndexCb);
	const float *srcCr = (const float *)mpSrcCr->GetRow(y, mSrcIndexCr);

	VDCPUCleanupExtensions();

	const float coRCr = mCoRCr;
	const float coGCr = mCoGCr;
	const float coGCb = mCoGCb;
	const float coBCb = mCoBCb;

	for(sint32 i=0; i<mWidth; ++i) {
		float y = srcY[i];
		float cb = srcCb[i] - (128.0f / 255.0f);
		float cr = srcCr[i] - (128.0f / 255.0f);

		dst[0] = y + coRCr * cr;
		dst[1] = y + coGCr * cr + coGCb * cb;
		dst[2] = y + coBCb * cb;
		dst[3] = 1.0f;
		dst += 4;
	}
}

////////////////////////////////////////////////////////////////////////////

VDPixmapGenRGB32ToYCbCrGeneric::VDPixmapGenRGB32ToYCbCrGeneric(const VDPixmapGenYCbCrBasis& basis, bool studioRGB, uint32 colorSpace)
	: mColorSpace(colorSpace)
{
	float scale;
	float bias;

	if (studioRGB) {
		scale = 255.0f / 219.0f;
		bias = -16.0f;
	} else {
		scale = 1.0f;
		bias = 0.0f;
	}


	// compute Y coefficients
	float coYR  = basis.mKr;
	float coYG  = (1.0f - basis.mKr - basis.mKb);
	float coYB  = basis.mKb;

	mCoYR  = VDRoundToInt32(scale * coYR  * 65536.0f);
	mCoYG  = VDRoundToInt32(scale * coYG  * 65536.0f);
	mCoYB  = VDRoundToInt32(scale * coYB  * 65536.0f);
	mCoYA  = 0x8000;

	// Cb = 0.5 * (B-Y) / (1-Kb)
	const float coCb = 0.5f / (1.0f - basis.mKb);
	float coCbR = (0.0f - coYR) * coCb;
	float coCbG = (0.0f - coYG) * coCb;
	float coCbB = (1.0f - coYB) * coCb;
	float coCbA = (coCbR + coCbG + coCbB) * bias;

	// Cr = 0.5 * (R-Y) / (1-Kr)
	const float coCr = 0.5f / (1.0f - basis.mKr);
	float coCrR = (1.0f - coYR) * coCr;
	float coCrG = (0.0f - coYG) * coCr;
	float coCrB = (0.0f - coYB) * coCr;
	float coCrA = (coCrR + coCrG + coCrB) * bias;

	mCoCbR = VDRoundToInt32(coCbR * 65536.0f);
	mCoCbG = VDRoundToInt32(coCbG * 65536.0f);
	mCoCbB = VDRoundToInt32(coCbB * 65536.0f);
	mCoCbA = VDRoundToInt32(coCbA * 65536.0f) + 0x808000;

	mCoCrR = VDRoundToInt32(coCrR * 65536.0f);
	mCoCrG = VDRoundToInt32(coCrG * 65536.0f);
	mCoCrB = VDRoundToInt32(coCrB * 65536.0f);
	mCoCrA = VDRoundToInt32(coCrA * 65536.0f) + 0x808000;
}

uint32 VDPixmapGenRGB32ToYCbCrGeneric::GetType(uint32 output) const {
	return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8 | mColorSpace;
}

void VDPixmapGenRGB32ToYCbCrGeneric::Compute(void *dst0, sint32 y) {
	uint8 *dstCr = (uint8 *)dst0;
	uint8 *dstY = dstCr + mWindowPitch;
	uint8 *dstCb = dstY + mWindowPitch;

	const uint8 *srcRGB = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	const sint32 coYR = mCoYR;
	const sint32 coYG = mCoYG;
	const sint32 coYB = mCoYB;
	const sint32 coCbR = mCoCbR;
	const sint32 coCbG = mCoCbG;
	const sint32 coCbB = mCoCbB;
	const sint32 coCrR = mCoCrR;
	const sint32 coCrG = mCoCrG;
	const sint32 coCrB = mCoCrB;
	const sint32 coYA = mCoYA;
	const sint32 coCbA = mCoCbA;
	const sint32 coCrA = mCoCrA;
	
	const sint32 w = mWidth;
	for(sint32 i=0; i<w; ++i) {
		int r = (int)srcRGB[2];
		int g = (int)srcRGB[1];
		int b = (int)srcRGB[0];
		srcRGB += 4;			

		// Normally, this can be optimized by encoding the chroma channels as
		// (R-Y) and (B-Y) differences. However, us working in fixed point complicates
		// things here, so for now we do a full 4x3 matrix.

		sint32 y16  = coYR  * r + coYG  * g + coYB  * b + coYA;
		sint32 cb16 = coCbR * r + coCbG * g + coCbB * b + coCbA;
		sint32 cr16 = coCrR * r + coCrG * g + coCrB * b + coCrA;

		// Quite annoyingly, we have to clip chroma on the high end since the transformation
		// targets [0,1] instead of [0,1). This occurs due to the bias by +0.5 to make
		// reference black for chroma fall on 128 instead of 127.5. The resulting
		// transformation is the one used for JFIF and also for TIFF with the full
		// range ReferenceBlack/ReferenceWhite values.

		cb16 |= (0xffffff - cb16) >> 31;
		cr16 |= (0xffffff - cr16) >> 31;

		*dstCb++ = (uint8)(cb16 >> 16);
		*dstY ++ = (uint8)( y16 >> 16);
		*dstCr++ = (uint8)(cr16 >> 16);
	}
}

////////////////////////////////////////////////////////////////////////////

VDPixmapGenRGB32FToYCbCrGeneric::VDPixmapGenRGB32FToYCbCrGeneric(const VDPixmapGenYCbCrBasis& basis, uint32 colorSpace)
	: mColorSpace(colorSpace)
{
	mCoYR = basis.mKr;
	mCoYG = 1.0f - basis.mKr - basis.mKb;
	mCoYB = basis.mKb;

	// Cb = 0.5 * (B-Y) / (1-Kb)
	mCoCb = 0.5f / (1.0f - basis.mKb);

	// Cr = 0.5 * (R-Y) / (1-Kr)
	mCoCr = 0.5f / (1.0f - basis.mKr);
}

uint32 VDPixmapGenRGB32FToYCbCrGeneric::GetType(uint32 output) const {
	return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32F_LE | mColorSpace;
}

void VDPixmapGenRGB32FToYCbCrGeneric::Compute(void *dst0, sint32 y) {
	float *dstCr = (float *)dst0;
	float *dstY  = dstCr + mWindowPitch;
	float *dstCb = dstY + mWindowPitch;

	const float *srcRGB = (const float *)mpSrc->GetRow(y, mSrcIndex);

	VDCPUCleanupExtensions();

	const float coYR = mCoYR;
	const float coYG = mCoYG;
	const float coYB = mCoYB;
	const float coCb = mCoCb;
	const float coCr = mCoCr;

	const sint32 w = mWidth;
	for(sint32 i=0; i<w; ++i) {
		float r = srcRGB[2];
		float g = srcRGB[1];
		float b = srcRGB[0];
		srcRGB += 4;

		float y = coYR * r + coYG * g + coYB * b;

		*dstY++  = y;
		*dstCb++ = coCb * (b - y) + (128.0f / 255.0f);
		*dstCr++ = coCr * (r - y) + (128.0f / 255.0f);
	}
}

////////////////////////////////////////////////////////////////////////////

VDPixmapGenYCbCrToYCbCrGeneric::VDPixmapGenYCbCrToYCbCrGeneric(const VDPixmapGenYCbCrBasis& dstBasis, bool dstLimitedRange, const VDPixmapGenYCbCrBasis& srcBasis, bool srcLimitedRange, uint32 colorSpace)
	: mColorSpace(colorSpace)
{
	vdfloat3x3 dstToRGB;
	dstToRGB.m[0] = vdfloat3c(1, 1, 1);
	dstToRGB.m[1] = vdfloat3c(dstBasis.mToRGB[0]);
	dstToRGB.m[2] = vdfloat3c(dstBasis.mToRGB[1]);

	if (dstLimitedRange) {
		dstToRGB.m[0] *= (255.0f / 219.0f);
		dstToRGB.m[1] *= (128.0f / 112.0f);
		dstToRGB.m[2] *= (128.0f / 112.0f);
	}

	vdfloat3x3 srcToRGB;
	srcToRGB.m[0] = vdfloat3c(1, 1, 1);
	srcToRGB.m[1] = vdfloat3c(srcBasis.mToRGB[0]);
	srcToRGB.m[2] = vdfloat3c(srcBasis.mToRGB[1]);

	if (srcLimitedRange) {
		srcToRGB.m[0] *= (255.0f / 219.0f);
		srcToRGB.m[1] *= (128.0f / 112.0f);
		srcToRGB.m[2] *= (128.0f / 112.0f);
	}

	vdfloat3x3 xf(srcToRGB * ~dstToRGB);

	// We should get a transform that looks like this:
	//
	//	            |k 0 0|
	//	[y cb cr 1] |a c e| = [y' cb' cr]
	//	            |b d f|
	//				|x y z|

	VDASSERT(fabsf(xf.m[0].v[1]) < 1e-5f);
	VDASSERT(fabsf(xf.m[0].v[2]) < 1e-5f);

	mCoYY   = VDRoundToInt32(xf.m[0].v[0] * 65536.0f);
	mCoYCb  = VDRoundToInt32(xf.m[1].v[0] * 65536.0f);
	mCoYCr  = VDRoundToInt32(xf.m[2].v[0] * 65536.0f);
	mCoCbCb = VDRoundToInt32(xf.m[1].v[1] * 65536.0f);
	mCoCbCr = VDRoundToInt32(xf.m[2].v[1] * 65536.0f);
	mCoCrCb = VDRoundToInt32(xf.m[1].v[2] * 65536.0f);
	mCoCrCr = VDRoundToInt32(xf.m[2].v[2] * 65536.0f);

	vdfloat3c srcBias(0, 128.0f/255.0f, 128.0f/255.0f);
	if (srcLimitedRange)
		srcBias.set(16.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);

	vdfloat3c dstBias(0, 128.0f/255.0f, 128.0f/255.0f);
	if (dstLimitedRange)
		dstBias.set(16.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);

	vdfloat3 bias = -srcBias * xf + dstBias;

	mCoYA  = VDRoundToInt32(bias.x * 255.0f * 65536.0f) + 0x8000;
	mCoCbA = VDRoundToInt32(bias.y * 255.0f * 65536.0f) + 0x8000;
	mCoCrA = VDRoundToInt32(bias.z * 255.0f * 65536.0f) + 0x8000;
}

void VDPixmapGenYCbCrToYCbCrGeneric::Start() {
	mpSrcY->Start();
	mpSrcCb->Start();
	mpSrcCr->Start();

	StartWindow(mWidth, 3);
}

const void *VDPixmapGenYCbCrToYCbCrGeneric::GetRow(sint32 y, uint32 index) {
	return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
}

uint32 VDPixmapGenYCbCrToYCbCrGeneric::GetType(uint32 output) const {
	return (mpSrcY->GetType(mSrcIndexY) & ~kVDPixSpace_Mask) | mColorSpace;
}

void VDPixmapGenYCbCrToYCbCrGeneric::Compute(void *dst0, sint32 ypos) {
	uint8 *dstCr = (uint8 *)dst0;
	uint8 *dstY  = dstCr + mWindowPitch;
	uint8 *dstCb = dstY + mWindowPitch;

	const uint8 *srcY  = (const uint8 *)mpSrcY ->GetRow(ypos, mSrcIndexY );
	const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
	const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

	const sint32 coYY   = mCoYY;
	const sint32 coYCb  = mCoYCb;
	const sint32 coYCr  = mCoYCr;
	const sint32 coYA   = mCoYA;
	const sint32 coCbCb = mCoCbCb;
	const sint32 coCbCr = mCoCbCr;
	const sint32 coCbA  = mCoCbA;
	const sint32 coCrCb = mCoCrCb;
	const sint32 coCrCr = mCoCrCr;
	const sint32 coCrA  = mCoCrA;

	for(sint32 i=0; i<mWidth; ++i) {
		sint32 y = srcY[i];
		sint32 cb = srcCb[i];
		sint32 cr = srcCr[i];

		sint32 y2  = y*coYY  + cb*coYCb  + cr*coYCr  + coYA;
		sint32 cb2 =           cb*coCbCb + cr*coCbCr + coCbA;
		sint32 cr2 =           cb*coCrCb + cr*coCrCr + coCrA;

		y2 &= ~y2 >> 31;
		cb2 &= ~cb2 >> 31;
		cr2 &= ~cr2 >> 31;

		y2 |= (0xffffff - y2) >> 31;
		cb2 |= (0xffffff - cb2) >> 31;
		cr2 |= (0xffffff - cr2) >> 31;

		*dstY++  = (uint8)(y2 >> 16);
		*dstCb++ = (uint8)(cb2 >> 16);
		*dstCr++ = (uint8)(cr2 >> 16);
	}
}

////////////////////////////////////////////////////////////////////////////

VDPixmapGenYCbCrToYCbCrGeneric_32F::VDPixmapGenYCbCrToYCbCrGeneric_32F(const VDPixmapGenYCbCrBasis& dstBasis, bool dstLimitedRange, const VDPixmapGenYCbCrBasis& srcBasis, bool srcLimitedRange, uint32 colorSpace)
	: mColorSpace(colorSpace)
{
	vdfloat3x3 dstToRGB;
	dstToRGB.m[0] = vdfloat3c(1, 1, 1);
	dstToRGB.m[1] = vdfloat3c(dstBasis.mToRGB[0]);
	dstToRGB.m[2] = vdfloat3c(dstBasis.mToRGB[1]);

	if (dstLimitedRange) {
		dstToRGB.m[0] *= (255.0f / 219.0f);
		dstToRGB.m[1] *= (255.0f / 224.0f);
		dstToRGB.m[2] *= (255.0f / 224.0f);
	}

	vdfloat3x3 srcToRGB;
	srcToRGB.m[0] = vdfloat3c(1, 1, 1);
	srcToRGB.m[1] = vdfloat3c(srcBasis.mToRGB[0]);
	srcToRGB.m[2] = vdfloat3c(srcBasis.mToRGB[1]);

	if (srcLimitedRange) {
		srcToRGB.m[0] *= (255.0f / 219.0f);
		srcToRGB.m[1] *= (255.0f / 224.0f);
		srcToRGB.m[2] *= (255.0f / 224.0f);
	}

	vdfloat3x3 xf(srcToRGB * ~dstToRGB);

	// We should get a transform that looks like this:
	//
	//	            |k 0 0|
	//	[y cb cr 1] |a c e| = [y' cb' cr]
	//	            |b d f|
	//				|x y z|

	VDASSERT(fabsf(xf.m[0].v[1]) < 1e-5f);
	VDASSERT(fabsf(xf.m[0].v[2]) < 1e-5f);

	mCoYY   = xf.m[0].v[0];
	mCoYCb  = xf.m[1].v[0];
	mCoYCr  = xf.m[2].v[0];
	mCoCbCb = xf.m[1].v[1];
	mCoCbCr = xf.m[2].v[1];
	mCoCrCb = xf.m[1].v[2];
	mCoCrCr = xf.m[2].v[2];

	vdfloat3c srcBias(0, 128.0f/255.0f, 128.0f/255.0f);
	if (srcLimitedRange)
		srcBias.set(16.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);

	vdfloat3c dstBias(0, 128.0f/255.0f, 128.0f/255.0f);
	if (dstLimitedRange)
		dstBias.set(16.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);

	vdfloat3 bias = -srcBias * xf + dstBias;

	mCoYA  = bias.x;
	mCoCbA = bias.y;
	mCoCrA = bias.z;
}

void VDPixmapGenYCbCrToYCbCrGeneric_32F::Start() {
	mpSrcY->Start();
	mpSrcCb->Start();
	mpSrcCr->Start();

	StartWindow(mWidth * sizeof(float), 3);
}

const void *VDPixmapGenYCbCrToYCbCrGeneric_32F::GetRow(sint32 y, uint32 index) {
	return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
}

uint32 VDPixmapGenYCbCrToYCbCrGeneric_32F::GetType(uint32 output) const {
	return (mpSrcY->GetType(mSrcIndexY) & ~kVDPixSpace_Mask) | mColorSpace;
}

void VDPixmapGenYCbCrToYCbCrGeneric_32F::Compute(void *dst0, sint32 ypos) {
	float *dstCr = (float *)dst0;
	float *dstY  = vdptroffset(dstCr, mWindowPitch);
	float *dstCb = vdptroffset(dstY, mWindowPitch);

	const float *srcY  = (const float *)mpSrcY ->GetRow(ypos, mSrcIndexY );
	const float *srcCb = (const float *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
	const float *srcCr = (const float *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

	VDCPUCleanupExtensions();

	const float coYY   = mCoYY;
	const float coYCb  = mCoYCb;
	const float coYCr  = mCoYCr;
	const float coYA   = mCoYA;
	const float coCbCb = mCoCbCb;
	const float coCbCr = mCoCbCr;
	const float coCbA  = mCoCbA;
	const float coCrCb = mCoCrCb;
	const float coCrCr = mCoCrCr;
	const float coCrA  = mCoCrA;

	for(sint32 i=0; i<mWidth; ++i) {
		float y  = srcY [i];
		float cb = srcCb[i];
		float cr = srcCr[i];

		*dstY++  = y*coYY + cb*coYCb  + cr*coYCr  + coYA;
		*dstCb++ =          cb*coCbCb + cr*coCbCr + coCbA;
		*dstCr++ =          cb*coCrCb + cr*coCrCr + coCrA;
	}
}
