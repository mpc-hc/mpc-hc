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
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Kasumi/region.h>
#include <vd2/system/math.h>
#include <vd2/system/vdstl.h>

void VDPixmapRegion::clear() {
	mSpans.clear();
}

void VDPixmapRegion::swap(VDPixmapRegion& x) {
	mSpans.swap(x.mSpans);
	std::swap(mBounds, x.mBounds);
}

VDPixmapPathRasterizer::VDPixmapPathRasterizer()
	: mpEdgeBlocks(NULL)
	, mpFreeEdgeBlocks(NULL)
	, mEdgeBlockIdx(kEdgeBlockMax)
	, mpScanBuffer(NULL)
{
	ClearScanBuffer();
}

VDPixmapPathRasterizer::VDPixmapPathRasterizer(const VDPixmapPathRasterizer&)
	: mpEdgeBlocks(NULL)
	, mpFreeEdgeBlocks(NULL)
	, mEdgeBlockIdx(kEdgeBlockMax)
	, mpScanBuffer(NULL)
{
	ClearScanBuffer();
}

VDPixmapPathRasterizer::~VDPixmapPathRasterizer() {
	Clear();
	FreeEdgeLists();
}

VDPixmapPathRasterizer& VDPixmapPathRasterizer::operator=(const VDPixmapPathRasterizer&) {
	return *this;
}

void VDPixmapPathRasterizer::Clear() {
	ClearEdgeList();
	ClearScanBuffer();
}

void VDPixmapPathRasterizer::QuadraticBezier(const vdint2 *pts) {
	int x0 = pts[0].x;
	int x1 = pts[1].x;
	int x2 = pts[2].x;
	int y0 = pts[0].y;
	int y1 = pts[1].y;
	int y2 = pts[2].y;

	// P = (1-t)^2*P0 + 2t(1-t)*P1 + t^2*P2
	// P = (1-2t+t^2)P0 + 2(t-t^2)P1 + t^2*P2
	// P = (P0-2P1+P2)t^2 + 2(P1-P0)t + P0

	int cx2 =    x0-2*x1+x2;
	int cx1 = -2*x0+2*x1;
	int cx0 =    x0;

	int cy2 =    y0-2*y1+y2;
	int cy1 = -2*y0+2*y1;
	int cy0 =    y0;

	// This equation is from Graphics Gems I.
	//
	// The idea is that since we're approximating a cubic curve with lines,
	// any error we incur is due to the curvature of the line, which we can
	// estimate by calculating the maximum acceleration of the curve.  For
	// a cubic, the acceleration (second derivative) is a line, meaning that
	// the absolute maximum acceleration must occur at either the beginning
	// (|c2|) or the end (|c2+c3|).  Our bounds here are a little more
	// conservative than that, but that's okay.
	//
	// If the acceleration of the parametric formula is zero (c2 = c3 = 0),
	// that component of the curve is linear and does not incur any error.
	// If a=0 for both X and Y, the curve is a line segment and we can
	// use a step size of 1.

	int maxaccel1 = abs(cy2);
	int maxaccel2 = abs(cx2);

	int maxaccel = maxaccel1 > maxaccel2 ? maxaccel1 : maxaccel2;
	int h = 1;

	while(maxaccel > 8 && h < 1024) {
		maxaccel >>= 2;
		h += h;
	}

	int lastx = x0;
	int lasty = y0;

	// compute forward differences
	sint64 h1 = (sint64)(0x40000000 / h) << 2;
	sint64 h2 = h1/h;

	sint64 ax0 = (sint64)cx0 << 32;
	sint64 ax1 =   h1*(sint64)cx1 +   h2*(sint64)cx2;
	sint64 ax2 = 2*h2*(sint64)cx2;

	sint64 ay0 = (sint64)cy0 << 32;
	sint64 ay1 =   h1*(sint64)cy1 +   h2*(sint64)cy2;
	sint64 ay2 = 2*h2*(sint64)cy2;

	// round, not truncate
	ax0 += 0x80000000;
	ay0 += 0x80000000;

	do {
		ax0 += ax1;
		ax1 += ax2;
		ay0 += ay1;
		ay1 += ay2;

		int xi = (int)((uint64)ax0 >> 32);
		int yi = (int)((uint64)ay0 >> 32);

		FastLine(lastx, lasty, xi, yi);
		lastx = xi;
		lasty = yi;
	} while(--h);
}

