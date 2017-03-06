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
#include <vd2/system/error.h>
#include <vd2/system/vdtypes.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/system/memory.h>

extern VDPixmapFormatInfo const g_vdPixmapFormats[] = {
									// name         qchnk qw qh qwb qhb  qs ab aw ah as   ps
	/* Null */						{ "null",		false, 1, 1,  0,  0,  0, 0, 0, 0, 0,   0 },
	/* Pal1 */						{ "Pal1",		 true, 8, 1,  3,  0,  1, 0, 0, 0, 0,   2 },
	/* Pal2 */						{ "Pal2",		 true, 4, 1,  2,  0,  1, 0, 0, 0, 0,   4 },
	/* Pal4 */						{ "Pal4",		 true, 2, 1,  1,  0,  1, 0, 0, 0, 0,  16 },
	/* Pal8 */						{ "Pal8",		false, 1, 1,  0,  0,  1, 0, 0, 0, 0, 256 },
	/* RGB16_555 */					{ "XRGB1555",	false, 1, 1,  0,  0,  2, 0, 0, 0, 0,   0 },
	/* RGB16_565 */					{ "RGB565",		false, 1, 1,  0,  0,  2, 0, 0, 0, 0,   0 },
	/* RGB24 */						{ "RGB888",		false, 1, 1,  0,  0,  3, 0, 0, 0, 0,   0 },
	/* RGB32 */						{ "XRGB8888",	false, 1, 1,  0,  0,  4, 0, 0, 0, 0,   0 },
	/* Y8 */						{ "Y8",			false, 1, 1,  0,  0,  1, 0, 0, 0, 0,   0 },
	/* YUV422_UYVY */				{ "UYVY",		 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV422_YUYV */				{ "YUYV",		 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV444_XVYU */				{ "XVYU",		false, 1, 1,  0,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV444_Planar */				{ "YUV444",		false, 1, 1,  0,  0,  1, 2, 0, 0, 1,   0 },
	/* YUV422_Planar */				{ "YUV422",		false, 1, 1,  0,  0,  1, 2, 1, 0, 1,   0 },
	/* YUV420_Planar */				{ "YUV420",		false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV411_Planar */				{ "YUV411",		false, 1, 1,  0,  0,  1, 2, 2, 0, 1,   0 },
	/* YUV410_Planar */				{ "YUV410",		false, 1, 1,  0,  0,  1, 2, 2, 2, 1,   0 },
	/* YUV422_Planar_Centered */	{ "YUV422C",	false, 1, 1,  0,  0,  1, 2, 1, 0, 1,   0 },
	/* YUV420_Planar_Centered */	{ "YUV420C",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV422_Planar_16F */			{ "YUV422_16F",	false, 1, 1,  0,  0,  2, 2, 1, 0, 2,   0 },
	/* V210 */						{ "v210",		 true,24, 1,  2,  0, 64, 0, 0, 0, 1,   0 },
	/* YUV422_UYVY_709 */			{ "UYVY-709",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* NV12 */						{ "NV12",		false, 1, 1,  0,  0,  1, 1, 1, 1, 2,   0 },
	/* Y8-FR */						{ "I8",			false, 1, 1,  1,  0,  1, 0, 0, 0, 0,   0 },
	/* YUV422_YUYV_709 */			{ "YUYV-709",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV444_Planar_709 */			{ "YUV444-709",	false, 1, 1,  0,  0,  1, 2, 0, 0, 1,   0 },
	/* YUV422_Planar_709 */			{ "YUV422-709",	false, 1, 1,  0,  0,  1, 2, 1, 0, 1,   0 },
	/* YUV420_Planar_709 */			{ "YUV420-709",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV411_Planar_709 */			{ "YUV411-709",	false, 1, 1,  0,  0,  1, 2, 2, 0, 1,   0 },
	/* YUV410_Planar_709 */			{ "YUV410-709",	false, 1, 1,  0,  0,  1, 2, 2, 2, 1,   0 },
	/* YUV422_UYVY_FR */			{ "UYVY-FR",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV422_YUYV_FR */			{ "YUYV-FR",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV444_Planar_FR */			{ "YUV444-FR",	false, 1, 1,  0,  0,  1, 2, 0, 0, 1,   0 },
	/* YUV422_Planar_FR */			{ "YUV422-FR",	false, 1, 1,  0,  0,  1, 2, 1, 0, 1,   0 },
	/* YUV420_Planar_FR */			{ "YUV420-FR",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV411_Planar_FR */			{ "YUV411-FR",	false, 1, 1,  0,  0,  1, 2, 2, 0, 1,   0 },
	/* YUV410_Planar_FR */			{ "YUV410-FR",	false, 1, 1,  0,  0,  1, 2, 2, 2, 1,   0 },
	/* YUV422_UYVY_FR_709 */		{ "UYVY-709-FR",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV422_YUYV_FR_709 */		{ "YUYV-709-FR",	 true, 2, 1,  1,  0,  4, 0, 0, 0, 0,   0 },
	/* YUV444_Planar_FR_709 */		{ "YUV444-709-FR",	false, 1, 1,  0,  0,  1, 2, 0, 0, 1,   0 },
	/* YUV422_Planar_FR_709 */		{ "YUV422-709-FR",	false, 1, 1,  0,  0,  1, 2, 1, 0, 1,   0 },
	/* YUV420_Planar_FR_709 */		{ "YUV420-709-FR",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV411_Planar_FR_709 */		{ "YUV411-709-FR",	false, 1, 1,  0,  0,  1, 2, 2, 0, 1,   0 },
	/* YUV410_Planar_FR_709 */		{ "YUV410-709-FR",	false, 1, 1,  0,  0,  1, 2, 2, 2, 1,   0 },
	/* YUV420i_Planar */			{ "YUV420i",		false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420i_Planar_FR */			{ "YUV420i-FR",		false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420i_Planar_709 */		{ "YUV420i-709",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420i_Planar_709_FR */		{ "YUV420i-709-FR",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420it_Planar */			{ "YUV420it",		false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420it_Planar_FR */		{ "YUV420it-FR",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420it_Planar_709 */		{ "YUV420it-709",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420it_Planar_709_FR */	{ "YUV420it-709-FR",false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420ib_Planar */			{ "YUV420ib",		false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420ib_Planar_FR */		{ "YUV420ib-FR",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420ib_Planar_709 */		{ "YUV420ib-709",	false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
	/* YUV420ib_Planar_709_FR */	{ "YUV420ib-709-FR",false, 1, 1,  0,  0,  1, 2, 1, 1, 1,   0 },
};

namespace {
	void check() {
		VDASSERTCT(sizeof(g_vdPixmapFormats)/sizeof(g_vdPixmapFormats[0]) == nsVDPixmap::kPixFormat_Max_Standard);
	}
}

#ifdef _DEBUG
	bool VDIsValidPixmapPlane(const void *p, ptrdiff_t pitch, vdpixsize w, vdpixsize h) {
		bool isvalid;

		if (pitch < 0)
			isvalid = VDIsValidReadRegion((const char *)p + pitch*(h-1), (-pitch)*(h-1)+w);
		else
			isvalid = VDIsValidReadRegion(p, pitch*(h-1)+w);

		if (!isvalid) {
			VDDEBUG("Kasumi: Invalid pixmap plane detected.\n"
					"        Base=%p, pitch=%d, size=%dx%d (bytes)\n", p, (int)pitch, w, h);
		}

		return isvalid;
	}

	bool VDAssertValidPixmap(const VDPixmap& px) {
		const VDPixmapFormatInfo& info = VDPixmapGetInfo(px.format);

		if (px.format) {
			if (!VDIsValidPixmapPlane(px.data, px.pitch, -(-px.w / info.qw)*info.qsize, -(-px.h >> info.qhbits))) {
				VDDEBUG("Kasumi: Invalid primary plane detected in pixmap.\n"
						"        Pixmap info: format=%d (%s), dimensions=%dx%d\n", px.format, info.name, px.w, px.h);
				VDASSERT(!"Kasumi: Invalid primary plane detected in pixmap.\n");
				return false;
			}

			if (info.palsize)
				if (!VDIsValidReadRegion(px.palette, sizeof(uint32) * info.palsize)) {
					VDDEBUG("Kasumi: Invalid palette detected in pixmap.\n"
							"        Pixmap info: format=%d (%s), dimensions=%dx%d\n", px.format, info.name, px.w, px.h);
					VDASSERT(!"Kasumi: Invalid palette detected in pixmap.\n");
					return false;
				}

			if (info.auxbufs) {
				const vdpixsize auxw = -(-px.w >> info.auxwbits);
				const vdpixsize auxh = -(-px.h >> info.auxhbits);

				if (!VDIsValidPixmapPlane(px.data2, px.pitch2, auxw * info.auxsize, auxh)) {
					VDDEBUG("Kasumi: Invalid Cb plane detected in pixmap.\n"
							"        Pixmap info: format=%d (%s), dimensions=%dx%d\n", px.format, info.name, px.w, px.h);
					VDASSERT(!"Kasumi: Invalid Cb plane detected in pixmap.\n");
					return false;
				}

				if (info.auxbufs > 2) {
					if (!VDIsValidPixmapPlane(px.data3, px.pitch3, auxw * info.auxsize, auxh)) {
						VDDEBUG("Kasumi: Invalid Cr plane detected in pixmap.\n"
								"        Pixmap info: format=%d, dimensions=%dx%d\n", px.format, px.w, px.h);
						VDASSERT(!"Kasumi: Invalid Cr plane detected in pixmap.\n");
						return false;
					}
				}
			}
		}

		return true;
	}
#endif

VDPixmap VDPixmapOffset(const VDPixmap& src, vdpixpos x, vdpixpos y) {
	VDPixmap temp(src);
	const VDPixmapFormatInfo& info = VDPixmapGetInfo(temp.format);

	if (info.qchunky) {
		x = (x + info.qw - 1) / info.qw;
		y >>= info.qhbits;
	}

	switch(info.auxbufs) {
	case 2:
		temp.data3 = (char *)temp.data3 + (x >> info.auxwbits)*info.auxsize + (y >> info.auxhbits)*temp.pitch3;
	case 1:
		temp.data2 = (char *)temp.data2 + (x >> info.auxwbits)*info.auxsize + (y >> info.auxhbits)*temp.pitch2;
	case 0:
		temp.data = (char *)temp.data + x*info.qsize + y*temp.pitch;
	}

	return temp;
}

VDPixmapLayout VDPixmapLayoutOffset(const VDPixmapLayout& src, vdpixpos x, vdpixpos y) {
	VDPixmapLayout temp(src);
	const VDPixmapFormatInfo& info = VDPixmapGetInfo(temp.format);

	if (info.qchunky) {
		x = (x + info.qw - 1) / info.qw;
		y = -(-y >> info.qhbits);
	}

	switch(info.auxbufs) {
	case 2:
		temp.data3 += -(-x >> info.auxwbits)*info.auxsize + -(-y >> info.auxhbits)*temp.pitch3;
	case 1:
		temp.data2 += -(-x >> info.auxwbits)*info.auxsize + -(-y >> info.auxhbits)*temp.pitch2;
	case 0:
		temp.data += x*info.qsize + y*temp.pitch;
	}

	return temp;
}

uint32 VDPixmapCreateLinearLayout(VDPixmapLayout& layout, int format, vdpixsize w, vdpixsize h, int alignment) {
	const ptrdiff_t alignmask = alignment - 1;

	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(format);
	sint32		qw			= (w + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-h >> srcinfo.qhbits);
	sint32		subw		= -(-w >> srcinfo.auxwbits);
	sint32		subh		= -(-h >> srcinfo.auxhbits);
	sint32		auxsize		= srcinfo.auxsize;

	ptrdiff_t	mainpitch	= (srcinfo.qsize * qw + alignmask) & ~alignmask;
	size_t		mainsize	= mainpitch * qh;

	layout.data		= 0;
	layout.pitch	= mainpitch;
	layout.palette	= NULL;
	layout.data2	= 0;
	layout.pitch2	= 0;
	layout.data3	= 0;
	layout.pitch3	= 0;
	layout.w		= w;
	layout.h		= h;
	layout.format	= format;

	if (srcinfo.auxbufs >= 1) {
		ptrdiff_t	subpitch	= (subw * auxsize + alignmask) & ~alignmask;
		size_t		subsize		= subpitch * subh;

		layout.data2	= mainsize;
		layout.pitch2	= subpitch;
		mainsize += subsize;

		if (srcinfo.auxbufs >= 2) {
			layout.data3	= mainsize;
			layout.pitch3	= subpitch;
			mainsize += subsize;
		}
	}

	return mainsize;
}

void VDPixmapFlipV(VDPixmap& px) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(px.format);
	sint32		w			= px.w;
	sint32		h			= px.h;
	sint32		qw			= (w + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-h >> srcinfo.qhbits);
	sint32		subh		= -(-h >> srcinfo.auxhbits);

	vdptrstep(px.data, px.pitch * (qh - 1));
	px.pitch = -px.pitch;

	if (srcinfo.auxbufs >= 1) {
		vdptrstep(px.data2, px.pitch2 * (subh - 1));
		px.pitch2 = -px.pitch2;

		if (srcinfo.auxbufs >= 2) {
			vdptrstep(px.data3, px.pitch3 * (subh - 1));
			px.pitch3 = -px.pitch3;
		}
	}
}

void VDPixmapLayoutFlipV(VDPixmapLayout& layout) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(layout.format);
	sint32		w			= layout.w;
	sint32		h			= layout.h;
	sint32		qw			= (w + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-h >> srcinfo.qhbits);
	sint32		subh		= -(-h >> srcinfo.auxhbits);

	layout.data += layout.pitch * (qh - 1);
	layout.pitch = -layout.pitch;

	if (srcinfo.auxbufs >= 1) {
		layout.data2 += layout.pitch2 * (subh - 1);
		layout.pitch2 = -layout.pitch2;

		if (srcinfo.auxbufs >= 2) {
			layout.data3 += layout.pitch3 * (subh - 1);
			layout.pitch3 = -layout.pitch3;
		}
	}
}

uint32 VDPixmapLayoutGetMinSize(const VDPixmapLayout& layout) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(layout.format);
	sint32		w			= layout.w;
	sint32		h			= layout.h;
	sint32		qw			= (w + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-h >> srcinfo.qhbits);
	sint32		subh		= -(-h >> srcinfo.auxhbits);

	uint32 limit = layout.data;
	if (layout.pitch >= 0)
		limit += layout.pitch * qh;
	else
		limit -= layout.pitch;

	if (srcinfo.auxbufs >= 1) {
		uint32 limit2 = layout.data2;

		if (layout.pitch2 >= 0)
			limit2 += layout.pitch2 * subh;
		else
			limit2 -= layout.pitch2;

		if (limit < limit2)
			limit = limit2;

		if (srcinfo.auxbufs >= 2) {
			uint32 limit3 = layout.data3;

			if (layout.pitch3 >= 0)
				limit3 += layout.pitch3 * subh;
			else
				limit3 -= layout.pitch3;

			if (limit < limit3)
				limit = limit3;
		}
	}

	return limit;
}

VDPixmap VDPixmapExtractField(const VDPixmap& src, bool field2) {
	VDPixmap px(src);

	if (field2) {
		const VDPixmapFormatInfo& info = VDPixmapGetInfo(px.format);

		if (px.data) {
			if (info.qh == 1)
				vdptrstep(px.data, px.pitch);

			if (!info.auxhbits ||
				src.format == nsVDPixmap::kPixFormat_YUV420i_Planar ||
				src.format == nsVDPixmap::kPixFormat_YUV420i_Planar_FR ||
				src.format == nsVDPixmap::kPixFormat_YUV420i_Planar_709 ||
				src.format == nsVDPixmap::kPixFormat_YUV420i_Planar_709_FR) {

				vdptrstep(px.data2, px.pitch2);
				vdptrstep(px.data3, px.pitch3);
			}
		}
	}

	switch(src.format) {
		case nsVDPixmap::kPixFormat_YUV420i_Planar:
			if (field2)
				px.format = nsVDPixmap::kPixFormat_YUV420ib_Planar;
			else
				px.format = nsVDPixmap::kPixFormat_YUV420it_Planar;
			break;

		case nsVDPixmap::kPixFormat_YUV420i_Planar_FR:
			if (field2)
				px.format = nsVDPixmap::kPixFormat_YUV420ib_Planar_FR;
			else
				px.format = nsVDPixmap::kPixFormat_YUV420it_Planar_FR;
			break;

		case nsVDPixmap::kPixFormat_YUV420i_Planar_709:
			if (field2)
				px.format = nsVDPixmap::kPixFormat_YUV420ib_Planar_709;
			else
				px.format = nsVDPixmap::kPixFormat_YUV420it_Planar_709;
			break;

		case nsVDPixmap::kPixFormat_YUV420i_Planar_709_FR:
			if (field2)
				px.format = nsVDPixmap::kPixFormat_YUV420ib_Planar_709_FR;
			else
				px.format = nsVDPixmap::kPixFormat_YUV420it_Planar_709_FR;
			break;
	}

	px.h >>= 1;
	px.pitch += px.pitch;
	px.pitch2 += px.pitch2;
	px.pitch3 += px.pitch3;
	return px;
}

///////////////////////////////////////////////////////////////////////////

VDPixmapBuffer::VDPixmapBuffer(const VDPixmap& src)
	: mpBuffer(NULL)
	, mLinearSize(0)
{
	assign(src);
}

VDPixmapBuffer::VDPixmapBuffer(const VDPixmapBuffer& src)
	: mpBuffer(NULL)
	, mLinearSize(0)
{
	assign(src);
}

VDPixmapBuffer::VDPixmapBuffer(const VDPixmapLayout& layout) {
	init(layout);
}

VDPixmapBuffer::~VDPixmapBuffer() {
#ifdef _DEBUG
	validate();
#endif

	delete[] mpBuffer;
}

void VDPixmapBuffer::init(sint32 width, sint32 height, int f) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(f);
	sint32		qw			= (width + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-height >> srcinfo.qhbits);
	sint32		subw		= -(-width >> srcinfo.auxwbits);
	sint32		subh		= -(-height >> srcinfo.auxhbits);
	ptrdiff_t	mainpitch	= (srcinfo.qsize * qw + 15) & ~15;
	ptrdiff_t	subpitch	= (srcinfo.auxsize * subw + 15) & ~15;
	uint64		mainsize	= (uint64)mainpitch * qh;
	uint64		subsize		= (uint64)subpitch * subh;
	uint64		totalsize64	= mainsize + subsize*srcinfo.auxbufs + 4 * srcinfo.palsize;

#ifdef _DEBUG
	totalsize64 += 28;
#endif

	// reject huge allocations
	if (totalsize64 > (size_t)-1 - 4096)
		throw MyMemoryError();

	size_t totalsize = (uint32)totalsize64;

	if (mLinearSize != totalsize) {
		clear();
		mpBuffer = new_nothrow char[totalsize + 15];
		if (!mpBuffer)
			throw MyMemoryError(totalsize + 15);
		mLinearSize = totalsize;
	}

	char *p = mpBuffer + (-(int)(uintptr)mpBuffer & 15);

#ifdef _DEBUG
	*(uint32 *)p = totalsize;
	for(int i=0; i<12; ++i)
		p[4+i] = (char)(0xa0 + i);

	p += 16;
#endif

	data	= p;
	pitch	= mainpitch;
	p += mainsize;

	palette	= NULL;
	data2	= NULL;
	pitch2	= NULL;
	data3	= NULL;
	pitch3	= NULL;
	w		= width;
	h		= height;
	format	= f;

	if (srcinfo.auxbufs >= 1) {
		data2	= p;
		pitch2	= subpitch;
		p += subsize;
	}

	if (srcinfo.auxbufs >= 2) {
		data3	= p;
		pitch3	= subpitch;
		p += subsize;
	}

	if (srcinfo.palsize) {
		palette = (const uint32 *)p;
		p += srcinfo.palsize * 4;
	}

#ifdef _DEBUG
	for(int j=0; j<12; ++j)
		p[j] = (char)(0xb0 + j);
#endif
}

void VDPixmapBuffer::init(const VDPixmapLayout& layout, uint32 additionalPadding) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(layout.format);
	sint32		qw			= (layout.w + srcinfo.qw - 1) / srcinfo.qw;
	sint32		qh			= -(-layout.h >> srcinfo.qhbits);
	sint32		subw		= -(-layout.w >> srcinfo.auxwbits);
	sint32		subh		= -(-layout.h >> srcinfo.auxhbits);

	sint64 mino=0, maxo=0;

	if (layout.pitch < 0) {
		mino = std::min<sint64>(mino, layout.data + (sint64)layout.pitch * (qh-1));
		maxo = std::max<sint64>(maxo, layout.data - (sint64)layout.pitch);
	} else {
		mino = std::min<sint64>(mino, layout.data);
		maxo = std::max<sint64>(maxo, layout.data + (sint64)layout.pitch*qh);
	}

	if (srcinfo.auxbufs >= 1) {
		if (layout.pitch2 < 0) {
			mino = std::min<sint64>(mino, layout.data2 + (sint64)layout.pitch2 * (subh-1));
			maxo = std::max<sint64>(maxo, layout.data2 - (sint64)layout.pitch2);
		} else {
			mino = std::min<sint64>(mino, layout.data2);
			maxo = std::max<sint64>(maxo, layout.data2 + (sint64)layout.pitch2*subh);
		}

		if (srcinfo.auxbufs >= 2) {
			if (layout.pitch3 < 0) {
				mino = std::min<sint64>(mino, layout.data3 + (sint64)layout.pitch3 * (subh-1));
				maxo = std::max<sint64>(maxo, layout.data3 - (sint64)layout.pitch3);
			} else {
				mino = std::min<sint64>(mino, layout.data3);
				maxo = std::max<sint64>(maxo, layout.data3 + (sint64)layout.pitch3*subh);
			}
		}
	}

	sint64 linsize64 = ((maxo - mino + 3) & ~(uint64)3);

	sint64 totalsize64 = linsize64 + 4*srcinfo.palsize + additionalPadding;

#ifdef _DEBUG
	totalsize64 += 28;
#endif

	// reject huge allocations
	if (totalsize64 > (size_t)-1 - 4096)
		throw MyMemoryError();

	size_t totalsize = (uint32)totalsize64;
	ptrdiff_t linsize = (uint32)linsize64;

	if (mLinearSize != totalsize) {
		clear();
		mpBuffer = new_nothrow char[totalsize + 15];
		if (!mpBuffer)
			throw MyMemoryError(totalsize + 15);
		mLinearSize = totalsize;
	}

	char *p = mpBuffer + (-(int)(uintptr)mpBuffer & 15);

#ifdef _DEBUG
	*(uint32 *)p = totalsize - 28;
	for(int i=0; i<12; ++i)
		p[4+i] = (char)(0xa0 + i);

	p += 16;
#endif

	w		= layout.w;
	h		= layout.h;
	format	= layout.format;
	data	= p + layout.data - mino;
	data2	= p + layout.data2 - mino;
	data3	= p + layout.data3 - mino;
	pitch	= layout.pitch;
	pitch2	= layout.pitch2;
	pitch3	= layout.pitch3;
	palette	= NULL;

	if (srcinfo.palsize) {
		palette = (const uint32 *)(p + linsize);

		if (layout.palette)
			memcpy((void *)palette, layout.palette, 4*srcinfo.palsize);
	}

#ifdef _DEBUG
	for(int j=0; j<12; ++j)
		p[totalsize + j - 28] = (char)(0xb0 + j);
#endif

	VDAssertValidPixmap(*this);
}

void VDPixmapBuffer::assign(const VDPixmap& src) {
	if (!src.format) {
		delete[] mpBuffer;
		mpBuffer = NULL;
		data = NULL;
		format = 0;
	} else {
		init(src.w, src.h, src.format);

		const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(src.format);
		int qw = (src.w + srcinfo.qw - 1) / srcinfo.qw;
		int qh = -(-src.h >> srcinfo.qhbits);
		int subw = -(-src.w >> srcinfo.auxwbits);
		int subh = -(-src.h >> srcinfo.auxhbits);

		if (srcinfo.palsize)
			memcpy((void *)palette, src.palette, 4 * srcinfo.palsize);

		switch(srcinfo.auxbufs) {
		case 2:
			VDMemcpyRect(data3, pitch3, src.data3, src.pitch3, subw, subh);
		case 1:
			VDMemcpyRect(data2, pitch2, src.data2, src.pitch2, subw, subh);
		case 0:
			VDMemcpyRect(data, pitch, src.data, src.pitch, qw * srcinfo.qsize, qh);
		}
	}
}

void VDPixmapBuffer::swap(VDPixmapBuffer& dst) {
	std::swap(mpBuffer, dst.mpBuffer);
	std::swap(mLinearSize, dst.mLinearSize);
	std::swap(static_cast<VDPixmap&>(*this), static_cast<VDPixmap&>(dst));
}

#ifdef _DEBUG
void VDPixmapBuffer::validate() {
	if (mpBuffer) {
		char *p = (char *)(((uintptr)mpBuffer + 15) & ~(uintptr)15);

		// verify head bytes
		for(int i=0; i<12; ++i)
			if (p[i+4] != (char)(0xa0 + i))
				VDASSERT(!"VDPixmapBuffer: Buffer underflow detected.\n");

		// verify tail bytes
		for(int j=0; j<12; ++j)
			if (p[mLinearSize - 12 + j] != (char)(0xb0 + j))
				VDASSERT(!"VDPixmapBuffer: Buffer overflow detected.\n");
	}
}
#endif
