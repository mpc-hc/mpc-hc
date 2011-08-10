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
#include <vd2/system/cpuaccel.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/vdstl.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "blt_spanutils.h"

#ifdef _M_IX86
	#include "blt_spanutils_x86.h"
#endif

using namespace nsVDPixmapSpanUtils;

namespace {
	// From Jim Blinn's "Dirty Pixels":
	//
	// Y  = .299R + .587G + .114B
	// Cr = 0.713(R-Y)
	// Cb = 0.564(B-Y)
	//
	// IY  = 219Y  + 16  = ((yt = 1052IR + 2065IG + 401IB) + 67584) >> 12
	// ICr = 224Cr + 128 = (yt*2987 - 10507932IR + 2155872256) >> 24
	// ICb = 224Cb + 128 = (yt*2363 - 8312025IB + 2155872256) >> 24

	void ConvertRGB32ToXVYU32(uint32 *dst, const uint8 *src, sint32 count) {
		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			const sint32 yt = 1052*r + 2065*g + 401*b;
			const sint32 y  = (yt + 67584) >> 4;							// <<8 alignment shift
			const sint32 cr = (10507932*r - yt*2987 + 2155872256U) >> 8;	// <<16 alignment shift
			const sint32 cb = ( 8312025*b - yt*2363 + 2155872256U) >> 24;

			*dst++ = (y&0xff00) + cb + (cr&0xff0000);		// VYU order
			src += 4;
		} while(--count);
	}

	void ConvertRGB24ToXVYU32(uint32 *dst, const uint8 *src, sint32 count) {
		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			const sint32 yt = 1052*r + 2065*g + 401*b;
			const sint32 y  = (yt + 67584) >> 4;							// <<8 alignment shift
			const sint32 cr = (10507932*r - yt*2987 + 2155872256U) >> 8;	// <<16 alignment shift
			const sint32 cb = ( 8312025*b - yt*2363 + 2155872256U) >> 24;

			*dst++ = (y&0xff00) + cb + (cr&0xff0000);		// VYU order
			src += 3;
		} while(--count);
	}

	void ConvertRGB16ToXVYU32(uint32 *dst, const uint16 *src, sint32 count) {
		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0xf800) >> 11;
			const sint32 g  = (px & 0x07e0) >> 5;
			const sint32 b  = (px & 0x001f);
			const sint32 yt = 8652*r + 8358*g + 3299*b;
			const sint32 y  = (yt + 67584) >> 4;							// <<8 alignment shift
			const sint32 cr = (86436217*r - yt*2987 + 2155872256U) >> 8;
			const sint32 cb = (68373108*b - yt*2363 + 2155872256U) >> 24;	// <<16 alignment shift

			*dst++ = (y&0xff00) + cb + (cr&0xff0000);		// VYU order
		} while(--count);
	}

	void ConvertRGB15ToXVYU32(uint32 *dst, const uint16 *src, sint32 count) {
		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0x7c00) >> 10;
			const sint32 g  = (px & 0x03e0) >> 5;
			const sint32 b  = (px & 0x001f);
			const sint32 yt = 8652*r + 16986*g + 3299*b;
			const sint32 y  = (yt + 67584) >> 4;							// <<8 alignment shift
			const sint32 cr = (86436217*r - yt*2987 + 2155872256U) >> 8;	// <<16 alignment shift
			const sint32 cb = (68373108*b - yt*2363 + 2155872256U) >> 24;

			*dst++ = (y&0xff00) + cb + (cr&0xff0000);		// VYU order
		} while(--count);
	}

	void ConvertRGB32ToY8(uint8 *dst, const uint8 *src, sint32 count) {
		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			*dst++ = (uint8)((1052*r + 2065*g + 401*b + 67584) >> 12);
			src += 4;
		} while(--count);
	}

	void ConvertRGB24ToY8(uint8 *dst, const uint8 *src, sint32 count) {
		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			*dst++ = (uint8)((1052*r + 2065*g + 401*b + 67584) >> 12);
			src += 3;
		} while(--count);
	}

	void ConvertRGB16ToY8(uint8 *dst, const uint16 *src, sint32 count) {
		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0xf800) >> 11;
			const sint32 g  = (px & 0x07e0) >> 5;
			const sint32 b  = (px & 0x001f);
			*dst++ = (uint8)((8652*r + 8358*g + 3299*b + 67584) >> 12);
		} while(--count);
	}

	void ConvertRGB15ToY8(uint8 *dst, const uint16 *src, sint32 count) {
		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0x7c00) >> 10;
			const sint32 g  = (px & 0x03e0) >> 5;
			const sint32 b  = (px & 0x001f);
			*dst++ = (uint8)((8652*r + 16986*g + 3299*b + 67584) >> 12);
		} while(--count);
	}
}