void VDPixmapPathRasterizer::CubicBezier(const vdint2 *pts) {
	int x0 = pts[0].x;
	int x1 = pts[1].x;
	int x2 = pts[2].x;
	int x3 = pts[3].x;
	int y0 = pts[0].y;
	int y1 = pts[1].y;
	int y2 = pts[2].y;
	int y3 = pts[3].y;

	int cx3 = -  x0+3*x1-3*x2+x3;
	int cx2 =  3*x0-6*x1+3*x2;
	int cx1 = -3*x0+3*x1;
	int cx0 =    x0;

	int cy3 = -  y0+3*y1-3*y2+y3;
	int cy2 =  3*y0-6*y1+3*y2;
	int cy1 = -3*y0+3*y1;
	int cy0 =    y0;

	// This equation is from Graphics Gems I.
	//
	// The idea is that since we're approximating a cubic curve with lines,
	// any error we incur is due to the curvature of the line, which we can
	// estimate by calculating the maximum acceleration of the curve.  For
	// a cubic, the acceleration (second derivative) is a line, meaning that
	// the absolute maximum acceleration must occur at either the beginning
	// (|c2|) or the end (|c2+c3|).  Our bounds here are a little more
	// conservative than that, but that's okay.
	//
	// If the acceleration of the parametric formula is zero (c2 = c3 = 0),
	// that component of the curve is linear and does not incur any error.
	// If a=0 for both X and Y, the curve is a line segment and we can
	// use a step size of 1.

	int maxaccel1 = abs(2*cy2) + abs(6*cy3);
	int maxaccel2 = abs(2*cx2) + abs(6*cx3);

	int maxaccel = maxaccel1 > maxaccel2 ? maxaccel1 : maxaccel2;
	int h = 1;

	while(maxaccel > 8 && h < 1024) {
		maxaccel >>= 2;
		h += h;
	}

	int lastx = x0;
	int lasty = y0;

	// compute forward differences
	sint64 h1 = (sint64)(0x40000000 / h) << 2;
	sint64 h2 = h1/h;
	sint64 h3 = h2/h;

	sint64 ax0 = (sint64)cx0 << 32;
	sint64 ax1 =   h1*(sint64)cx1 +   h2*(sint64)cx2 + h3*(sint64)cx3;
	sint64 ax2 = 2*h2*(sint64)cx2 + 6*h3*(sint64)cx3;
	sint64 ax3 = 6*h3*(sint64)cx3;

	sint64 ay0 = (sint64)cy0 << 32;
	sint64 ay1 =   h1*(sint64)cy1 +   h2*(sint64)cy2 + h3*(sint64)cy3;
	sint64 ay2 = 2*h2*(sint64)cy2 + 6*h3*(sint64)cy3;
	sint64 ay3 = 6*h3*(sint64)cy3;

	// round, not truncate
	ax0 += 0x80000000;
	ay0 += 0x80000000;

	do {
		ax0 += ax1;
		ax1 += ax2;
		ax2 += ax3;
		ay0 += ay1;
		ay1 += ay2;
		ay2 += ay3;

		int xi = (int)((uint64)ax0 >> 32);
		int yi = (int)((uint64)ay0 >> 32);

		FastLine(lastx, lasty, xi, yi);
		lastx = xi;
		lasty = yi;
	} while(--h);
}

void VDPixmapPathRasterizer::Line(const vdint2& pt1, const vdint2& pt2) {
	FastLine(pt1.x, pt1.y, pt2.x, pt2.y);
}

void VDPixmapPathRasterizer::FastLine(int x0, int y0, int x1, int y1) {
	int flag = 1;

	if (y1 == y0)
		return;

	if (y1 < y0) {
		int t;

		t=x0; x0=x1; x1=t;
		t=y0; y0=y1; y1=t;
		flag = 0;
	}

	int dy = y1-y0;
	int xacc = x0<<13;

	// prestep y0 down
	int iy0 = (y0+3) >> 3;
	int iy1 = (y1+3) >> 3;

	if (iy0 < iy1) {
		int invslope = (x1-x0)*65536/dy;

		int prestep = (4-y0) & 7;
		xacc += (invslope * prestep)>>3;

		if (iy0 < mScanYMin || iy1 > mScanYMax) {
			ReallocateScanBuffer(iy0, iy1);
			VDASSERT(iy0 >= mScanYMin && iy1 <= mScanYMax);
		}

		while(iy0 < iy1) {
			int ix = (xacc+32767)>>16;

			if (mEdgeBlockIdx >= kEdgeBlockMax) {
				if (mpFreeEdgeBlocks) {
					EdgeBlock *newBlock = mpFreeEdgeBlocks;
					mpFreeEdgeBlocks = mpFreeEdgeBlocks->next;
					newBlock->next = mpEdgeBlocks;
					mpEdgeBlocks = newBlock;
				} else {
					mpEdgeBlocks = new EdgeBlock(mpEdgeBlocks);
				}

				mEdgeBlockIdx = 0;
			}

			Edge& e = mpEdgeBlocks->edges[mEdgeBlockIdx];
			Scan& s = mpScanBufferBiased[iy0];
			VDASSERT(iy0 >= mScanYMin && iy0 < mScanYMax);
			++mEdgeBlockIdx;

			e.posandflag = ix*2+flag;
			e.next = s.chain;
			s.chain = &e;
			++s.count;

			++iy0;
			xacc += invslope;
		}
	}
}

