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
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "blt_setup.h"

#define DECLARE_PALETTED(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h, const void *pal0)
#define DECLARE_RGB(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)
#define DECLARE_YUV(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)
#define DECLARE_YUV_REV(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)
#define DECLARE_YUV_PLANAR(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h)

DECLARE_RGB(RGB565,	  XRGB1555);
DECLARE_RGB(RGB888,   XRGB1555);
DECLARE_RGB(XRGB8888, XRGB1555);
DECLARE_RGB(XRGB1555, RGB565);
DECLARE_RGB(RGB888,   RGB565);
DECLARE_RGB(XRGB8888, RGB565);
DECLARE_RGB(XRGB1555, RGB888);
DECLARE_RGB(RGB565,   RGB888);
DECLARE_RGB(XRGB8888, RGB888);
DECLARE_RGB(XRGB1555, XRGB8888);
DECLARE_RGB(RGB565,   XRGB8888);
DECLARE_RGB(RGB888,   XRGB8888);

DECLARE_PALETTED(Pal1, Any8);
DECLARE_PALETTED(Pal1, Any16);
DECLARE_PALETTED(Pal1, Any24);
DECLARE_PALETTED(Pal1, Any32);
DECLARE_PALETTED(Pal2, Any8);
DECLARE_PALETTED(Pal2, Any16);
DECLARE_PALETTED(Pal2, Any24);
DECLARE_PALETTED(Pal2, Any32);
DECLARE_PALETTED(Pal4, Any8);
DECLARE_PALETTED(Pal4, Any16);
DECLARE_PALETTED(Pal4, Any24);
DECLARE_PALETTED(Pal4, Any32);
DECLARE_PALETTED(Pal8, Any8);
DECLARE_PALETTED(Pal8, Any16);
DECLARE_PALETTED(Pal8, Any24);
DECLARE_PALETTED(Pal8, Any32);

DECLARE_YUV(XVYU, UYVY);
DECLARE_YUV(XVYU, YUYV);
DECLARE_YUV(Y8, UYVY);
DECLARE_YUV(Y8, YUYV);
DECLARE_YUV(UYVY, Y8);
DECLARE_YUV(YUYV, Y8);
DECLARE_YUV(UYVY, YUYV);
DECLARE_YUV_PLANAR(YUV411, YV12);

DECLARE_YUV(UYVY, XRGB1555);
DECLARE_YUV(UYVY, RGB565);
DECLARE_YUV(UYVY, RGB888);
DECLARE_YUV(UYVY, XRGB8888);
DECLARE_YUV(YUYV, XRGB1555);
DECLARE_YUV(YUYV, RGB565);
DECLARE_YUV(YUYV, RGB888);
DECLARE_YUV(YUYV, XRGB8888);
DECLARE_YUV(Y8, XRGB1555);
DECLARE_YUV(Y8, RGB565);
DECLARE_YUV(Y8, RGB888);
DECLARE_YUV(Y8, XRGB8888);

DECLARE_YUV_REV(XRGB1555, Y8);
DECLARE_YUV_REV(RGB565,   Y8);
DECLARE_YUV_REV(RGB888,   Y8);
DECLARE_YUV_REV(XRGB8888, Y8);

DECLARE_YUV_REV(XRGB1555, XVYU);
DECLARE_YUV_REV(RGB565,   XVYU);
DECLARE_YUV_REV(RGB888,   XVYU);
DECLARE_YUV_REV(XRGB8888, XVYU);

DECLARE_YUV_PLANAR(YV12, XRGB1555);
DECLARE_YUV_PLANAR(YV12, RGB565);
DECLARE_YUV_PLANAR(YV12, RGB888);
DECLARE_YUV_PLANAR(YV12, XRGB8888);

DECLARE_YUV_PLANAR(YUV411, XRGB1555);
DECLARE_YUV_PLANAR(YUV411, RGB565);
DECLARE_YUV_PLANAR(YUV411, RGB888);
DECLARE_YUV_PLANAR(YUV411, XRGB8888);

extern void VDPixmapBlt_YUVPlanar_decode_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);
extern void VDPixmapBlt_YUVPlanar_encode_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);
extern void VDPixmapBlt_YUVPlanar_convert_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);
extern void VDPixmapBlt_UberblitAdapter(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);

using namespace nsVDPixmap;

