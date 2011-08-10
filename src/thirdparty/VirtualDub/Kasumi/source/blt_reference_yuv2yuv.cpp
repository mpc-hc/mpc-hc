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
#include <vd2/system/vdtypes.h>
#include <vd2/system/memory.h>
#include <vd2/system/vdstl.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>

#include "bitutils.h"
#include "blt_spanutils.h"

#define DECLARE_YUV(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)
#define DECLARE_YUV_PLANAR(x, y) void VDPixmapBlt_##x##_to_##y##_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h)

using namespace nsVDPixmapBitUtils;
using namespace nsVDPixmapSpanUtils;

DECLARE_YUV(XVYU, UYVY) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;

	srcpitch -= (w&~1)*4;
	dstpitch -= (w&~1)*2;

	do {
		vdpixsize wt = w;

		wt = -wt;

		if (++wt) {
			uint32 a, b, c;

			a = src[0];
			b = src[1];
			*dst++ = (avg_8888_121(a, a, b) & 0xff00ff) + (a & 0xff00) + ((b & 0xff00)<<16);
			src += 2;

			if ((wt+=2) < 0) {
				do {
					a = src[-1];
					b = src[0];
					c = src[1];

					*dst++ = (avg_8888_121(a, b, c) & 0xff00ff) + (b & 0xff00) + ((c & 0xff00)<<16);
					src += 2;
				} while((wt+=2) < 0);
			}
		}

		if (!(wt&1))
			*dst = *src;

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(XVYU, YUYV) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;

	srcpitch -= (w&~1)*4;
	dstpitch -= (w&~1)*2;

	do {
		vdpixsize wt = w;

		wt = -wt;

		if (++wt) {
			uint32 a, b, c;

			a = src[0];
			b = src[1];
			*dst++ = ((avg_8888_121(a, a, b) & 0xff00ff)<<8) + ((a & 0xff00)>>8) + ((b & 0xff00)<<8);
			src += 2;

			if ((wt+=2)<0) {
				do {
					a = src[-1];
					b = src[0];
					c = src[1];

					*dst++ = ((avg_8888_121(a, b, c) & 0xff00ff)<<8) + ((b & 0xff00)>>8) + ((c & 0xff00)<<8);
					src += 2;
				} while((wt+=2) < 0);
			}
		}

		if (!(wt&1)) {
			uint32 v = *src;
			*dst = ((v&0xff00ff)<<8) + ((v&0xff00ff00)>>8);
		}

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(UYVY, YUYV) {			// also YUYV->UYVY
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;

	w = (w+1) >> 1;

	dstpitch -= 4*w;
	srcpitch -= 4*w;

	do {
		vdpixsize w2 = w;

		do {
			const uint32 p = *src++;

			*dst++ = ((p & 0xff00ff00)>>8) + ((p & 0x00ff00ff)<<8);
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(UYVY, Y8) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= w;
	srcpitch -= 2*w;

	do {
		vdpixsize w2 = w;

		do {
			*dst++ = src[1];
			src += 2;
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(YUYV, Y8) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= w;
	srcpitch -= 2*w;

	do {
		vdpixsize w2 = w;

		do {
			*dst++ = src[0];
			src += 2;
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, UYVY) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 2*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			dst[0] = 0x80;
			dst[1] = *src++;
			dst += 2;
		} while(--w2);

		if (w & 1) {
			dst[0] = 0x80;
			dst[1] = dst[-1];
		}

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, YUYV) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 2*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			dst[0] = *src++;
			dst[1] = 0x80;
			dst += 2;
		} while(--w2);

		if (w & 1) {
			dst[0] = dst[-1];
			dst[1] = 0x80;
		}

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV_PLANAR(YUV411, YV12) {
	VDMemcpyRect(dst.data, dst.pitch, src.data, src.pitch, w, h);

	vdblock<uint8> tmprow(w);	
	const uint8 *srcp = (const uint8 *)src.data2;
	ptrdiff_t srcpitch = src.pitch2;
	uint8 *dstp = (uint8 *)dst.data2;
	ptrdiff_t dstpitch = dst.pitch2;
	const uint8 *src1, *src2;

	vdpixsize h2;
	for(h2 = h; h2 > 0; h2 -= 2) {
		src1 = srcp;
		vdptrstep(srcp, srcpitch);
		if (h2 > 1)
			src2 = srcp;
		else
			src2 = src1;
		vdptrstep(srcp, srcpitch);

		const uint8 *sources[2] = {src1, src2};

		vert_compress2x_centered_fast(tmprow.data(), sources, w, 0);
		horiz_expand2x_coaligned(dstp, tmprow.data(), w);

		vdptrstep(dstp, dstpitch);
	}

	srcp = (const uint8 *)src.data3;
	srcpitch = src.pitch3;
	dstp = (uint8 *)dst.data3;
	dstpitch = dst.pitch3;
	for(h2 = h; h2 > 0; h2 -= 2) {
		src1 = srcp;
		vdptrstep(srcp, srcpitch);
		if (h2 > 1)
			src2 = srcp;
		else
			src2 = src1;
		vdptrstep(srcp, srcpitch);

		const uint8 *sources[2] = {src1, src2};
		vert_compress2x_centered_fast(tmprow.data(), sources, w, 0);
		horiz_expand2x_coaligned(dstp, tmprow.data(), w);

		vdptrstep(dstp, dstpitch);
	}
}