#define DECLARE_YUV_REV(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)

DECLARE_YUV_REV(XRGB1555, XVYU) {
	do {
		ConvertRGB15ToXVYU32((uint32 *)dst0, (const uint16 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(RGB565, XVYU) {
	do {
		ConvertRGB16ToXVYU32((uint32 *)dst0, (const uint16 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(RGB888, XVYU) {
	do {
		ConvertRGB24ToXVYU32((uint32 *)dst0, (const uint8 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(XRGB8888, XVYU) {
	do {
		ConvertRGB32ToXVYU32((uint32 *)dst0, (const uint8 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(XRGB1555, Y8) {
	do {
		ConvertRGB15ToY8((uint8 *)dst0, (const uint16 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(RGB565, Y8) {
	do {
		ConvertRGB16ToY8((uint8 *)dst0, (const uint16 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(RGB888, Y8) {
	do {
		ConvertRGB24ToY8((uint8 *)dst0, (const uint8 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}

DECLARE_YUV_REV(XRGB8888, Y8) {
	do {
		ConvertRGB32ToY8((uint8 *)dst0, (const uint8 *)src0, w);

		vdptrstep(dst0, dstpitch);
		vdptrstep(src0, srcpitch);
	} while(--h);
}





namespace {
	void ConvertRGB32ToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint8 *src = (const uint8 *)src0;

		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			const sint32 yt = 1052*r + 2065*g + 401*b;
			*ydst++  = (yt + 67584) >> 12;
			*crdst++ = (10507932*r - yt*2987 + 2155872256U) >> 24;
			*cbdst++ = ( 8312025*b - yt*2363 + 2155872256U) >> 24;
			src += 4;
		} while(--count);
	}

	void ConvertRGB24ToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint8 *src = (const uint8 *)src0;

		do {
			const sint32 r  = src[2];
			const sint32 g  = src[1];
			const sint32 b  = src[0];
			const sint32 yt = 1052*r + 2065*g + 401*b;
			*ydst++  = (yt + 67584) >> 12;
			*crdst++ = (10507932*r - yt*2987 + 2155872256U) >> 24;
			*cbdst++ = ( 8312025*b - yt*2363 + 2155872256U) >> 24;
			src += 3;
		} while(--count);
	}

	void ConvertRGB16ToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint16 *src = (const uint16 *)src0;

		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0xf800) >> 11;
			const sint32 g  = (px & 0x07e0) >> 5;
			const sint32 b  = (px & 0x001f);
			const sint32 yt = 8652*r + 8358*g + 3299*b;
			*ydst++  = (yt + 67584) >> 12;
			*crdst++ = (86436217*r - yt*2987 + 2155872256U) >> 24;
			*cbdst++ = (68373108*b - yt*2363 + 2155872256U) >> 24;
		} while(--count);
	}

	void ConvertRGB15ToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint16 *src = (const uint16 *)src0;

		do {
			const sint16 px = *src++;
			const sint32 r  = (px & 0x7c00) >> 10;
			const sint32 g  = (px & 0x03e0) >> 5;
			const sint32 b  = (px & 0x001f);
			const sint32 yt = 8652*r + 16986*g + 3299*b;
			*ydst++  = (yt + 67584) >> 12;
			*crdst++ = (86436217*r - yt*2987 + 2155872256U) >> 24;
			*cbdst++ = (68373108*b - yt*2363 + 2155872256U) >> 24;
		} while(--count);
	}

	void ConvertUYVYToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint8 *src = (const uint8 *)src0;

		do {
			*cbdst++ = src[0];
			*ydst++ = src[1];
			*crdst++ = src[2];
			if (!--count)
				break;
			*ydst++ = src[3];
			src += 4;
		} while(--count);
	}

	void ConvertYUYVToYUVPlanar(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src0, sint32 count) {
		const uint8 *src = (const uint8 *)src0;

		do {
			*cbdst++ = src[1];
			*ydst++ = src[0];
			*crdst++ = src[3];
			if (!--count)
				break;
			*ydst++ = src[2];
			src += 4;
		} while(--count);
	}
}

void VDPixmapBlt_YUVPlanar_encode_reference(const VDPixmap& dstbm, const VDPixmap& srcbm, vdpixsize w, vdpixsize h) {
	void (*cfunc)(uint8 *ydst, uint8 *cbdst, uint8 *crdst, const void *src, sint32 w) = NULL;
	void (*hfunc)(uint8 *dst, const uint8 *src, sint32 w) = NULL;
	void (*vfunc)(uint8 *dst, const uint8 *const *sources, sint32 w, uint8 phase) = NULL;

	bool halfchroma = false;

	switch(srcbm.format) {
	case nsVDPixmap::kPixFormat_XRGB1555:
		cfunc = ConvertRGB15ToYUVPlanar;
		break;
	case nsVDPixmap::kPixFormat_RGB565:
		cfunc = ConvertRGB16ToYUVPlanar;
		break;
	case nsVDPixmap::kPixFormat_RGB888:
		cfunc = ConvertRGB24ToYUVPlanar;
		break;
	case nsVDPixmap::kPixFormat_XRGB8888:
		cfunc = ConvertRGB32ToYUVPlanar;
		break;
	case nsVDPixmap::kPixFormat_YUV422_UYVY:
		cfunc = ConvertUYVYToYUVPlanar;
		halfchroma = true;
		break;
	case nsVDPixmap::kPixFormat_YUV422_YUYV:
		cfunc = ConvertYUYVToYUVPlanar;
		halfchroma = true;
		break;
	default:
		VDNEVERHERE;
		return;
	}

	vdpixsize w2 = w;
	vdpixsize h2 = h;
	int winstep = 1;
	int winsize = 1;
	int winposnext = 0;
	vdpixsize chroma_srcw = w;

	switch(dstbm.format) {

	case nsVDPixmap::kPixFormat_YUV444_Planar:
		if (halfchroma)
			hfunc = horiz_expand2x_coaligned;
		break;

	case nsVDPixmap::kPixFormat_YUV422_Planar:
		if (halfchroma)
			chroma_srcw = (chroma_srcw + 1) >> 1;
		else
			hfunc = horiz_compress2x_coaligned;

		w2 = (w2+1) >> 1;
		break;

	case nsVDPixmap::kPixFormat_YUV422_Planar_Centered:
		if (halfchroma) {
			chroma_srcw = (chroma_srcw + 1) >> 1;
			hfunc = horiz_realign_to_centered;
		} else
			hfunc = horiz_compress2x_centered;

		w2 = (w2+1) >> 1;
		break;

	case nsVDPixmap::kPixFormat_YUV420_Planar:
		if (halfchroma)
			chroma_srcw = (chroma_srcw + 1) >> 1;
		else
			hfunc = horiz_compress2x_coaligned;

		vfunc = vert_compress2x_centered;
		winstep = 2;
		winposnext = 2;
		winsize = 4;
		h2 = (h+1) >> 1;
		w2 = (w2+1) >> 1;
		break;

	case nsVDPixmap::kPixFormat_YUV420_Planar_Centered:
		if (halfchroma) {
			chroma_srcw = (chroma_srcw + 1) >> 1;
			hfunc = horiz_realign_to_centered;
		} else
			hfunc = horiz_compress2x_centered;

		vfunc = vert_compress2x_centered;
		winstep = 2;
		winposnext = 2;
		winsize = 4;
		h2 = (h+1) >> 1;
		w2 = (w2+1) >> 1;
		break;

	case nsVDPixmap::kPixFormat_YUV411_Planar:
		if (halfchroma) {
			chroma_srcw = (chroma_srcw + 1) >> 1;
			hfunc = horiz_compress2x_coaligned;
		} else
			hfunc = horiz_compress4x_coaligned;
		w2 = (w2+1) >> 2;
		break;

	case nsVDPixmap::kPixFormat_YUV410_Planar:
		if (halfchroma) {
			chroma_srcw = (chroma_srcw + 1) >> 1;
			hfunc = horiz_compress2x_coaligned;
		} else
			hfunc = horiz_compress4x_coaligned;
		vfunc = vert_compress4x_centered;
		winsize = 8;
		winposnext = 5;
		winstep = 4;
		h2 = (h+3) >> 2;
		w2 = (w2+3) >> 2;
		break;
	}

#ifdef _M_IX86
	uint32 cpuflags = CPUGetEnabledExtensions();

	if (cpuflags & CPUF_SUPPORTS_INTEGER_SSE) {
		if (hfunc == horiz_expand2x_coaligned)
			hfunc = horiz_expand2x_coaligned_ISSE;
	}
#endif

	const uint8 *src = (const uint8 *)srcbm.data;
	const ptrdiff_t srcpitch = srcbm.pitch;

	uint8 *ydst = (uint8 *)dstbm.data;
	uint8 *cbdst = (uint8 *)dstbm.data2;
	uint8 *crdst = (uint8 *)dstbm.data3;
	const ptrdiff_t ydstpitch = dstbm.pitch;
	const ptrdiff_t cbdstpitch = dstbm.pitch2;
	const ptrdiff_t crdstpitch = dstbm.pitch3;

	if (!vfunc) {
		if (hfunc) {
			uint32 tmpsize = (w + 15) & ~15;

			vdblock<uint8> tmp(tmpsize * 2);
			uint8 *const cbtmp = tmp.data();
			uint8 *const crtmp = cbtmp + tmpsize;

			do {
				cfunc(ydst, cbtmp, crtmp, src, w);
				src += srcpitch;
				ydst += ydstpitch;
				hfunc(cbdst, cbtmp, chroma_srcw);
				hfunc(crdst, crtmp, chroma_srcw);
				cbdst += cbdstpitch;
				crdst += crdstpitch;
			} while(--h);
		} else if (dstbm.format == nsVDPixmap::kPixFormat_Y8) {
			// wasteful, but oh well
			uint32 tmpsize = (w2+15)&~15;
			vdblock<uint8> tmp(tmpsize);

			cbdst = tmp.data();
			crdst = cbdst + tmpsize;

			do {
				cfunc(ydst, cbdst, crdst, src, w);
				src += srcpitch;
				ydst += ydstpitch;
			} while(--h2);
		} else {
			do {
				cfunc(ydst, cbdst, crdst, src, w);
				src += srcpitch;
				ydst += ydstpitch;
				cbdst += cbdstpitch;
				crdst += crdstpitch;
			} while(--h2);
		}
	} else {
		const uint32 tmpsize = w2;

		vdblock<uint8>		tmpbuf(tmpsize * (winsize + 1) * 2 + 2 * w);

		uint8 *cbwindow[16];
		uint8 *crwindow[16];

		uint8 *p = tmpbuf.data();
		for(int i=0; i<winsize; ++i) {
			cbwindow[i] = cbwindow[winsize+i] = p;
			p += tmpsize;
			crwindow[i] = crwindow[winsize+i] = p;
			p += tmpsize;
		}

		uint8 *cbtmp = p;
		uint8 *crtmp = p + w;

		int winoffset;
		int winpos = winposnext - winsize;
		bool firstline = true;

		do {
			while(winpos < winposnext) {
				winoffset = ++winpos & (winsize - 1);

				bool valid = (unsigned)(winpos-1) < (unsigned)(h-1);		// -1 because we generate line 0 as the first window line
				if (valid || firstline) {
					if (hfunc) {
						cfunc(ydst, cbtmp, crtmp, src, w);
						hfunc(cbwindow[winoffset + winsize - 1], cbtmp, chroma_srcw);
						hfunc(crwindow[winoffset + winsize - 1], crtmp, chroma_srcw);
					} else {
						cfunc(ydst, cbwindow[winoffset + winsize - 1], crwindow[winoffset + winsize - 1], src, w);
					}
					src += srcpitch;
					ydst += ydstpitch;
					firstline = false;
				} else {
					// dupe last generated line -- could be done by pointer swabbing, but I'm lazy
					memcpy(cbwindow[winoffset + winsize - 1], cbwindow[winoffset + winsize - 2], w2);
					memcpy(crwindow[winoffset + winsize - 1], crwindow[winoffset + winsize - 2], w2);
				}
			}
			winposnext += winstep;

			vfunc(cbdst, cbwindow + winoffset, w2, 0);
			vfunc(crdst, crwindow + winoffset, w2, 0);
			cbdst += cbdstpitch;
			crdst += crdstpitch;
		} while(--h2);
	}

#ifdef _M_IX86
	if (cpuflags & CPUF_SUPPORTS_MMX) {
		__asm emms
	}
#endif
}