void VDPixmapPathRasterizer::ScanConvert(VDPixmapRegion& region) {
	// Convert the edges to spans.  We couldn't do this before because some of
	// the regions may have winding numbers >+1 and it would have been a pain
	// to try to adjust the spans on the fly.  We use one heap to detangle
	// a scanline's worth of edges from the singly-linked lists, and another
	// to collect the actual scans.
	vdfastvector<int> heap;

	region.mSpans.clear();
	int xmin = INT_MAX;
	int xmax = INT_MIN;
	int ymin = INT_MAX;
	int ymax = INT_MIN;

	for(int y=mScanYMin; y<mScanYMax; ++y) {
		uint32 flipcount = mpScanBufferBiased[y].count;

		if (!flipcount)
			continue;

		// Keep the edge heap from doing lots of stupid little reallocates.
		if (heap.capacity() < flipcount)
			heap.resize((flipcount + 63)&~63);

		// Detangle scanline into edge heap.
		int *heap0 = heap.data();
		int *heap1 = heap0;
		for(const Edge *ptr = mpScanBufferBiased[y].chain; ptr; ptr = ptr->next)
			*heap1++ = ptr->posandflag;

		VDASSERT(heap1 - heap0 == flipcount);

		// Sort edge heap.  Note that we conveniently made the opening edges
		// one more than closing edges at the same spot, so we won't have any
		// problems with abutting spans.

		std::sort(heap0, heap1);

#if 0
		while(heap0 != heap1) {
			int x = *heap0++ >> 1;
			region.mSpans.push_back((y<<16) + x + 0x80008000);
			region.mSpans.push_back((y<<16) + x + 0x80008001);
		}
		continue;
#endif

		// Trim any odd edges off, since we can never close on one.
		if (flipcount & 1)
			--heap1;

		// Process edges and add spans.  Since we only check for a non-zero
		// winding number, it doesn't matter which way the outlines go. Also, since
		// the parity always flips after each edge regardless of direction, we can
		// process the edges in pairs.

		size_t spanstart = region.mSpans.size();

		int x_left;
		int count = 0;
		while(heap0 != heap1) {
			int x = *heap0++;

			if (!count)
				x_left = (x>>1);

			count += (x&1);

			x = *heap0++;

			count += (x&1);

			if (!--count) {
				int x_right = (x>>1);

				if (x_right > x_left) {
					region.mSpans.push_back((y<<16) + x_left  + 0x80008000);
					region.mSpans.push_back((y<<16) + x_right + 0x80008000);

				}
			}
		}

		size_t spanend = region.mSpans.size();

		if (spanend > spanstart) {
			if (ymin > y)
				ymin = y;

			if (ymax < y)
				ymax = y;

			int x1 = (region.mSpans[spanstart] & 0xffff) - 0x8000;
			int x2 = (region.mSpans[spanend-1] & 0xffff) - 0x8000;

			if (xmin > x1)
				xmin = x1;

			if (xmax < x2)
				xmax = x2;
		}
	}

	if (xmax > xmin) {
		region.mBounds.set(xmin, ymin, xmax, ymax);
	} else {
		region.mBounds.set(0, 0, 0, 0);
	}

	// Dump the edge and scan buffers, since we no longer need them.
	ClearEdgeList();
	ClearScanBuffer();
}

void VDPixmapPathRasterizer::ClearEdgeList() {
	if (mpEdgeBlocks) {
		EdgeBlock *block = mpEdgeBlocks;
		
		while(EdgeBlock *next = block->next)
			block = next;

		block->next = mpFreeEdgeBlocks;
		mpFreeEdgeBlocks = mpEdgeBlocks;
		mpEdgeBlocks = NULL;
	}

	mEdgeBlockIdx = kEdgeBlockMax;
}

void VDPixmapPathRasterizer::FreeEdgeLists() {
	ClearEdgeList();

	while(EdgeBlock *block = mpFreeEdgeBlocks) {
		mpFreeEdgeBlocks = block->next;

		delete block;
	}
}

void VDPixmapPathRasterizer::ClearScanBuffer() {
	delete[] mpScanBuffer;
	mpScanBuffer = mpScanBufferBiased = NULL;
	mScanYMin = 0;
	mScanYMax = 0;
}

void VDPixmapPathRasterizer::ReallocateScanBuffer(int ymin, int ymax) {
	// 
	// check if there actually is a scan buffer to avoid unintentionally pinning at zero
	if (mpScanBuffer) {
		int nicedelta = (mScanYMax - mScanYMin);

		if (ymin < mScanYMin) {
			int yminnice = mScanYMin - nicedelta;
			if (ymin > yminnice)
				ymin = yminnice;

			ymin &= ~31;
		} else
			ymin = mScanYMin;

		if (ymax > mScanYMax) {
			int ymaxnice = mScanYMax + nicedelta;
			if (ymax < ymaxnice)
				ymax = ymaxnice;

			ymax = (ymax + 31) & ~31;
		} else
			ymax = mScanYMax;

		VDASSERT(ymin <= mScanYMin && ymax >= mScanYMax);
	}

	// reallocate scan buffer
	Scan *pNewBuffer = new Scan[ymax - ymin];
	Scan *pNewBufferBiased = pNewBuffer - ymin;

	if (mpScanBuffer) {
		memcpy(pNewBufferBiased + mScanYMin, mpScanBufferBiased + mScanYMin, (mScanYMax - mScanYMin) * sizeof(Scan));
		delete[] mpScanBuffer;

		// zero new areas of scan buffer
		for(int y=ymin; y<mScanYMin; ++y) {
			pNewBufferBiased[y].chain = NULL;
			pNewBufferBiased[y].count = 0;
		}

		for(int y=mScanYMax; y<ymax; ++y) {
			pNewBufferBiased[y].chain = NULL;
			pNewBufferBiased[y].count = 0;
		}
	} else {
		for(int y=ymin; y<ymax; ++y) {
			pNewBufferBiased[y].chain = NULL;
			pNewBufferBiased[y].count = 0;
		}
	}

	mpScanBuffer = pNewBuffer;
	mpScanBufferBiased = pNewBufferBiased;
	mScanYMin = ymin;
	mScanYMax = ymax;
}

