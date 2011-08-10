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
#include <vd2/system/memory.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "resample_stages_reference.h"
#include <vd2/Kasumi/resample_kernels.h>
#include "blt_spanutils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

int VDResamplerRowStageSeparablePoint8::GetWindowSize() const {
	return 1;
}

void VDResamplerRowStageSeparablePoint8::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	do {
		*dst++ = src[u>>16];
		u += dudx;
	} while(--w);
}

int VDResamplerRowStageSeparablePoint16::GetWindowSize() const {
	return 1;
}

void VDResamplerRowStageSeparablePoint16::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint16 *dst = (uint16 *)dst0;
	const uint16 *src = (const uint16 *)src0;

	do {
		*dst++ = src[u>>16];
		u += dudx;
	} while(--w);
}

int VDResamplerRowStageSeparablePoint32::GetWindowSize() const {
	return 1;
}

void VDResamplerRowStageSeparablePoint32::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;

	do {
		*dst++ = src[u>>16];
		u += dudx;
	} while(--w);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int VDResamplerRowStageSeparableLinear8::GetWindowSize() const {return 2;}
void VDResamplerRowStageSeparableLinear8::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	do {
		const sint32 iu = u>>16;
		const uint32 p0 = src[iu];
		const uint32 p1 = src[iu+1];
		const uint32 f = (u >> 8) & 0xff;

		*dst++	= (uint8)(p0 + (((sint32)(p1 - p0)*f + 0x80)>>8));
		u += dudx;
	} while(--w);
}

void VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	VDASSERT(!u && dudx == 0x8000);

	nsVDPixmapSpanUtils::horiz_expand2x_coaligned(dst, src, w);
}

int VDResamplerRowStageSeparableLinear32::GetWindowSize() const {return 2;}
void VDResamplerRowStageSeparableLinear32::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;

	do {
		const sint32 iu = u>>16;
		const uint32 p0 = src[iu];
		const uint32 p1 = src[iu+1];
		const uint32 f = (u >> 8) & 0xff;

		const uint32 p0_rb = p0 & 0xff00ff;
		const uint32 p1_rb = p1 & 0xff00ff;
		const uint32 p0_g = p0 & 0xff00;
		const uint32 p1_g = p1 & 0xff00;

		*dst++	= ((p0_rb + (((p1_rb - p0_rb)*f + 0x800080)>>8)) & 0xff00ff)
				+ ((p0_g  + (((p1_g  - p0_g )*f + 0x008000)>>8)) & 0x00ff00);
		u += dudx;
	} while(--w);
}

int VDResamplerColStageSeparableLinear8::GetWindowSize() const {return 2;}
void VDResamplerColStageSeparableLinear8::Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src0 = (const uint8 *)srcarray[0];
	const uint8 *src1 = (const uint8 *)srcarray[1];
	const uint32 f = (phase >> 8) & 0xff;

	do {
		const uint32 p0 = *src0++;
		const uint32 p1 = *src1++;

		*dst++ = (uint8)(p0 + (((p1 - p0)*f + 0x80)>>8));
	} while(--w);
}

int VDResamplerColStageSeparableLinear32::GetWindowSize() const {return 2;}
void VDResamplerColStageSeparableLinear32::Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src0 = (const uint32 *)srcarray[0];
	const uint32 *src1 = (const uint32 *)srcarray[1];
	const uint32 f = (phase >> 8) & 0xff;

	do {
		const uint32 p0 = *src0++;
		const uint32 p1 = *src1++;

		const uint32 p0_rb = p0 & 0xff00ff;
		const uint32 p1_rb = p1 & 0xff00ff;
		const uint32 p0_g = p0 & 0xff00;
		const uint32 p1_g = p1 & 0xff00;

		*dst++	= ((p0_rb + (((p1_rb - p0_rb)*f + 0x800080)>>8)) & 0xff00ff)
				+ ((p0_g  + (((p1_g  - p0_g )*f + 0x008000)>>8)) & 0x00ff00);
	} while(--w);
}

