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
#include <vd2/system/math.h>
#include <vd2/system/vdstl.h>
#include <vd2/Kasumi/resample_kernels.h>
#include "resample_stages.h"

VDSteppedAllocator::VDSteppedAllocator(size_t initialSize)
	: mpHead(NULL)
	, mpAllocNext(NULL)
	, mAllocLeft(0)
	, mAllocNext(initialSize)
	, mAllocInit(initialSize)
{
}

VDSteppedAllocator::~VDSteppedAllocator() {
	clear();
}

void VDSteppedAllocator::clear() {
	while(Block *p = mpHead) {
		mpHead = mpHead->next;
		free(p);
	}
	mAllocLeft = 0;
	mAllocNext = mAllocInit;
}

void *VDSteppedAllocator::allocate(size_type n) {
	n = (n+15) & ~15;
	if (mAllocLeft < n) {
		mAllocLeft = mAllocNext;
		mAllocNext += (mAllocNext >> 1);
		if (mAllocLeft < n)
			mAllocLeft = n;

		Block *t = (Block *)malloc(sizeof(Block) + mAllocLeft);

		if (mpHead)
			mpHead->next = t;

		mpHead = t;
		mpHead->next = NULL;

		mpAllocNext = (char *)(mpHead + 1);
	}

	void *p = mpAllocNext;
	mpAllocNext += n;
	mAllocLeft -= n;
	return p;
}

void VDResamplerGenerateTable(sint32 *dst, const IVDResamplerFilter& filter) {
	const unsigned width = filter.GetFilterWidth();
	vdblock<float> filters(width * 256);
	float *src = filters.data();

	filter.GenerateFilterBank(src);

	for(unsigned phase=0; phase < 256; ++phase) {
		float sum = 0;

		for(unsigned i=0; i<width; ++i)
			sum += src[i];

		float scalefac = 16384.0f / sum;

		for(unsigned j=0; j<width; j += 2) {
			int v0 = VDRoundToIntFast(src[j+0] * scalefac);
			int v1 = VDRoundToIntFast(src[j+1] * scalefac);

			dst[j+0] = v0;
			dst[j+1] = v1;
		}

		src += width;
		dst += width;
	}
}

void VDResamplerGenerateTableF(float *dst, const IVDResamplerFilter& filter) {
	const unsigned width = filter.GetFilterWidth();
	filter.GenerateFilterBank(dst);

	for(unsigned phase=0; phase < 256; ++phase) {
		float sum = 0;

		for(unsigned i=0; i<width; ++i)
			sum += dst[i];

		float scalefac = 1.0f / sum;

		for(unsigned j=0; j<width; ++j)
			*dst++ *= scalefac;
	}
}

void VDResamplerGenerateTable2(sint32 *dst, const IVDResamplerFilter& filter, sint32 count, sint32 u0, sint32 dudx) {
	const unsigned width = filter.GetFilterWidth();
	vdblock<float> filters(width);
	float *src = filters.data();

	filter.GenerateFilterBank(src);

	for(sint32 i=0; i<count; ++i) {
		sint32 u = u0 + dudx*i;

		*dst++ = u >> 16;
		filter.GenerateFilter(src, (double)(u & 0xffff) / 65536.0);

		float sum = 0;
		for(uint32 j=0; j<width; ++j)
			sum += src[j];

		float scalefac = 16384.0f / sum;

		sint32 isum = 0;
		for(uint32 j=0; j<width; ++j) {
			sint32 v = VDRoundToIntFast(src[j] * scalefac);

			dst[j] = v;
			isum += v;
		}

		sint32 ierr = 16384 - isum;
		sint32 idelta = 2*(ierr >> 31) - 1;
		while(ierr) {
			for(uint32 j=0; j<width && ierr; ++j) {
				if (!dst[j])
					continue;

				dst[j] += idelta;
				ierr -= idelta;
			}
		}

		dst += width;
	}
}

void VDResamplerSwizzleTable(sint32 *dst, unsigned pairs) {
	do {
		sint32 v0 = dst[0];
		sint32 v1 = dst[1];

		dst[0] = dst[1] = (v0 & 0xffff) + (v1<<16);
		dst += 2;
	} while(--pairs);
}