void VDPixmapInitBlittersReference(VDPixmapBlitterTable& table) {
	// use uberblit as the baseline
	VDPixmapFormatSubset uberblitSrcFormats;
	VDPixmapFormatSubset uberblitDstFormats;

	uberblitSrcFormats =
		kPixFormat_Pal1,
		kPixFormat_Pal2,
		kPixFormat_Pal4,
		kPixFormat_Pal8,
		kPixFormat_XRGB1555,
		kPixFormat_RGB565,
		kPixFormat_RGB888,
		kPixFormat_XRGB8888,
		kPixFormat_Y8,
		kPixFormat_YUV422_UYVY,
		kPixFormat_YUV422_YUYV,
		kPixFormat_YUV444_XVYU,
		kPixFormat_YUV444_Planar,
		kPixFormat_YUV422_Planar,
		kPixFormat_YUV422_Planar_16F,
		kPixFormat_YUV420_Planar,
		kPixFormat_YUV411_Planar,
		kPixFormat_YUV410_Planar,
		kPixFormat_YUV422_Planar_Centered,
		kPixFormat_YUV420_Planar_Centered,
		kPixFormat_YUV422_V210,
		kPixFormat_YUV422_UYVY_709,
		kPixFormat_YUV420_NV12,
		kPixFormat_Y8_FR,
		kPixFormat_YUV422_YUYV_709,
		kPixFormat_YUV444_Planar_709,
		kPixFormat_YUV422_Planar_709,
		kPixFormat_YUV420_Planar_709,
		kPixFormat_YUV411_Planar_709,
		kPixFormat_YUV410_Planar_709,
		kPixFormat_YUV422_UYVY_FR,
		kPixFormat_YUV422_YUYV_FR,
		kPixFormat_YUV444_Planar_FR,
		kPixFormat_YUV422_Planar_FR,
		kPixFormat_YUV420_Planar_FR,
		kPixFormat_YUV411_Planar_FR,
		kPixFormat_YUV410_Planar_FR,
		kPixFormat_YUV422_UYVY_709_FR,
		kPixFormat_YUV422_YUYV_709_FR,
		kPixFormat_YUV444_Planar_709_FR,
		kPixFormat_YUV422_Planar_709_FR,
		kPixFormat_YUV420_Planar_709_FR,
		kPixFormat_YUV411_Planar_709_FR,
		kPixFormat_YUV410_Planar_709_FR,
		kPixFormat_YUV420i_Planar,
		kPixFormat_YUV420i_Planar_FR,
		kPixFormat_YUV420i_Planar_709,
		kPixFormat_YUV420i_Planar_709_FR,
		kPixFormat_YUV420it_Planar,
		kPixFormat_YUV420it_Planar_FR,
		kPixFormat_YUV420it_Planar_709,
		kPixFormat_YUV420it_Planar_709_FR,
		kPixFormat_YUV420ib_Planar,
		kPixFormat_YUV420ib_Planar_FR,
		kPixFormat_YUV420ib_Planar_709,
		kPixFormat_YUV420ib_Planar_709_FR;

	uberblitDstFormats =
		kPixFormat_XRGB1555,
		kPixFormat_RGB565,
		kPixFormat_RGB888,
		kPixFormat_XRGB8888,
		kPixFormat_Y8,
		kPixFormat_YUV422_UYVY,
		kPixFormat_YUV422_YUYV,
		kPixFormat_YUV444_XVYU,
		kPixFormat_YUV444_Planar,
		kPixFormat_YUV422_Planar,
		kPixFormat_YUV422_Planar_16F,
		kPixFormat_YUV420_Planar,
		kPixFormat_YUV411_Planar,
		kPixFormat_YUV410_Planar,
		kPixFormat_YUV422_Planar_Centered,
		kPixFormat_YUV420_Planar_Centered,
		kPixFormat_YUV422_V210,
		kPixFormat_YUV422_UYVY_709,
		kPixFormat_YUV420_NV12,
		kPixFormat_Y8_FR,
		kPixFormat_YUV422_YUYV_709,
		kPixFormat_YUV444_Planar_709,
		kPixFormat_YUV422_Planar_709,
		kPixFormat_YUV420_Planar_709,
		kPixFormat_YUV411_Planar_709,
		kPixFormat_YUV410_Planar_709,
		kPixFormat_YUV422_UYVY_FR,
		kPixFormat_YUV422_YUYV_FR,
		kPixFormat_YUV444_Planar_FR,
		kPixFormat_YUV422_Planar_FR,
		kPixFormat_YUV420_Planar_FR,
		kPixFormat_YUV411_Planar_FR,
		kPixFormat_YUV410_Planar_FR,
		kPixFormat_YUV422_UYVY_709_FR,
		kPixFormat_YUV422_YUYV_709_FR,
		kPixFormat_YUV444_Planar_709_FR,
		kPixFormat_YUV422_Planar_709_FR,
		kPixFormat_YUV420_Planar_709_FR,
		kPixFormat_YUV411_Planar_709_FR,
		kPixFormat_YUV410_Planar_709_FR,
		kPixFormat_YUV420i_Planar,
		kPixFormat_YUV420i_Planar_FR,
		kPixFormat_YUV420i_Planar_709,
		kPixFormat_YUV420i_Planar_709_FR,
		kPixFormat_YUV420it_Planar,
		kPixFormat_YUV420it_Planar_FR,
		kPixFormat_YUV420it_Planar_709,
		kPixFormat_YUV420it_Planar_709_FR,
		kPixFormat_YUV420ib_Planar,
		kPixFormat_YUV420ib_Planar_FR,
		kPixFormat_YUV420ib_Planar_709,
		kPixFormat_YUV420ib_Planar_709_FR;

	table.AddBlitter(uberblitSrcFormats, uberblitDstFormats, VDPixmapBlt_UberblitAdapter);

	// standard formats

	table.AddBlitter(kPixFormat_Pal1,	kPixFormat_Y8,			VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal1_to_Any8_reference>);
	table.AddBlitter(kPixFormat_Pal1,	kPixFormat_XRGB1555,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal1_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal1,	kPixFormat_RGB565,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal1_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal1,	kPixFormat_RGB888,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal1_to_Any24_reference>);
	table.AddBlitter(kPixFormat_Pal1,	kPixFormat_XRGB8888,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal1_to_Any32_reference>);
	table.AddBlitter(kPixFormat_Pal2,	kPixFormat_Y8,			VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal2_to_Any8_reference>);
	table.AddBlitter(kPixFormat_Pal2,	kPixFormat_XRGB1555,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal2_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal2,	kPixFormat_RGB565,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal2_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal2,	kPixFormat_RGB888,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal2_to_Any24_reference>);
	table.AddBlitter(kPixFormat_Pal2,	kPixFormat_XRGB8888,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal2_to_Any32_reference>);
	table.AddBlitter(kPixFormat_Pal4,	kPixFormat_Y8,			VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal4_to_Any8_reference>);
	table.AddBlitter(kPixFormat_Pal4,	kPixFormat_XRGB1555,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal4_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal4,	kPixFormat_RGB565,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal4_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal4,	kPixFormat_RGB888,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal4_to_Any24_reference>);
	table.AddBlitter(kPixFormat_Pal4,	kPixFormat_XRGB8888,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal4_to_Any32_reference>);
	table.AddBlitter(kPixFormat_Pal8,	kPixFormat_Y8,			VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal8_to_Any8_reference>);
	table.AddBlitter(kPixFormat_Pal8,	kPixFormat_XRGB1555,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal8_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal8,	kPixFormat_RGB565,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal8_to_Any16_reference>);
	table.AddBlitter(kPixFormat_Pal8,	kPixFormat_RGB888,		VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal8_to_Any24_reference>);
	table.AddBlitter(kPixFormat_Pal8,	kPixFormat_XRGB8888,	VDPixmapBlitterPalettedAdapter<VDPixmapBlt_Pal8_to_Any32_reference>);

	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB1555_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB1555_to_RGB888_reference>);
	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB1555_to_XRGB8888_reference>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB565_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB565_to_RGB888_reference>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB565_to_XRGB8888_reference>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB888_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB888_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB888_to_XRGB8888_reference>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB8888_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB8888_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB8888_to_RGB888_reference>);

	table.AddBlitter(kPixFormat_YUV444_XVYU,	kPixFormat_YUV422_UYVY,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XVYU_to_UYVY_reference>);
	table.AddBlitter(kPixFormat_YUV444_XVYU,	kPixFormat_YUV422_YUYV,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XVYU_to_YUYV_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_YUV422_UYVY,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_UYVY_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_YUV422_YUYV,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_YUYV_reference>);
	table.AddBlitter(kPixFormat_YUV422_UYVY,	kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_Y8_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV,	kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_YUYV_to_Y8_reference>);

	table.AddBlitter(kPixFormat_YUV422_UYVY,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_YUV422_UYVY,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_YUV422_UYVY,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_RGB888_reference>);
	table.AddBlitter(kPixFormat_YUV422_UYVY,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_XRGB8888_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_YUYV_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_YUYV_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_YUYV_to_RGB888_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_YUYV_to_XRGB8888_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_XRGB1555_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_RGB565_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_RGB888_reference>);
	table.AddBlitter(kPixFormat_Y8,				kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<VDPixmapBlt_Y8_to_XRGB8888_reference>);

	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_YUV444_XVYU, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB1555_to_XVYU_reference>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_YUV444_XVYU, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB565_to_XVYU_reference>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_YUV444_XVYU, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB888_to_XVYU_reference>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_YUV444_XVYU, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB8888_to_XVYU_reference>);

	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB1555_to_Y8_reference>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB565_to_Y8_reference>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_RGB888_to_Y8_reference>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_Y8,			VDPixmapBlitterChunkyAdapter<VDPixmapBlt_XRGB8888_to_Y8_reference>);

	table.AddBlitter(kPixFormat_YUV411_Planar, kPixFormat_YUV420_Planar, VDPixmapBlt_YUV411_to_YV12_reference);

	table.AddBlitter(kPixFormat_YUV422_UYVY, kPixFormat_YUV422_YUYV, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_YUYV_reference>);
	table.AddBlitter(kPixFormat_YUV422_YUYV, kPixFormat_YUV422_UYVY, VDPixmapBlitterChunkyAdapter<VDPixmapBlt_UYVY_to_YUYV_reference>);		// not an error -- same routine

	//////////////////////////////////////////////////////////

	VDPixmapFormatSubset srcFormats;
	VDPixmapFormatSubset dstFormats;

	srcFormats = kPixFormat_YUV444_Planar,
				kPixFormat_YUV422_Planar,
				kPixFormat_YUV420_Planar,
				kPixFormat_YUV411_Planar,
				kPixFormat_YUV410_Planar,
				kPixFormat_YUV422_Planar_Centered,
				kPixFormat_YUV420_Planar_Centered;

	dstFormats = kPixFormat_XRGB1555, kPixFormat_RGB565, kPixFormat_RGB888, kPixFormat_XRGB8888, kPixFormat_YUV422_UYVY, kPixFormat_YUV422_YUYV;

	table.AddBlitter(srcFormats, dstFormats, VDPixmapBlt_YUVPlanar_decode_reference);

	//////////////////////////////////////////////////////////

	dstFormats = kPixFormat_YUV444_Planar, kPixFormat_YUV422_Planar, kPixFormat_YUV420_Planar, kPixFormat_YUV411_Planar, kPixFormat_YUV410_Planar, kPixFormat_YUV422_Planar_Centered, kPixFormat_YUV420_Planar_Centered;
	srcFormats = kPixFormat_XRGB1555, kPixFormat_RGB565, kPixFormat_RGB888, kPixFormat_XRGB8888, kPixFormat_YUV422_UYVY, kPixFormat_YUV422_YUYV;

	table.AddBlitter(srcFormats, dstFormats, VDPixmapBlt_YUVPlanar_encode_reference);

	//////////////////////////////////////////////////////////

	srcFormats = kPixFormat_YUV444_Planar, kPixFormat_YUV422_Planar, kPixFormat_YUV420_Planar, kPixFormat_YUV411_Planar, kPixFormat_YUV410_Planar, kPixFormat_Y8, kPixFormat_YUV422_Planar_Centered, kPixFormat_YUV420_Planar_Centered;
	dstFormats = kPixFormat_YUV444_Planar, kPixFormat_YUV422_Planar, kPixFormat_YUV420_Planar, kPixFormat_YUV411_Planar, kPixFormat_YUV410_Planar, kPixFormat_Y8, kPixFormat_YUV422_Planar_Centered, kPixFormat_YUV420_Planar_Centered;

	table.AddBlitter(srcFormats, dstFormats, VDPixmapBlt_YUVPlanar_convert_reference);
}

tpVDPixBltTable VDGetPixBltTableReferenceInternal() {
	static VDPixmapBlitterTable sReferenceTable;

	VDPixmapInitBlittersReference(sReferenceTable);

	return sReferenceTable.mTable;
}

tpVDPixBltTable VDGetPixBltTableReference() {
	static tpVDPixBltTable spTable = VDGetPixBltTableReferenceInternal();

	return spTable;
}
