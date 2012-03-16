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
#include <vd2/system/halffloat.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixel.h>

uint32 VDPixmapSample(const VDPixmap& px, sint32 x, sint32 y) {
	if (x >= px.w)
		x = px.w - 1;
	if (y >= px.h)
		y = px.h - 1;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	switch(px.format) {
	case nsVDPixmap::kPixFormat_Pal1:
		{
			uint8 idx = ((const uint8 *)px.data + px.pitch*y)[x >> 3];

			return px.palette[(idx >> (7 - (x & 7))) & 1];
		}

	case nsVDPixmap::kPixFormat_Pal2:
		{
			uint8 idx = ((const uint8 *)px.data + px.pitch*y)[x >> 2];

			return px.palette[(idx >> (6 - (x & 3)*2)) & 3];
		}

	case nsVDPixmap::kPixFormat_Pal4:
		{
			uint8 idx = ((const uint8 *)px.data + px.pitch*y)[x >> 1];

			if (!(x & 1))
				idx >>= 4;

			return px.palette[idx & 15];
		}

	case nsVDPixmap::kPixFormat_Pal8:
		{
			uint8 idx = ((const uint8 *)px.data + px.pitch*y)[x];

			return px.palette[idx];
		}

	case nsVDPixmap::kPixFormat_XRGB1555:
		{
			uint16 c = ((const uint16 *)((const uint8 *)px.data + px.pitch*y))[x];
			uint32 r = c & 0x7c00;
			uint32 g = c & 0x03e0;
			uint32 b = c & 0x001f;
			uint32 rgb = (r << 9) + (g << 6) + (b << 3);

			return rgb + ((rgb >> 5) & 0x070707);
		}
		break;

	case nsVDPixmap::kPixFormat_RGB565:
		{
			uint16 c = ((const uint16 *)((const uint8 *)px.data + px.pitch*y))[x];
			uint32 r = c & 0xf800;
			uint32 g = c & 0x07e0;
			uint32 b = c & 0x001f;
			uint32 rb = (r << 8) + (b << 3);

			return rb + ((rb >> 5) & 0x070007) + (g << 5) + ((g >> 1) & 0x0300);
		}
		break;

	case nsVDPixmap::kPixFormat_RGB888:
		{
			const uint8 *src = (const uint8 *)px.data + px.pitch*y + 3*x;
			uint32 b = src[0];
			uint32 g = src[1];
			uint32 r = src[2];

			return (r << 16) + (g << 8) + b;
		}
		break;

	case nsVDPixmap::kPixFormat_XRGB8888:
		return ((const uint32 *)((const uint8 *)px.data + px.pitch*y))[x];

	case nsVDPixmap::kPixFormat_Y8:
		{
			uint8 luma = ((const uint8 *)px.data + px.pitch*y)[x];

			return ((luma - 16)*255/219) * 0x010101;
		}
		break;

	case nsVDPixmap::kPixFormat_Y8_FR:
		{
			uint8 luma = ((const uint8 *)px.data + px.pitch*y)[x];

			return (uint32)luma * 0x010101;
		}
		break;

	case nsVDPixmap::kPixFormat_YUV444_Planar:
		return VDConvertYCbCrToRGB(VDPixmapSample8(px.data, px.pitch, x, y), VDPixmapSample8(px.data2, px.pitch2, x, y), VDPixmapSample8(px.data3, px.pitch3, x, y), false, false);

	case nsVDPixmap::kPixFormat_YUV422_Planar:
		{
			sint32 u = (x << 7) + 128;
			sint32 v = (y << 8);
			uint32 w2 = px.w >> 1;
			uint32 h2 = px.h;

			return VDConvertYCbCrToRGB(
						VDPixmapSample8(px.data, px.pitch, x, y),
						VDPixmapInterpolateSample8(px.data2, px.pitch2, w2, h2, u, v),
						VDPixmapInterpolateSample8(px.data3, px.pitch3, w2, h2, u, v),
						false, false);
		}

	case nsVDPixmap::kPixFormat_YUV420_Planar:
		{
			sint32 u = (x << 7) + 128;
			sint32 v = (y << 7);
			uint32 w2 = px.w >> 1;
			uint32 h2 = px.h >> 1;

			return VDConvertYCbCrToRGB(
						VDPixmapSample8(px.data, px.pitch, x, y),
						VDPixmapInterpolateSample8(px.data2, px.pitch2, w2, h2, u, v),
						VDPixmapInterpolateSample8(px.data3, px.pitch3, w2, h2, u, v),
						false, false);
		}

	case nsVDPixmap::kPixFormat_YUV411_Planar:
		{
			sint32 u = (x << 6) + 128;
			sint32 v = (y << 8);
			uint32 w2 = px.w >> 2;
			uint32 h2 = px.h;

			return VDConvertYCbCrToRGB(
						VDPixmapSample8(px.data, px.pitch, x, y),
						VDPixmapInterpolateSample8(px.data2, px.pitch2, w2, h2, u, v),
						VDPixmapInterpolateSample8(px.data3, px.pitch3, w2, h2, u, v),
						false, false);
		}

	case nsVDPixmap::kPixFormat_YUV410_Planar:
		{
			sint32 u = (x << 6) + 128;
			sint32 v = (y << 6);
			uint32 w2 = px.w >> 2;
			uint32 h2 = px.h >> 2;

			return VDConvertYCbCrToRGB(
						VDPixmapSample8(px.data, px.pitch, x, y),
						VDPixmapInterpolateSample8(px.data2, px.pitch2, w2, h2, u, v),
						VDPixmapInterpolateSample8(px.data3, px.pitch3, w2, h2, u, v),
						false, false);
		}

	default:
		return VDPixmapInterpolateSampleRGB24(px, (x << 8) + 128, (y << 8) + 128);
	}
}