VDResamplerRowStageSeparableTable8::VDResamplerRowStageSeparableTable8(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTable(mFilterBank.data(), filter);
}

int VDResamplerRowStageSeparableTable8::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerRowStageSeparableTable8::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;
	const unsigned ksize = (int)mFilterBank.size() >> 8;
	const sint32 *filterBase = mFilterBank.data();

	do {
		const uint8 *src2 = src + (u>>16);
		const sint32 *filter = filterBase + ksize*((u>>8)&0xff);
		u += dudx;

		int b = 0x2000;
		for(unsigned i = ksize; i; --i) {
			uint8 p = *src2++;
			sint32 coeff = *filter++;

			b += (sint32)p*coeff;
		}

		b >>= 14;

		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		*dst++ = (uint8)b;
	} while(--w);
}

VDResamplerRowStageSeparableTable32::VDResamplerRowStageSeparableTable32(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTable(mFilterBank.data(), filter);
}

int VDResamplerRowStageSeparableTable32::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerRowStageSeparableTable32::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;
	const unsigned ksize = (int)mFilterBank.size() >> 8;
	const sint32 *filterBase = mFilterBank.data();

	do {
		const uint32 *src2 = src + (u>>16);
		const sint32 *filter = filterBase + ksize*((u>>8)&0xff);
		u += dudx;

		int r = 0x2000, g = 0x2000, b = 0x2000;
		for(unsigned i = ksize; i; --i) {
			uint32 p = *src2++;
			sint32 coeff = *filter++;

			r += ((p>>16)&0xff)*coeff;
			g += ((p>> 8)&0xff)*coeff;
			b += ((p    )&0xff)*coeff;
		}

		r <<= 2;
		g >>= 6;
		b >>= 14;

		if ((uint32)r >= 0x01000000)
			r = ~r >> 31;
		if ((uint32)g >= 0x00010000)
			g = ~g >> 31;
		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		*dst++ = (r & 0xff0000) + (g & 0xff00) + (b & 0xff);
	} while(--w);
}

VDResamplerRowStageSeparableTable32Fx4::VDResamplerRowStageSeparableTable32Fx4(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTableF(mFilterBank.data(), filter);
}

int VDResamplerRowStageSeparableTable32Fx4::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerRowStageSeparableTable32Fx4::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	float *dst = (float *)dst0;
	const float *src = (const float *)src0;
	const unsigned ksize = (int)mFilterBank.size() >> 8;
	const float *filterBase = mFilterBank.data();

	do {
		const float *src2 = src + (u>>16)*4;
		const float *filter = filterBase + ksize*((u>>8)&0xff);
		u += dudx;

		float r = 0, g = 0, b = 0, a = 0;
		for(unsigned i = ksize; i; --i) {
			float coeff = *filter++;

			r += coeff * src2[0];
			g += coeff * src2[1];
			b += coeff * src2[2];
			a += coeff * src2[3];
			src2 += 4;
		}

		dst[0] = r;
		dst[1] = g;
		dst[2] = b;
		dst[3] = a;
		dst += 4;
	} while(--w);
}

VDResamplerRowStageSeparableTable32F::VDResamplerRowStageSeparableTable32F(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTableF(mFilterBank.data(), filter);
}

int VDResamplerRowStageSeparableTable32F::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerRowStageSeparableTable32F::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	float *dst = (float *)dst0;
	const float *src = (const float *)src0;
	const unsigned ksize = (int)mFilterBank.size() >> 8;
	const float *filterBase = mFilterBank.data();

	VDCPUCleanupExtensions();

	do {
		const float *src2 = src + (u>>16);
		const float *filter = filterBase + ksize*((u>>8)&0xff);
		u += dudx;

		float r = 0;
		for(unsigned i = ksize; i; --i) {
			float coeff = *filter++;

			r += coeff * src2[0];
			++src2;
		}

		dst[0] = r;
		++dst;
	} while(--w);
}