bool VDPixmapFillRegion(const VDPixmap& dst, const VDPixmapRegion& region, int x, int y, uint32 color) {
	if (dst.format != nsVDPixmap::kPixFormat_XRGB8888)
		return false;

	// fast out
	if (region.mSpans.empty())
		return true;

	// check if vertical clipping is required
	const size_t n = region.mSpans.size();
	uint32 start = 0;
	uint32 end = n;

	uint32 spanmin = (-x) + ((-y) << 16) + 0x80008000;

	if (region.mSpans.front() < spanmin) {
		uint32 lo = 0, hi = n;

		// compute top clip
		while(lo < hi) {
			int mid = ((lo + hi) >> 1) & ~1;

			if (region.mSpans[mid + 1] < spanmin)
				lo = mid + 2;
			else
				hi = mid;
		}

		start = lo;

		// check for total top clip
		if (start >= n)
			return true;
	}

	uint32 spanlimit = (dst.w - x) + ((dst.h - y - 1) << 16) + 0x80008000;

	if (region.mSpans.back() > spanlimit) {
		// compute bottom clip
		int lo = start;
		int hi = n;

		while(lo < hi) {
			int mid = ((lo + hi) >> 1) & ~1;

			if (region.mSpans[mid] >= spanlimit)
				hi = mid;
			else
				lo = mid+2;
		}

		end = lo;

		// check for total bottom clip
		if (start >= end)
			return true;
	}

	// fill region
	const uint32 *pSpan = &region.mSpans[start];
	const uint32 *pEnd  = &region.mSpans[0] + end;
	int lasty = -1;
	uint32 *dstp;

	for(; pSpan != pEnd; pSpan += 2) {
		uint32 span0 = pSpan[0];
		uint32 span1 = pSpan[1];

		uint32 py = (span0 >> 16) - 0x8000 + y;
		uint32 px = (span0 & 0xffff) - 0x8000 + x;
		uint32 w = span1-span0;

		VDASSERT(py < (uint32)dst.h);
		VDASSERT(px < (uint32)dst.w);
		VDASSERT(dst.w - (int)px >= (int)w);

		if (lasty != py)
			dstp = (uint32 *)vdptroffset(dst.data, dst.pitch * py);

		uint32 *p = dstp + px;
		do {
			*p++ = color;
		} while(--w);
	}

	return true;
}

namespace {
	void RenderABuffer32(const VDPixmap& dst, int y, const uint8 *alpha, uint32 w, uint32 color) {
		if (!w)
			return;

		// update dest pointer
		uint32 *dstp = (uint32 *)vdptroffset(dst.data, dst.pitch * y);

		const uint32 color_rb = color & 0x00FF00FF;
		const uint32 color_g  = color & 0x0000FF00;
		do {
			const uint32 px = *dstp;
			const uint32 px_rb = px & 0x00FF00FF;
			const uint32 px_g  = px & 0x0000FF00;
			const sint32 a     = *alpha++;

			const uint32 result_rb = (((px_rb << 6) + ((sint32)(color_rb - px_rb)*a + 0x00200020)) & 0x3FC03FC0);
			const uint32 result_g  = (((px_g  << 6) + ((sint32)(color_g  - px_g )*a + 0x00002000)) & 0x003FC000);

			*dstp++ = (result_rb + result_g) >> 6;
		} while(--w);
	}

	void RenderABuffer8(const VDPixmap& dst, int y, const uint8 *alpha, uint32 w, uint32 color) {
		if (!w)
			return;

		// update dest pointer
		uint8 *dstp = (uint8 *)vdptroffset(dst.data, dst.pitch * y);

		do {
			const uint8 px = *dstp;
			const sint8 a = *alpha++;

			*dstp++ = px + (((sint32)(color - px) * a + 32) >> 6);
		} while(--w);
	}

	void RenderABuffer8_128(const VDPixmap& dst, int y, const uint8 *alpha, uint32 w, uint32 color) {
		if (!w)
			return;

		// update dest pointer
		uint8 *dstp = (uint8 *)vdptroffset(dst.data, dst.pitch * y);

		do {
			const uint8 px = *dstp;
			const sint16 a = *alpha++;

			*dstp++ = px + (((sint32)(color - px) * a + 64) >> 7);
		} while(--w);
	}

	void RenderABuffer8_256(const VDPixmap& dst, int y, const uint16 *alpha, uint32 w, uint32 color) {
		if (!w)
			return;

		// update dest pointer
		uint8 *dstp = (uint8 *)vdptroffset(dst.data, dst.pitch * y);

		do {
			const uint8 px = *dstp;
			const sint32 a = *alpha++;

			*dstp++ = px + (((sint32)(color - px) * a + 128) >> 8);
		} while(--w);
	}

	void RenderABuffer8_1024(const VDPixmap& dst, int y, const uint16 *alpha, uint32 w, uint32 color) {
		if (!w)
			return;

		// update dest pointer
		uint8 *dstp = (uint8 *)vdptroffset(dst.data, dst.pitch * y);

		do {
			const uint8 px = *dstp;
			const sint32 a = *alpha++;

			*dstp++ = px + (((sint32)(color - px) * a + 512) >> 10);
		} while(--w);
	}
}