uint8 VDPixmapInterpolateSample8(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256) {
	// bias coordinates to integer
	x_256 -= 128;
	y_256 -= 128;

	// clamp coordinates
	x_256 &= ~(x_256 >> 31);
	y_256 &= ~(y_256 >> 31);

	uint32 w_256 = (w - 1) << 8;
	uint32 h_256 = (h - 1) << 8;
	x_256 ^= (x_256 ^ w_256) & ((x_256 - w_256) >> 31);
	y_256 ^= (y_256 ^ h_256) & ((y_256 - h_256) >> 31);

	const uint8 *row0 = (const uint8 *)data + pitch * (y_256 >> 8);
	const uint8 *row1 = row0;

	if ((uint32)y_256 < h_256)
		row1 += pitch;

	ptrdiff_t xstep = (uint32)x_256 < w_256 ? 1 : 0;
	sint32 xoffset = x_256 & 255;
	sint32 yoffset = y_256 & 255;
	sint32 p00 = row0[0];
	sint32 p10 = row0[xstep];
	sint32 p01 = row1[0];
	sint32 p11 = row1[xstep];
	sint32 p0 = (p00 << 8) + (p10 - p00)*xoffset;
	sint32 p1 = (p01 << 8) + (p11 - p01)*xoffset;
	sint32 p = ((p0 << 8) + (p1 - p0)*yoffset + 0x8000) >> 16;

	return (uint8)p;
}

uint32 VDPixmapInterpolateSample8To24(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256) {
	// bias coordinates to integer
	x_256 -= 128;
	y_256 -= 128;

	// clamp coordinates
	x_256 &= ~(x_256 >> 31);
	y_256 &= ~(y_256 >> 31);

	sint32 w_256 = (w - 1) << 8;
	sint32 h_256 = (h - 1) << 8;
	x_256 += (w_256 - x_256) & ((w_256 - x_256) >> 31);
	y_256 += (h_256 - y_256) & ((h_256 - y_256) >> 31);

	const uint8 *row0 = (const uint8 *)data + pitch * (y_256 >> 8) + (x_256 >> 8);
	const uint8 *row1 = row0;

	if (y_256 < h_256)
		row1 += pitch;

	ptrdiff_t xstep = x_256 < w_256 ? 1 : 0;
	sint32 xoffset = x_256 & 255;
	sint32 yoffset = y_256 & 255;
	sint32 p00 = row0[0];
	sint32 p10 = row0[xstep];
	sint32 p01 = row1[0];
	sint32 p11 = row1[xstep];
	sint32 p0 = (p00 << 8) + (p10 - p00)*xoffset;
	sint32 p1 = (p01 << 8) + (p11 - p01)*xoffset;
	sint32 p = (p0 << 8) + (p1 - p0)*yoffset;

	return p;
}

uint32 VDPixmapInterpolateSample8x2To24(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256) {
	// bias coordinates to integer
	x_256 -= 128;
	y_256 -= 128;

	// clamp coordinates
	x_256 &= ~(x_256 >> 31);
	y_256 &= ~(y_256 >> 31);

	uint32 w_256 = (w - 1) << 8;
	uint32 h_256 = (h - 1) << 8;
	x_256 ^= (x_256 ^ w_256) & ((x_256 - w_256) >> 31);
	y_256 ^= (y_256 ^ h_256) & ((y_256 - h_256) >> 31);

	const uint8 *row0 = (const uint8 *)data + pitch * (y_256 >> 8) + (x_256 >> 8)*2;
	const uint8 *row1 = row0;

	if ((uint32)y_256 < h_256)
		row1 += pitch;

	ptrdiff_t xstep = (uint32)x_256 < w_256 ? 2 : 0;
	sint32 xoffset = x_256 & 255;
	sint32 yoffset = y_256 & 255;
	sint32 p00 = row0[0];
	sint32 p10 = row0[xstep];
	sint32 p01 = row1[0];
	sint32 p11 = row1[xstep];
	sint32 p0 = (p00 << 8) + (p10 - p00)*xoffset;
	sint32 p1 = (p01 << 8) + (p11 - p01)*xoffset;
	sint32 p = (p0 << 8) + (p1 - p0)*yoffset;

	return p;
}