VDResamplerColStageSeparableTable8::VDResamplerColStageSeparableTable8(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTable(mFilterBank.data(), filter);
}

int VDResamplerColStageSeparableTable8::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerColStageSeparableTable8::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *const *src = (const uint8 *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const sint32 *filter = &mFilterBank[((phase>>8)&0xff) * ksize];

	for(uint32 i=0; i<w; ++i) {
		int b = 0x2000;
		const sint32 *filter2 = filter;
		const uint8 *const *src2 = src;

		for(unsigned j = ksize; j; --j) {
			sint32 p = (*src2++)[i];
			sint32 coeff = *filter2++;

			b += p*coeff;
		}

		b >>= 14;

		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		*dst++ = (uint8)b;
	}
}

VDResamplerColStageSeparableTable32::VDResamplerColStageSeparableTable32(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTable(mFilterBank.data(), filter);
}

int VDResamplerColStageSeparableTable32::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerColStageSeparableTable32::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *const *src = (const uint32 *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const sint32 *filter = &mFilterBank[((phase>>8)&0xff) * ksize];

	for(uint32 i=0; i<w; ++i) {
		int r = 0x2000, g = 0x2000, b = 0x2000;
		const sint32 *filter2 = filter;
		const uint32 *const *src2 = src;

		for(unsigned j = ksize; j; --j) {
			uint32 p = (*src2++)[i];
			sint32 coeff = *filter2++;

			r += ((p>>16)&0xff)*coeff;
			g += ((p>> 8)&0xff)*coeff;
			b += ((p    )&0xff)*coeff;
		}

		r <<= 2;
		g >>= 6;
		b >>= 14;

		if ((uint32)r >= 0x01000000)
			r = ~r >> 31;
		if ((uint32)g >= 0x00010000)
			g = ~g >> 31;
		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		*dst++ = (r & 0xff0000) + (g & 0xff00) + (b & 0xff);
	}
}

VDResamplerColStageSeparableTable32F::VDResamplerColStageSeparableTable32F(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTableF(mFilterBank.data(), filter);
}

int VDResamplerColStageSeparableTable32F::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerColStageSeparableTable32F::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	float *dst = (float *)dst0;
	const float *const *src = (const float *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const float *filter = &mFilterBank[((phase>>8)&0xff) * ksize];

	for(uint32 i=0; i<w; ++i) {
		float r = 0;
		const float *filter2 = filter;
		const float *const *src2 = src;

		for(unsigned j = ksize; j; --j) {
			const float *p = (*src2++) + i;
			float coeff = *filter2++;

			r += p[0]*coeff;
		}

		dst[0] = r;
		++dst;
	}
}

VDResamplerColStageSeparableTable32Fx4::VDResamplerColStageSeparableTable32Fx4(const IVDResamplerFilter& filter) {
	mFilterBank.resize(filter.GetFilterWidth() * 256);
	VDResamplerGenerateTableF(mFilterBank.data(), filter);
}

int VDResamplerColStageSeparableTable32Fx4::GetWindowSize() const {return (int)mFilterBank.size() >> 8;}

void VDResamplerColStageSeparableTable32Fx4::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	float *dst = (float *)dst0;
	const float *const *src = (const float *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const float *filter = &mFilterBank[((phase>>8)&0xff) * ksize];

	for(uint32 i=0; i<w; ++i) {
		float r = 0, g = 0, b = 0, a = 0;
		const float *filter2 = filter;
		const float *const *src2 = src;

		for(unsigned j = ksize; j; --j) {
			const float *p = (*src2++) + i*4;
			float coeff = *filter2++;

			r += p[0]*coeff;
			g += p[1]*coeff;
			b += p[2]*coeff;
			a += p[3]*coeff;
		}

		dst[0] = r;
		dst[1] = g;
		dst[2] = b;
		dst[3] = a;
		dst += 4;
	}
}