bool VDPixmapFillRegionAntialiased_32x_32x(const VDPixmap& dst, const VDPixmapRegion& region, int x, int y, uint32 color) {
	if (dst.format != nsVDPixmap::kPixFormat_Y8)
		return false;

	// fast out
	if (region.mSpans.empty())
		return true;

	// check if vertical clipping is required
	const size_t n = region.mSpans.size();
	uint32 start = 0;
	uint32 end = n;

	uint32 spanmin = -x + (-y << 16) + 0x80008000;

	if (region.mSpans.front() < spanmin) {
		// find first span : x2 > spanmin
		start = std::upper_bound(region.mSpans.begin(), region.mSpans.end(), spanmin) - region.mSpans.begin();
		start &= ~1;

		// check for total top clip
		if (start >= n)
			return true;
	}

	uint32 spanlimit = (dst.w*32 - x) + (((dst.h*32 - y) - 1) << 16) + 0x80008000;

	if (region.mSpans.back() > spanlimit) {
		// find last span : x1 < spanlimit
		end = std::lower_bound(region.mSpans.begin(), region.mSpans.end(), spanlimit) - region.mSpans.begin();

		end = (end + 1) & ~1;

		// check for total bottom clip
		if (start >= end)
			return true;
	}

	// allocate A-buffer
	vdfastvector<uint16> abuffer(dst.w, 0);

	// fill region
	const uint32 *pSpan = &region.mSpans[start];
	const uint32 *pEnd  = &region.mSpans[0] + end;
	int lasty = -1;

	sint32 dstw32 = dst.w * 32;
	sint32 dsth32 = dst.h * 32;

	for(; pSpan != pEnd; pSpan += 2) {
		uint32 span0 = pSpan[0];
		uint32 span1 = pSpan[1];

		sint32 py = (span0 >> 16) - 0x8000 + y;

		if ((uint32)py >= (uint32)dsth32)
			continue;

		sint32 px1 = (span0 & 0xffff) - 0x8000 + x;
		sint32 px2 = (span1 & 0xffff) - 0x8000 + x;
		sint32 w = span1-span0;

		if (lasty != py) {
			if (((lasty ^ py) & 0xFFFFFFE0)) {
				if (lasty >= 0) {
					// flush scanline

					RenderABuffer8_1024(dst, lasty >> 5, abuffer.data(), dst.w, color);
				}

				memset(abuffer.data(), 0, abuffer.size() * sizeof(abuffer[0]));
			}
			lasty = py;
		}

		if (px1 < 0)
			px1 = 0;
		if (px2 > dstw32)
			px2 = dstw32;

		if (px1 >= px2)
			continue;

		uint32 ix1 = px1 >> 5;
		uint32 ix2 = px2 >> 5;
		uint16 *p1 = abuffer.data() + ix1;
		uint16 *p2 = abuffer.data() + ix2;

		if (p1 == p2) {
			p1[0] += (px2 - px1);
		} else {
			if (px1 & 31) {
				p1[0] += 32 - (px1 & 31);
				++p1;
			}

			while(p1 != p2) {
				p1[0] += 32;
				++p1;
			}

			if (px2 & 31)
				p1[0] += px2 & 32;
		}
	}

	if (lasty >= 0)
		RenderABuffer8_1024(dst, lasty >> 5, abuffer.data(), dst.w, color);

	return true;
}

bool VDPixmapFillRegionAntialiased_16x_16x(const VDPixmap& dst, const VDPixmapRegion& region, int x, int y, uint32 color) {
	if (dst.format != nsVDPixmap::kPixFormat_Y8)
		return false;

	// fast out
	if (region.mSpans.empty())
		return true;

	// check if vertical clipping is required
	const size_t n = region.mSpans.size();
	uint32 start = 0;
	uint32 end = n;

	uint32 spanmin = -x + (-y << 16) + 0x80008000;

	if (region.mSpans.front() < spanmin) {
		// find first span : x2 > spanmin
		start = std::upper_bound(region.mSpans.begin(), region.mSpans.end(), spanmin) - region.mSpans.begin();
		start &= ~1;

		// check for total top clip
		if (start >= n)
			return true;
	}

	uint32 spanlimit = (dst.w*16 - x) + (((dst.h*16 - y) - 1) << 16) + 0x80008000;

	if (region.mSpans.back() > spanlimit) {
		// find last span : x1 < spanlimit
		end = std::lower_bound(region.mSpans.begin(), region.mSpans.end(), spanlimit) - region.mSpans.begin();

		end = (end + 1) & ~1;

		// check for total bottom clip
		if (start >= end)
			return true;
	}

	// allocate A-buffer
	vdfastvector<uint16> abuffer(dst.w, 0);

	// fill region
	const uint32 *pSpan = &region.mSpans[start];
	const uint32 *pEnd  = &region.mSpans[0] + end;
	int lasty = -1;

	sint32 dstw16 = dst.w * 16;
	sint32 dsth16 = dst.h * 16;

	for(; pSpan != pEnd; pSpan += 2) {
		uint32 span0 = pSpan[0];
		uint32 span1 = pSpan[1];

		sint32 py = (span0 >> 16) - 0x8000 + y;

		if ((uint32)py >= (uint32)dsth16)
			continue;

		sint32 px1 = (span0 & 0xffff) - 0x8000 + x;
		sint32 px2 = (span1 & 0xffff) - 0x8000 + x;
		sint32 w = span1-span0;

		if (lasty != py) {
			if (((lasty ^ py) & 0xFFFFFFF0)) {
				if (lasty >= 0) {
					// flush scanline

					RenderABuffer8_256(dst, lasty >> 4, abuffer.data(), dst.w, color);
				}

				memset(abuffer.data(), 0, abuffer.size() * sizeof(abuffer[0]));
			}
			lasty = py;
		}

		if (px1 < 0)
			px1 = 0;
		if (px2 > dstw16)
			px2 = dstw16;

		if (px1 >= px2)
			continue;

		uint32 ix1 = px1 >> 4;
		uint32 ix2 = px2 >> 4;
		uint16 *p1 = abuffer.data() + ix1;
		uint16 *p2 = abuffer.data() + ix2;

		if (p1 == p2) {
			p1[0] += (px2 - px1);
		} else {
			if (px1 & 15) {
				p1[0] += 16 - (px1 & 15);
				++p1;
			}

			while(p1 != p2) {
				p1[0] += 16;
				++p1;
			}

			if (px2 & 15)
				p1[0] += px2 & 15;
		}
	}

	if (lasty >= 0)
		RenderABuffer8_256(dst, lasty >> 4, abuffer.data(), dst.w, color);

	return true;
}