uint32 VDPixmapInterpolateSample8x4To24(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256) {
	// bias coordinates to integer
	x_256 -= 128;
	y_256 -= 128;

	// clamp coordinates
	x_256 &= ~(x_256 >> 31);
	y_256 &= ~(y_256 >> 31);

	uint32 w_256 = (w - 1) << 8;
	uint32 h_256 = (h - 1) << 8;
	x_256 ^= (x_256 ^ w_256) & ((x_256 - w_256) >> 31);
	y_256 ^= (y_256 ^ h_256) & ((y_256 - h_256) >> 31);

	const uint8 *row0 = (const uint8 *)data + pitch * (y_256 >> 8) + (x_256 >> 8)*4;
	const uint8 *row1 = row0;

	if ((uint32)y_256 < h_256)
		row1 += pitch;

	ptrdiff_t xstep = (uint32)x_256 < w_256 ? 4 : 0;
	sint32 xoffset = x_256 & 255;
	sint32 yoffset = y_256 & 255;
	sint32 p00 = row0[0];
	sint32 p10 = row0[xstep];
	sint32 p01 = row1[0];
	sint32 p11 = row1[xstep];
	sint32 p0 = (p00 << 8) + (p10 - p00)*xoffset;
	sint32 p1 = (p01 << 8) + (p11 - p01)*xoffset;
	sint32 p = (p0 << 8) + (p1 - p0)*yoffset;

	return p;
}

float VDPixmapInterpolateSample16F(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256) {
	// bias coordinates to integer
	x_256 -= 128;
	y_256 -= 128;

	// clamp coordinates
	x_256 &= ~(x_256 >> 31);
	y_256 &= ~(y_256 >> 31);

	uint32 w_256 = (w - 1) << 8;
	uint32 h_256 = (h - 1) << 8;
	x_256 ^= (x_256 ^ w_256) & ((x_256 - w_256) >> 31);
	y_256 ^= (y_256 ^ h_256) & ((y_256 - h_256) >> 31);

	const uint16 *row0 = (const uint16 *)((const uint8 *)data + pitch * (y_256 >> 8) + (x_256 >> 8)*2);
	const uint16 *row1 = row0;

	if ((uint32)y_256 < h_256)
		row1 = (const uint16 *)((const char *)row1 + pitch);

	ptrdiff_t xstep = (uint32)x_256 < w_256 ? 1 : 0;
	float xoffset = (float)(x_256 & 255) * (1.0f / 255.0f);
	float yoffset = (float)(y_256 & 255) * (1.0f / 255.0f);

	float p00;
	float p10;
	float p01;
	float p11;
	VDConvertHalfToFloat(row0[0], &p00);
	VDConvertHalfToFloat(row0[xstep], &p10);
	VDConvertHalfToFloat(row1[0], &p01);
	VDConvertHalfToFloat(row1[xstep], &p11);

	float p0 = p00 + (p10 - p00)*xoffset;
	float p1 = p01 + (p11 - p01)*xoffset;

	return p0 + (p1 - p0)*yoffset;
}

namespace {
	uint32 Lerp8888(uint32 p0, uint32 p1, uint32 p2, uint32 p3, uint32 xf, uint32 yf) {
		uint32 rb0 = p0 & 0x00ff00ff;
		uint32 ag0 = p0 & 0xff00ff00;
		uint32 rb1 = p1 & 0x00ff00ff;
		uint32 ag1 = p1 & 0xff00ff00;
		uint32 rb2 = p2 & 0x00ff00ff;
		uint32 ag2 = p2 & 0xff00ff00;
		uint32 rb3 = p3 & 0x00ff00ff;
		uint32 ag3 = p3 & 0xff00ff00;

		uint32 rbt = (rb0 + (((       rb1 - rb0       )*xf + 0x00800080) >> 8)) & 0x00ff00ff;
		uint32 agt = (ag0 + ((((ag1 >> 8) - (ag0 >> 8))*xf + 0x00800080)     )) & 0xff00ff00;
		uint32 rbb = (rb2 + (((       rb3 - rb2       )*xf + 0x00800080) >> 8)) & 0x00ff00ff;
		uint32 agb = (ag2 + ((((ag3 >> 8) - (ag2 >> 8))*xf + 0x00800080)     )) & 0xff00ff00;
		uint32 rb  = (rbt + (((       rbb - rbt       )*yf + 0x00800080) >> 8)) & 0x00ff00ff;
		uint32 ag  = (agt + ((((agb >> 8) - (agt >> 8))*yf + 0x00800080)     )) & 0xff00ff00;

		return rb + ag;
	}

	uint32 InterpPlanarY8(const VDPixmap& px, sint32 x1, sint32 y1) {
		sint32 y = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);

