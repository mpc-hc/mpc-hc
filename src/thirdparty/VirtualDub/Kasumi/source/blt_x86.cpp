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

void VDPixmapInitBlittersReference(VDPixmapBlitterTable& table);

#define DECLARE_PALETTED(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h, const void *pal0);
#define DECLARE_RGB(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
#define DECLARE_RGB_ASM(x, y) extern "C" void vdasm_pixblt_##x##_to_##y(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
#define DECLARE_RGB_ASM_MMX(x, y) extern "C" void vdasm_pixblt_##x##_to_##y##_MMX(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
#define DECLARE_YUV(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);
#define DECLARE_YUV_REV(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)
#define DECLARE_YUV_PLANAR(x, y) extern void VDPixmapBlt_##x##_to_##y##_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);

									DECLARE_RGB_ASM(RGB565,	  XRGB1555);	DECLARE_RGB_ASM_MMX(RGB565,   XRGB1555);
									DECLARE_RGB_ASM(RGB888,   XRGB1555);
									DECLARE_RGB_ASM(XRGB8888, XRGB1555);	DECLARE_RGB_ASM_MMX(XRGB8888, XRGB1555);
									DECLARE_RGB_ASM(XRGB1555, RGB565);		DECLARE_RGB_ASM_MMX(XRGB1555, RGB565);
									DECLARE_RGB_ASM(RGB888,   RGB565);
									DECLARE_RGB_ASM(XRGB8888, RGB565);		DECLARE_RGB_ASM_MMX(XRGB8888, RGB565);
DECLARE_RGB(XRGB1555, RGB888);
DECLARE_RGB(RGB565,   RGB888);
									DECLARE_RGB_ASM(XRGB8888, RGB888);		DECLARE_RGB_ASM_MMX(XRGB8888, RGB888);
									DECLARE_RGB_ASM(XRGB1555, XRGB8888);	DECLARE_RGB_ASM_MMX(XRGB1555, XRGB8888);
									DECLARE_RGB_ASM(RGB565,   XRGB8888);	DECLARE_RGB_ASM_MMX(RGB565,   XRGB8888);
									DECLARE_RGB_ASM(RGB888,   XRGB8888);	DECLARE_RGB_ASM_MMX(RGB888,   XRGB8888);

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

using namespace nsVDPixmap;

void VDPixmapInitBlittersX86(VDPixmapBlitterTable& table) {
	VDPixmapInitBlittersReference(table);

	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB1555_to_RGB565>);
	table.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB1555_to_XRGB8888>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB565_to_XRGB1555>);
	table.AddBlitter(kPixFormat_RGB565,		kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB565_to_XRGB8888>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB888_to_XRGB1555>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB888_to_RGB565>);
	table.AddBlitter(kPixFormat_RGB888,		kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB888_to_XRGB8888>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_XRGB1555>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_RGB565>);
	table.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_RGB888>);
}

tpVDPixBltTable VDGetPixBltTableX86ScalarInternal() {
	static VDPixmapBlitterTable sReferenceTable;

	VDPixmapInitBlittersX86(sReferenceTable);

	return sReferenceTable.mTable;
}

tpVDPixBltTable VDGetPixBltTableX86MMXInternal() {
	static VDPixmapBlitterTable sReferenceTable;

	VDPixmapInitBlittersX86(sReferenceTable);

	sReferenceTable.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB1555_to_RGB565_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_XRGB1555,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB1555_to_XRGB8888_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_RGB565,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB565_to_XRGB1555_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_RGB565,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB565_to_XRGB8888_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_RGB888,	kPixFormat_XRGB8888,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_RGB888_to_XRGB8888_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_XRGB1555,	VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_XRGB1555_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB565,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_RGB565_MMX>);
	sReferenceTable.AddBlitter(kPixFormat_XRGB8888,	kPixFormat_RGB888,		VDPixmapBlitterChunkyAdapter<vdasm_pixblt_XRGB8888_to_RGB888_MMX>);

	return sReferenceTable.mTable;
}

tpVDPixBltTable VDGetPixBltTableX86Scalar() {
	static tpVDPixBltTable spTable = VDGetPixBltTableX86ScalarInternal();

	return spTable;
}

tpVDPixBltTable VDGetPixBltTableX86MMX() {
	static tpVDPixBltTable spTable = VDGetPixBltTableX86MMXInternal();

	return spTable;
}