bool VDPixmapFillRegionAntialiased_16x_8x(const VDPixmap& dst, const VDPixmapRegion& region, int x, int y, uint32 color) {
	if (dst.format != nsVDPixmap::kPixFormat_XRGB8888 && dst.format != nsVDPixmap::kPixFormat_Y8)
		return false;

	// fast out
	if (region.mSpans.empty())
		return true;

	// check if vertical clipping is required
	const size_t n = region.mSpans.size();
	uint32 start = 0;
	uint32 end = n;

	uint32 spanmin = -x + (-y << 16) + 0x80008000;

	if (region.mSpans.front() < spanmin) {
		// find first span : x2 > spanmin
		start = std::upper_bound(region.mSpans.begin(), region.mSpans.end(), spanmin) - region.mSpans.begin();
		start &= ~1;

		// check for total top clip
		if (start >= n)
			return true;
	}

	uint32 spanlimit = (dst.w*16 - x) + (((dst.h*8 - y) - 1) << 16) + 0x80008000;

	if (region.mSpans.back() > spanlimit) {
		// find last span : x1 < spanlimit
		end = std::lower_bound(region.mSpans.begin(), region.mSpans.end(), spanlimit) - region.mSpans.begin();

		end = (end + 1) & ~1;

		// check for total bottom clip
		if (start >= end)
			return true;
	}

	// allocate A-buffer
	vdfastvector<uint8> abuffer(dst.w, 0);

	// fill region
	const uint32 *pSpan = &region.mSpans[start];
	const uint32 *pEnd  = &region.mSpans[0] + end;
	int lasty = -1;

	sint32 dstw16 = dst.w * 16;
	sint32 dsth8 = dst.h * 8;

	for(; pSpan != pEnd; pSpan += 2) {
		uint32 span0 = pSpan[0];
		uint32 span1 = pSpan[1];

		sint32 py = (span0 >> 16) - 0x8000 + y;

		if ((uint32)py >= (uint32)dsth8)
			continue;

		sint32 px1 = (span0 & 0xffff) - 0x8000 + x;
		sint32 px2 = (span1 & 0xffff) - 0x8000 + x;
		sint32 w = span1-span0;

		if (lasty != py) {
			if (((lasty ^ py) & 0xFFFFFFF8)) {
				if (lasty >= 0) {
					// flush scanline

					RenderABuffer8_128(dst, lasty >> 3, abuffer.data(), dst.w, color);
				}

				memset(abuffer.data(), 0, abuffer.size());
			}
			lasty = py;
		}

		if (px1 < 0)
			px1 = 0;
		if (px2 > dstw16)
			px2 = dstw16;

		if (px1 >= px2)
			continue;

		uint32 ix1 = px1 >> 4;
		uint32 ix2 = px2 >> 4;
		uint8 *p1 = abuffer.data() + ix1;
		uint8 *p2 = abuffer.data() + ix2;

		if (p1 == p2) {
			p1[0] += (px2 - px1);
		} else {
			if (px1 & 15) {
				p1[0] += 16 - (px1 & 15);
				++p1;
			}

			while(p1 != p2) {
				p1[0] += 16;
				++p1;
			}

			if (px2 & 15)
				p1[0] += px2 & 15;
		}
	}

	if (lasty >= 0)
		RenderABuffer8_128(dst, lasty >> 3, abuffer.data(), dst.w, color);

	return true;
}