		return VDClampedRoundFixedToUint8Fast((float)(y-0x100000) * (1.1643836f/65536.0f/255.0f))*0x010101;
	}

	uint32 ConvertYCC72ToRGB24(sint32 iy, sint32 icb, sint32 icr) {
		float y  = (float)iy;
		float cb = (float)icb;
		float cr = (float)icr;

		//	!   1.1643836  - 5.599D-17    1.5960268  - 222.92157 !
		//	!   1.1643836  - 0.3917623  - 0.8129676    135.57529 !
		//	!   1.1643836    2.0172321  - 1.110D-16  - 276.83585 !
		uint32 ir = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y + (1.5960268f/65536.0f/255.0f)*cr - (222.92157f / 255.0f));
		uint32 ig = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y - (0.3917623f/65536.0f/255.0f)*cb - (0.8129676f/65536.0f/255.0f)*cr + (135.57529f / 255.0f));
		uint32 ib = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y + (2.0172321f/65536.0f/255.0f)*cb - (276.83585f / 255.0f));

		return (ir << 16) + (ig << 8) + ib;
	}

	uint32 ConvertYCC72ToRGB24_FR(sint32 iy, sint32 icb, sint32 icr) {
		float y  = (float)iy;
		float cb = (float)icb;
		float cr = (float)icr;

		//	1.    0.           1.402      - 179.456    
		//	1.  - 0.3441363  - 0.7141363    135.45889 
		//	1.    1.772      - 2.220D-16  - 226.816    
 		uint32 ir = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y + (1.4020000f/65536.0f/255.0f)*cr - (179.456f / 255.0f));
		uint32 ig = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y - (0.3441363f/65536.0f/255.0f)*cb - (0.7141363f/65536.0f/255.0f)*cr + (135.45889f / 255.0f));
		uint32 ib = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y + (1.7720000f/65536.0f/255.0f)*cb - (226.816f / 255.0f));

		return (ir << 16) + (ig << 8) + ib;
	}

	uint32 ConvertYCC72ToRGB24_709(sint32 iy, sint32 icb, sint32 icr) {
		float y  = (float)iy;
		float cb = (float)icb;
		float cr = (float)icr;

		//	!   1.1643836  - 2.932D-17    1.7927411  - 248.10099 !
		//	!   1.1643836  - 0.2132486  - 0.5329093    76.87808  !
		//	!   1.1643836    2.1124018  - 5.551D-17  - 289.01757 !
		uint32 ir = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y + (1.7927411f/65536.0f/255.0f)*cr - (248.10099f / 255.0f));
		uint32 ig = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y - (0.2132486f/65536.0f/255.0f)*cb - (0.5329093f/65536.0f/255.0f)*cr + (76.87808f / 255.0f));
		uint32 ib = VDClampedRoundFixedToUint8Fast((1.1643836f/65536.0f/255.0f)*y + (2.1124018f/65536.0f/255.0f)*cb - (289.01757f / 255.0f));

		return (ir << 16) + (ig << 8) + ib;
	}

	uint32 ConvertYCC72ToRGB24_709_FR(sint32 iy, sint32 icb, sint32 icr) {
		float y  = (float)iy;
		float cb = (float)icb;
		float cr = (float)icr;

		//	    1.    0.           1.5748     - 201.5744   
		//	    1.  - 0.1873243  - 0.4681243    83.897414  
		//	    1.    1.8556       0.         - 237.5168   
 		uint32 ir = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y + (1.5748f/65536.0f/255.0f)*cr - (201.5744f / 255.0f));
		uint32 ig = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y - (0.1873243f/65536.0f/255.0f)*cb - (0.4681243f/65536.0f/255.0f)*cr + (83.897414f / 255.0f));
		uint32 ib = VDClampedRoundFixedToUint8Fast((1.0f/65536.0f/255.0f)*y + (1.8556f/65536.0f/255.0f)*cb - (237.5168f / 255.0f));

		return (ir << 16) + (ig << 8) + ib;
	}

	uint32 InterpPlanarYCC888(const VDPixmap& px, sint32 x1, sint32 y1, sint32 x23, sint32 y23, uint32 w23, uint32 h23) {
		sint32 y  = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);
		sint32 cb = VDPixmapInterpolateSample8To24(px.data2, px.pitch2, w23, h23, x23, y23);
		sint32 cr = VDPixmapInterpolateSample8To24(px.data3, px.pitch3, w23, h23, x23, y23);

		return ConvertYCC72ToRGB24(y, cb, cr);
	}

	uint32 InterpPlanarYCC888_709(const VDPixmap& px, sint32 x1, sint32 y1, sint32 x23, sint32 y23, uint32 w23, uint32 h23) {
		sint32 y  = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);
		sint32 cb = VDPixmapInterpolateSample8To24(px.data2, px.pitch2, w23, h23, x23, y23);
		sint32 cr = VDPixmapInterpolateSample8To24(px.data3, px.pitch3, w23, h23, x23, y23);

		return ConvertYCC72ToRGB24_709(y, cb, cr);
	}

	uint32 InterpPlanarYCC888_FR(const VDPixmap& px, sint32 x1, sint32 y1, sint32 x23, sint32 y23, uint32 w23, uint32 h23) {
		sint32 y  = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);
		sint32 cb = VDPixmapInterpolateSample8To24(px.data2, px.pitch2, w23, h23, x23, y23);
		sint32 cr = VDPixmapInterpolateSample8To24(px.data3, px.pitch3, w23, h23, x23, y23);

		return ConvertYCC72ToRGB24_FR(y, cb, cr);
	}

	uint32 InterpPlanarYCC888_709_FR(const VDPixmap& px, sint32 x1, sint32 y1, sint32 x23, sint32 y23, uint32 w23, uint32 h23) {
		sint32 y  = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);
		sint32 cb = VDPixmapInterpolateSample8To24(px.data2, px.pitch2, w23, h23, x23, y23);
		sint32 cr = VDPixmapInterpolateSample8To24(px.data3, px.pitch3, w23, h23, x23, y23);

		return ConvertYCC72ToRGB24_709_FR(y, cb, cr);
	}

	template<uint32 (*ConvFn)(sint32, sint32, sint32)>
	uint32 InterpPlanarYCC888_420i(const VDPixmap& px, sint32 x1, sint32 y1) {
		sint32 y  = VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x1, y1);
		sint32 cb;
		sint32 cr;

		const uint8 *src2 = (const uint8 *)px.data2;
		const uint8 *src3 = (const uint8 *)px.data3;
		const ptrdiff_t pitch2 = px.pitch2 + px.pitch2;
		const ptrdiff_t pitch3 = px.pitch3 + px.pitch3;
		const uint32 w23 = (px.w + 1) >> 1;
		const uint32 h23 = (px.h + 1) >> 1;
		const sint32 xc = (x1 >> 1) + 64;
		sint32 yc = (y1 >> 1) + 64;

		if (y1 & 1) {
			yc -= 256;
			cb = VDPixmapInterpolateSample8To24(src2, pitch2, w23, h23 >> 1, xc, yc);
			cr = VDPixmapInterpolateSample8To24(src3, pitch3, w23, h23 >> 1, xc, yc);
		} else {
			cb = VDPixmapInterpolateSample8To24(src2 + px.pitch2, pitch2, w23, (h23 + 1) >> 1, xc, yc);
			cr = VDPixmapInterpolateSample8To24(src3 + px.pitch3, pitch3, w23, (h23 + 1) >> 1, xc, yc);
		}

		return ConvFn(y, cb, cr);
	}

	uint32 SampleV210_Y(const void *src, ptrdiff_t srcpitch, sint32 x, sint32 y, uint32 w, uint32 h) {
		if (x < 0)
			x = 0;
		if ((uint32)x >= w)
			x = w - 1;
		if (y < 0)
			y = 0;
		if ((uint32)y >= h)
			y = h - 1;

		const uint32 *p = (const uint32 *)((const char *)src + srcpitch*y) + (x / 6)*4;

		switch((uint32)x % 6) {
			default:
			case 0:	return (p[0] >> 10) & 0x3ff;
			case 1:	return (p[1] >>  0) & 0x3ff;
			case 2:	return (p[1] >> 20) & 0x3ff;
			case 3:	return (p[2] >> 10) & 0x3ff;
			case 4:	return (p[3] >>  0) & 0x3ff;
			case 5:	return (p[3] >> 20) & 0x3ff;
		}
	}

	uint32 SampleV210_Cb(const void *src, ptrdiff_t srcpitch, sint32 x, sint32 y, uint32 w, uint32 h) {
		if (x < 0)
			x = 0;
		if ((uint32)x >= w)
			x = w - 1;
		if (y < 0)
			y = 0;
		if ((uint32)y >= h)
			y = h - 1;

		const uint32 *p = (const uint32 *)((const char *)src + srcpitch*y) + (x / 3)*4;

		switch((uint32)x % 3) {
			default:
			case 0:	return (p[0] >>  0) & 0x3ff;
			case 1:	return (p[1] >> 10) & 0x3ff;
			case 2:	return (p[2] >> 20) & 0x3ff;
		}
	}

	uint32 SampleV210_Cr(const void *src, ptrdiff_t srcpitch, sint32 x, sint32 y, uint32 w, uint32 h) {
		if (x < 0)
			x = 0;
		if ((uint32)x >= w)
			x = w - 1;
		if (y < 0)
			y = 0;
		if ((uint32)y >= h)
			y = h - 1;

		const uint32 *p = (const uint32 *)((const char *)src + srcpitch*y) + (x / 3)*4;

		switch((uint32)x % 3) {
			default:
			case 0:	return (p[0] >> 20) & 0x3ff;
			case 1:	return (p[2] >>  0) & 0x3ff;
			case 2:	return (p[3] >> 10) & 0x3ff;
		}
	}
}