bool VDPixmapFillRegionAntialiased8x(const VDPixmap& dst, const VDPixmapRegion& region, int x, int y, uint32 color) {
	switch(dst.format) {
	case nsVDPixmap::kPixFormat_YUV444_Planar:
	case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV444_Planar_709:
	case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV422_Planar:
	case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV422_Planar_709:
	case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV420_Planar:
	case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV420_Planar_709:
	case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV410_Planar:
	case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV410_Planar_709:
	case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
		{
			VDPixmap pxY;
			VDPixmap pxCb;
			VDPixmap pxCr;

			pxY.format = nsVDPixmap::kPixFormat_Y8;
			pxY.data = dst.data;
			pxY.pitch = dst.pitch;
			pxY.w = dst.w;
			pxY.h = dst.h;

			pxCb.format = nsVDPixmap::kPixFormat_Y8;
			pxCb.data = dst.data2;
			pxCb.pitch = dst.pitch2;
			pxCb.w = dst.w;
			pxCb.h = dst.h;

			pxCr.format = nsVDPixmap::kPixFormat_Y8;
			pxCr.data = dst.data3;
			pxCr.pitch = dst.pitch3;
			pxCr.w = dst.w;
			pxCr.h = dst.h;

			uint32 colorY = (color >> 8) & 0xff;
			uint32 colorCb = (color >> 0) & 0xff;
			uint32 colorCr = (color >> 16) & 0xff;

			VDPixmapFillRegionAntialiased8x(pxY, region, x, y, colorY);

			switch(dst.format) {
			case nsVDPixmap::kPixFormat_YUV410_Planar:
			case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV410_Planar_709:
			case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 2;
				pxCr.h = pxCb.h = dst.h >> 2;
				x >>= 2;
				y >>= 2;
				VDPixmapFillRegionAntialiased_32x_32x(pxCb, region, x, y, colorCb);
				VDPixmapFillRegionAntialiased_32x_32x(pxCr, region, x, y, colorCr);
				return true;
			case nsVDPixmap::kPixFormat_YUV420_Planar:
			case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV420_Planar_709:
			case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 1;
				pxCr.h = pxCb.h = dst.h >> 1;
				x >>= 1;
				y >>= 1;
				x += 2;
				VDPixmapFillRegionAntialiased_16x_16x(pxCb, region, x, y, colorCb);
				VDPixmapFillRegionAntialiased_16x_16x(pxCr, region, x, y, colorCr);
				return true;
			case nsVDPixmap::kPixFormat_YUV422_Planar:
			case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV422_Planar_709:
			case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 1;
				x >>= 1;
				x += 2;
				VDPixmapFillRegionAntialiased_16x_8x(pxCb, region, x, y, colorCb);
				VDPixmapFillRegionAntialiased_16x_8x(pxCr, region, x, y, colorCr);
				return true;
			case nsVDPixmap::kPixFormat_YUV444_Planar:
			case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV444_Planar_709:
			case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
				VDPixmapFillRegionAntialiased8x(pxCb, region, x, y, colorCb);
				VDPixmapFillRegionAntialiased8x(pxCr, region, x, y, colorCr);
				return true;
			}
		}
	}

	if (dst.format != nsVDPixmap::kPixFormat_XRGB8888 && dst.format != nsVDPixmap::kPixFormat_Y8)
		return false;

	// fast out
	if (region.mSpans.empty())
		return true;

	// check if vertical clipping is required
	const size_t n = region.mSpans.size();
	uint32 start = 0;
	uint32 end = n;

	uint32 spanmin = -x + (-y << 16) + 0x80008000;

	if (region.mSpans.front() < spanmin) {
		// find first span : x2 > spanmin
		start = std::upper_bound(region.mSpans.begin(), region.mSpans.end(), spanmin) - region.mSpans.begin();
		start &= ~1;

		// check for total top clip
		if (start >= n)
			return true;
	}

	uint32 spanlimit = (dst.w*8 - x) + (((dst.h*8 - y) - 1) << 16) + 0x80008000;

	if (region.mSpans.back() > spanlimit) {
		// find last span : x1 < spanlimit
		end = std::lower_bound(region.mSpans.begin(), region.mSpans.end(), spanlimit) - region.mSpans.begin();

		end = (end + 1) & ~1;

		// check for total bottom clip
		if (start >= end)
			return true;
	}

	// allocate A-buffer
	vdfastvector<uint8> abuffer(dst.w, 0);

	// fill region
	const uint32 *pSpan = &region.mSpans[start];
	const uint32 *pEnd  = &region.mSpans[0] + end;
	int lasty = -1;

	sint32 dstw8 = dst.w * 8;
	sint32 dsth8 = dst.h * 8;

	for(; pSpan != pEnd; pSpan += 2) {
		uint32 span0 = pSpan[0];
		uint32 span1 = pSpan[1];

		sint32 py = (span0 >> 16) - 0x8000 + y;

		if ((uint32)py >= (uint32)dsth8)
			continue;

		sint32 px1 = (span0 & 0xffff) - 0x8000 + x;
		sint32 px2 = (span1 & 0xffff) - 0x8000 + x;
		sint32 w = span1-span0;

		if (lasty != py) {
			if (((lasty ^ py) & 0xFFFFFFF8)) {
				if (lasty >= 0) {
					// flush scanline

					if (dst.format == nsVDPixmap::kPixFormat_XRGB8888)
						RenderABuffer32(dst, lasty >> 3, abuffer.data(), dst.w, color);
					else
						RenderABuffer8(dst, lasty >> 3, abuffer.data(), dst.w, color);
				}

				memset(abuffer.data(), 0, abuffer.size());
			}
			lasty = py;
		}

		if (px1 < 0)
			px1 = 0;
		if (px2 > dstw8)
			px2 = dstw8;

		if (px1 >= px2)
			continue;

		uint32 ix1 = px1 >> 3;
		uint32 ix2 = px2 >> 3;
		uint8 *p1 = abuffer.data() + ix1;
		uint8 *p2 = abuffer.data() + ix2;

		if (p1 == p2) {
			p1[0] += (px2 - px1);
		} else {
			if (px1 & 7) {
				p1[0] += 8 - (px1 & 7);
				++p1;
			}

			while(p1 != p2) {
				p1[0] += 8;
				++p1;
			}

			if (px2 & 7)
				p1[0] += px2 & 7;
		}
	}

	if (lasty >= 0) {
		if (dst.format == nsVDPixmap::kPixFormat_XRGB8888)
			RenderABuffer32(dst, lasty >> 3, abuffer.data(), dst.w, color);
		else
			RenderABuffer8(dst, lasty >> 3, abuffer.data(), dst.w, color);
	}

	return true;
}