uint32 VDPixmapInterpolateSampleRGB24(const VDPixmap& px, sint32 x_256, sint32 y_256) {
	switch(px.format) {
		case nsVDPixmap::kPixFormat_Pal1:
		case nsVDPixmap::kPixFormat_Pal2:
		case nsVDPixmap::kPixFormat_Pal4:
		case nsVDPixmap::kPixFormat_Pal8:
		case nsVDPixmap::kPixFormat_RGB565:
		case nsVDPixmap::kPixFormat_RGB888:
		case nsVDPixmap::kPixFormat_XRGB1555:
		case nsVDPixmap::kPixFormat_XRGB8888:
			{
				x_256 -= 128;
				y_256 -= 128;
				int ix = x_256 >> 8;
				int iy = y_256 >> 8;
				uint32 p0 = VDPixmapSample(px, ix, iy);
				uint32 p1 = VDPixmapSample(px, ix+1, iy);
				uint32 p2 = VDPixmapSample(px, ix, iy+1);
				uint32 p3 = VDPixmapSample(px, ix+1, iy+1);

				return Lerp8888(p0, p1, p2, p3, x_256 & 255, y_256 & 255);
			}
			break;

		case nsVDPixmap::kPixFormat_Y8:
			return InterpPlanarY8(px, x_256, y_256); 

		case nsVDPixmap::kPixFormat_YUV422_UYVY:
			return ConvertYCC72ToRGB24(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 1, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 0, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 2, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_YUYV:
			return ConvertYCC72ToRGB24(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 0, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 1, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 3, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_UYVY_FR:
			return ConvertYCC72ToRGB24_FR(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 1, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 0, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 2, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_YUYV_FR:
			return ConvertYCC72ToRGB24_FR(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 0, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 1, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 3, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV444_XVYU:
			return ConvertYCC72ToRGB24(
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 1, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 0, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 2, px.pitch, px.w, px.h, x_256, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_UYVY_709:
			return ConvertYCC72ToRGB24_709(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 1, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 0, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 2, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_YUYV_709:
			return ConvertYCC72ToRGB24_709(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 0, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 1, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 3, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_UYVY_709_FR:
			return ConvertYCC72ToRGB24_709_FR(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 1, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 0, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 2, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV422_YUYV_709_FR:
			return ConvertYCC72ToRGB24_709_FR(
					VDPixmapInterpolateSample8x2To24((const char *)px.data + 0, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 1, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256),
					VDPixmapInterpolateSample8x4To24((const char *)px.data + 3, px.pitch, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256)
				);

		case nsVDPixmap::kPixFormat_YUV420_NV12:
			return ConvertYCC72ToRGB24(
					VDPixmapInterpolateSample8To24(px.data, px.pitch, px.w, px.h, x_256, y_256),
					VDPixmapInterpolateSample8x2To24((const char *)px.data2 + 0, px.pitch2, (px.w + 1) >> 1, (px.h + 1) >> 1, (x_256 >> 1) + 128, y_256 >> 1),
					VDPixmapInterpolateSample8x2To24((const char *)px.data2 + 1, px.pitch2, (px.w + 1) >> 1, (px.h + 1) >> 1, (x_256 >> 1) + 128, y_256 >> 1)
				);

		case nsVDPixmap::kPixFormat_YUV444_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, x_256, y_256, px.w, px.h);

		case nsVDPixmap::kPixFormat_YUV422_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 1) + 64, y_256, (px.w + 1) >> 1, px.h);

		case nsVDPixmap::kPixFormat_YUV411_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 2) + 96, y_256, (px.w + 3) >> 2, px.h);

		case nsVDPixmap::kPixFormat_YUV420_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 1) + 64, y_256 >> 1, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420it_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) + 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420ib_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) - 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV410_Planar:
			return InterpPlanarYCC888(px, x_256, y_256, (x_256 >> 2) + 96, y_256 >> 2, (px.w + 3) >> 2, (px.h + 3) >> 2);

		case nsVDPixmap::kPixFormat_YUV420_Planar_Centered:
			return InterpPlanarYCC888(px, x_256, y_256, x_256 >> 1, y_256 >> 1, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV422_Planar_Centered:
			return InterpPlanarYCC888(px, x_256, y_256, x_256 >> 1, y_256, (px.w + 1) >> 1, px.h);

		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, x_256, y_256, px.w, px.h);

		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 1) + 64, y_256, (px.w + 1) >> 1, px.h);

		case nsVDPixmap::kPixFormat_YUV411_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 2) + 96, y_256, (px.w + 3) >> 2, px.h);

		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 1) + 64, y_256 >> 1, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420it_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) + 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420ib_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) - 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV410_Planar_709:
			return InterpPlanarYCC888_709(px, x_256, y_256, (x_256 >> 2) + 96, y_256 >> 2, (px.w + 3) >> 2, (px.h + 3) >> 2);

		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, x_256, y_256, px.w, px.h);

		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 1) + 64, y_256, (px.w + 1) >> 1, px.h);

		case nsVDPixmap::kPixFormat_YUV411_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 2) + 96, y_256, (px.w + 3) >> 2, px.h);

		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 1) + 64, y_256 >> 1, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420it_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) + 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420ib_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) - 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
			return InterpPlanarYCC888_FR(px, x_256, y_256, (x_256 >> 2) + 96, y_256 >> 2, (px.w + 3) >> 2, (px.h + 3) >> 2);

		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, x_256, y_256, px.w, px.h);

		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 1) + 64, y_256, (px.w + 1) >> 1, px.h);

		case nsVDPixmap::kPixFormat_YUV411_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 2) + 96, y_256, (px.w + 3) >> 2, px.h);

		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 1) + 64, y_256 >> 1, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420it_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) + 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV420ib_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 1) + 64, (y_256 >> 1) - 32, (px.w + 1) >> 1, (px.h + 1) >> 1);

		case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
			return InterpPlanarYCC888_709_FR(px, x_256, y_256, (x_256 >> 2) + 96, y_256 >> 2, (px.w + 3) >> 2, (px.h + 3) >> 2);

		case nsVDPixmap::kPixFormat_YUV420i_Planar:
			return InterpPlanarYCC888_420i<ConvertYCC72ToRGB24       >(px, x_256, y_256);

		case nsVDPixmap::kPixFormat_YUV420i_Planar_FR:
			return InterpPlanarYCC888_420i<ConvertYCC72ToRGB24_FR    >(px, x_256, y_256);

		case nsVDPixmap::kPixFormat_YUV420i_Planar_709:
			return InterpPlanarYCC888_420i<ConvertYCC72ToRGB24_709   >(px, x_256, y_256);

		case nsVDPixmap::kPixFormat_YUV420i_Planar_709_FR:
			return InterpPlanarYCC888_420i<ConvertYCC72ToRGB24_709_FR>(px, x_256, y_256);

		case nsVDPixmap::kPixFormat_YUV422_Planar_16F:
			{
				float y  = VDPixmapInterpolateSample16F(px.data, px.pitch, px.w, px.h, x_256, y_256);
				float cb = VDPixmapInterpolateSample16F(px.data2, px.pitch2, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256);
				float cr = VDPixmapInterpolateSample16F(px.data3, px.pitch3, (px.w + 1) >> 1, px.h, (x_256 >> 1) + 128, y_256);

				uint32 ir = VDClampedRoundFixedToUint8Fast(1.1643836f*y + 1.5960268f*cr - (222.92157f / 255.0f));
				uint32 ig = VDClampedRoundFixedToUint8Fast(1.1643836f*y - 0.3917623f*cb - 0.8129676f*cr + (135.57529f / 255.0f));
				uint32 ib = VDClampedRoundFixedToUint8Fast(1.1643836f*y + 2.0172321f*cb - (276.83585f / 255.0f));

				return (ir << 16) + (ig << 8) + ib;
			}

		case nsVDPixmap::kPixFormat_YUV422_V210:
			{
				sint32 luma_x = x_256 - 128;
				sint32 luma_y = y_256 - 128;

				if (luma_x < 0)
					luma_x = 0;

				if (luma_y < 0)
					luma_y = 0;

				if (luma_x > (sint32)((px.w - 1) << 8))
					luma_x = (sint32)((px.w - 1) << 8);

				if (luma_y > (sint32)((px.h - 1) << 8))
					luma_y = (sint32)((px.h - 1) << 8);

				sint32 luma_ix = luma_x >> 8;
				sint32 luma_iy = luma_y >> 8;
				float luma_fx = (float)(luma_x & 255) * (1.0f / 255.0f);
				float luma_fy = (float)(luma_y & 255) * (1.0f / 255.0f);

				float y0 = SampleV210_Y(px.data, px.pitch, luma_ix+0, luma_iy+0, px.w, px.h) * (1.0f / 1023.0f);
				float y1 = SampleV210_Y(px.data, px.pitch, luma_ix+1, luma_iy+0, px.w, px.h) * (1.0f / 1023.0f);
				float y2 = SampleV210_Y(px.data, px.pitch, luma_ix+0, luma_iy+1, px.w, px.h) * (1.0f / 1023.0f);
				float y3 = SampleV210_Y(px.data, px.pitch, luma_ix+1, luma_iy+1, px.w, px.h) * (1.0f / 1023.0f);
				float yt = y0 + (y1 - y0)*luma_fx;
				float yb = y2 + (y3 - y2)*luma_fx;
				float yr = yt + (yb - yt)*luma_fy;

				uint32 chroma_w = (px.w + 1) >> 1;
				uint32 chroma_h = px.h;
				sint32 chroma_x = x_256 >> 1;
				sint32 chroma_y = y_256 - 128;

				if (chroma_x < 0)
					chroma_x = 0;

				if (chroma_y < 0)
					chroma_y = 0;

				if (chroma_x > (sint32)((chroma_w - 1) << 8))
					chroma_x = (sint32)((chroma_w - 1) << 8);

				if (chroma_y > (sint32)((chroma_h - 1) << 8))
					chroma_y = (sint32)((chroma_h - 1) << 8);

				sint32 chroma_ix = chroma_x >> 8;
				sint32 chroma_iy = chroma_y >> 8;
				float chroma_fx = (float)(chroma_x & 255) * (1.0f / 255.0f);
				float chroma_fy = (float)(chroma_y & 255) * (1.0f / 255.0f);

				float cb0 = SampleV210_Cb(px.data, px.pitch, chroma_ix+0, chroma_iy+0, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cb1 = SampleV210_Cb(px.data, px.pitch, chroma_ix+1, chroma_iy+0, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cb2 = SampleV210_Cb(px.data, px.pitch, chroma_ix+0, chroma_iy+1, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cb3 = SampleV210_Cb(px.data, px.pitch, chroma_ix+1, chroma_iy+1, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cbt = cb0 + (cb1 - cb0)*chroma_fx;
				float cbb = cb2 + (cb3 - cb2)*chroma_fx;
				float cbr = cbt + (cbb - cbt)*chroma_fy;

				float cr0 = SampleV210_Cr(px.data, px.pitch, chroma_ix+0, chroma_iy+0, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cr1 = SampleV210_Cr(px.data, px.pitch, chroma_ix+1, chroma_iy+0, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cr2 = SampleV210_Cr(px.data, px.pitch, chroma_ix+0, chroma_iy+1, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float cr3 = SampleV210_Cr(px.data, px.pitch, chroma_ix+1, chroma_iy+1, chroma_w, chroma_h) * (1.0f / 1023.0f);
				float crt = cr0 + (cr1 - cr0)*chroma_fx;
				float crb = cr2 + (cr3 - cr2)*chroma_fx;
				float crr = crt + (crb - crt)*chroma_fy;

				uint32 ir = VDClampedRoundFixedToUint8Fast(1.1643836f*yr + 1.5960268f*crr - (222.92157f / 255.0f));
				uint32 ig = VDClampedRoundFixedToUint8Fast(1.1643836f*yr - 0.3917623f*cbr - 0.8129676f*crr + (135.57529f / 255.0f));
				uint32 ib = VDClampedRoundFixedToUint8Fast(1.1643836f*yr + 2.0172321f*cbr - (276.83585f / 255.0f));

				return (ir << 16) + (ig << 8) + ib;
			}
			break;

		default:
			return 0;
	}
}

uint32 VDConvertYCbCrToRGB(uint8 y0, uint8 cb0, uint8 cr0, bool use709, bool useFullRange) {
	sint32  y =  y0;
	sint32 cb = cb0 - 128;
	sint32 cr = cr0 - 128;
	sint32 r;
	sint32 g;
	sint32 b;

	if (use709) {
		if (useFullRange) {
			sint32 y2 = (y << 16) + 0x8000;
			r = y2 + cr * 103206;
			g = y2 + cr * -30679 + cb * -12276;
			b = y2 + cb * 121609;
		} else {
			sint32 y2 = (y - 16) * 76309 + 0x8000;
			r = y2 + cr * 117489;
			g = y2 + cr * -34925 + cb * -13975;
			b = y2 + cb * 138438;
		}
	} else {
		if (useFullRange) {
			sint32 y2 = (y << 16) + 0x8000;
			r = y2 + cr * 91181;
			g = y2 + cr * -46802 + cb * -22554;
			b = y2 + cb * 166130;
		} else {
			sint32 y2 = (y - 16) * 76309 + 0x8000;
			r = y2 + cr * 104597;
			g = y2 + cr * -53279 + cb * -25674;
			b = y2 + cb * 132201;
		}
	}

	r &= ~(r >> 31);
	g &= ~(g >> 31);
	b &= ~(b >> 31);
	r += (0xffffff - r) & ((0xffffff - r) >> 31);
	g += (0xffffff - g) & ((0xffffff - g) >> 31);
	b += (0xffffff - b) & ((0xffffff - b) >> 31);

	return (r & 0xff0000) + ((g & 0xff0000) >> 8) + (b >> 16);
}

uint32 VDConvertRGBToYCbCr(uint32 c) {
	return VDConvertRGBToYCbCr((uint8)(c >> 16), (uint8)(c >> 8), (uint8)c, false, false);
}

uint32 VDConvertRGBToYCbCr(uint8 r8, uint8 g8, uint8 b8, bool use709, bool useFullRange) {
	sint32 r  = r8;
	sint32 g  = g8;
	sint32 b  = b8;
	sint32 y;
	sint32 cb;
	sint32 cr;

	if (use709) {
		if (useFullRange) {
			y  = ( 13933*r + 46871*g +  4732*b +   0x8000) >> 8;
			cb = ( -7509*r - 25259*g + 32768*b + 0x808000) >> 16;
			cr = ( 32768*r - 29763*g -  3005*b + 0x808000);
		} else {
			y =  ( 11966*r + 40254*g +  4064*b + 0x108000) >> 8;
			cb = ( -6596*r - 22189*g + 28784*b + 0x808000) >> 16;
			cr = ( 28784*r - 26145*g -  2639*b + 0x808000);
		}
	} else {
		if (useFullRange) {
			y =  ( 19595*r + 38470*g +  7471*b +   0x8000) >> 8;
			cb = (-11058*r - 21710*g + 32768*b + 0x808000) >> 16;
			cr = ( 32768*r - 27439*g -  5329*b + 0x808000);
		} else {
			y  = ( 16829*r + 33039*g +  6416*b + 0x108000) >> 8;
			cb = ( -9714*r - 19071*g + 28784*b + 0x808000) >> 16;
			cr = ( 28784*r - 24103*g -  4681*b + 0x808000);
		}
	}

	return (uint8)cb + (y & 0xff00) + (cr&0xff0000);
}