void VDPixmapCreateRoundRegion(VDPixmapRegion& dst, float r) {
	int ir = VDCeilToInt(r);
	float r2 = r*r;

	dst.mSpans.clear();
	dst.mBounds.set(-ir, 0, ir+1, 0);

	for(int y = -ir; y <= ir; ++y) {
		int dx = VDCeilToInt(sqrtf(r2 - y*y));

		if (dx > 0) {
			dst.mSpans.push_back(0x80008000 + (y << 16) - dx);
			dst.mSpans.push_back(0x80008001 + (y << 16) + dx);
			if (dst.mBounds.top > y)
				dst.mBounds.top = y;
			if (dst.mBounds.bottom < y)
				dst.mBounds.bottom = y;
		}
	}
}

void VDPixmapConvolveRegion(VDPixmapRegion& dst, const VDPixmapRegion& r1, const VDPixmapRegion& r2, int dx1, int dx2, int dy) {
	dst.mSpans.clear();
	dst.mSpans.resize(r1.mSpans.size()+r2.mSpans.size());

	const uint32 *itA	= r1.mSpans.data();
	const uint32 *itAE	= itA + r1.mSpans.size();
	const uint32 *itB	= r2.mSpans.data();
	const uint32 *itBE	= itB + r2.mSpans.size();
	uint32 *dstp0 = dst.mSpans.data();
	uint32 *dstp = dst.mSpans.data();

	uint32 offset1 = (dy<<16) + dx1;
	uint32 offset2 = (dy<<16) + dx2;

	while(itA != itAE && itB != itBE) {
		uint32 x1;
		uint32 x2;

		if (itB[0] + offset1 < itA[0]) {
			// B span is earlier.  Use it.
			x1 = itB[0] + offset1;
			x2 = itB[1] + offset2;
			itB += 2;

			// B spans *can* overlap, due to the widening.
			while(itB != itBE && itB[0]+offset1 <= x2) {
				uint32 bx2 = itB[1] + offset2;
				if (x2 < bx2)
					x2 = bx2;

				itB += 2;
			}

			goto a_start;
		} else {
			// A span is earlier.  Use it.
			x1 = itA[0];
			x2 = itA[1];
			itA += 2;

			// A spans don't overlap, so begin merge loop with B first.
		}

		for(;;) {
			// If we run out of B spans or the B span doesn't overlap,
			// then the next A span can't either (because A spans don't
			// overlap) and we exit.

			if (itB == itBE || itB[0]+offset1 > x2)
				break;

			do {
				uint32 bx2 = itB[1] + offset2;
				if (x2 < bx2)
					x2 = bx2;

				itB += 2;
			} while(itB != itBE && itB[0]+offset1 <= x2);

			// If we run out of A spans or the A span doesn't overlap,
			// then the next B span can't either, because we would have
			// consumed all overlapping B spans in the above loop.
a_start:
			if (itA == itAE || itA[0] > x2)
				break;

			do {
				uint32 ax2 = itA[1];
				if (x2 < ax2)
					x2 = ax2;

				itA += 2;
			} while(itA != itAE && itA[0] <= x2);
		}

		// Flush span.
		dstp[0] = x1;
		dstp[1] = x2;
		dstp += 2;
	}

	// Copy over leftover spans.
	memcpy(dstp, itA, sizeof(uint32)*(itAE - itA));
	dstp += itAE - itA;

	while(itB != itBE) {
		// B span is earlier.  Use it.
		uint32 x1 = itB[0] + offset1;
		uint32 x2 = itB[1] + offset2;
		itB += 2;

		// B spans *can* overlap, due to the widening.
		while(itB != itBE && itB[0]+offset1 <= x2) {
			uint32 bx2 = itB[1] + offset2;
			if (x2 < bx2)
				x2 = bx2;

			itB += 2;
		}

		dstp[0] = x1;
		dstp[1] = x2;
		dstp += 2;
	}

	dst.mSpans.resize(dstp - dst.mSpans.data());
}

void VDPixmapConvolveRegion(VDPixmapRegion& dst, const VDPixmapRegion& r1, const VDPixmapRegion& r2, VDPixmapRegion *tempCache) {
	VDPixmapRegion temp;

	if (tempCache) {
		tempCache->swap(temp);

		temp.clear();
	}

	const uint32 *src1 = r2.mSpans.data();
	const uint32 *src2 = src1 + r2.mSpans.size();

	dst.mSpans.clear();
	while(src1 != src2) {
		uint32 p1 = src1[0];
		uint32 p2 = src1[1];
		src1 += 2;

		temp.mSpans.swap(dst.mSpans);
		VDPixmapConvolveRegion(dst, temp, r1, (p1 & 0xffff) - 0x8000, (p2 & 0xffff) - 0x8000, (p1 >> 16) - 0x8000);
	}

	if (tempCache)
		tempCache->swap(temp);
}
