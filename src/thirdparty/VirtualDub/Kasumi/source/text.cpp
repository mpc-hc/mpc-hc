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
#include <vd2/Kasumi/region.h>
#include <vd2/Kasumi/text.h>

#include "defaultfont.inl"

void VDPixmapGetTextExtents(const VDOutlineFontInfo *font, float size, const char *pText, VDTextLayoutMetrics& out_Metrics) {
	if (!font)
		font = &g_VDDefaultFont_FontInfo;

	float invEmSquare = 1.0f / (float)font->mEmSquare;
	float scale = 1.0f / (255.0f * 65536.0f);
	float xscale = (float)(font->mMaxX - font->mMinX) * scale;
	float yscale = (float)(font->mMaxY - font->mMinY) * scale;

	float xinitstep = font->mMinX * 255.0f * scale;
	float yinitstep = -(font->mMaxY * scale + font->mDescent);

	float xoffset = xinitstep;
	float yoffset = yinitstep;

	vdrect32f bounds;
	bounds.invalidate();

	while(const char c = *pText++) {
		int index = (unsigned char)c - font->mStartGlyph;

		if ((unsigned)index >= (unsigned)(font->mEndGlyph - font->mStartGlyph))
			continue;

		const VDOutlineFontGlyphInfo& glyph = font->mpGlyphArray[index];
		const VDOutlineFontGlyphInfo& glyphNext = font->mpGlyphArray[index + 1];
		int nPoints = glyphNext.mPointArrayStart - glyph.mPointArrayStart;

		if (nPoints) {
			vdrect32 localBounds;
			localBounds.invalidate();

			const uint16 *pPoints = font->mpPointArray + glyph.mPointArrayStart;
			for(int i=0; i<nPoints; ++i) {
				uint16 pt = *pPoints++;

				localBounds.add(pt & 255, pt >> 8);
			}

			vdrect32f localBoundsF((float)localBounds.left, -(float)localBounds.bottom, (float)localBounds.right, -(float)localBounds.top);
			localBoundsF.scale(xscale, yscale);
			localBoundsF.translate(xoffset, yoffset);
			bounds.add(localBoundsF);
		}

		xoffset += glyph.mAWidth + glyph.mBWidth + glyph.mCWidth;
	}

	if (bounds.valid())
		bounds.scale(size * invEmSquare, size * invEmSquare);

	out_Metrics.mExtents = bounds;
	out_Metrics.mAdvance = (xoffset - xinitstep) * size * invEmSquare;
}

void VDPixmapConvertTextToPath(VDPixmapPathRasterizer& rast, const VDOutlineFontInfo *pFont, float size, float x, float y, const char *pText, const float transform[2][2]) {
	if (!pFont)
		pFont = &g_VDDefaultFont_FontInfo;

	vdfastfixedvector<vdint2, 256> points;

	float scale = size / ((float)pFont->mEmSquare * 255.0f * 65536.0f);
	float xscale = (float)(pFont->mMaxX - pFont->mMinX) * scale;
	float yscale = -(float)(pFont->mMaxY - pFont->mMinY) * scale;

	float xinitstep = pFont->mMinX * 255.0f * scale;
	float yinitstep = -pFont->mMaxY * scale - pFont->mDescent * size / (float)pFont->mEmSquare;

	static const float kIdentity[2][2]={1,0,0,1};

	if (!transform)
		transform = kIdentity;

	float xoffset = x + xinitstep * transform[0][0] + yinitstep * transform[0][1];
	float yoffset = y + xinitstep * transform[1][0] + yinitstep * transform[1][1];

	while(const char c = *pText++) {
		int index = (unsigned char)c - pFont->mStartGlyph;

		if ((unsigned)index >= (unsigned)(pFont->mEndGlyph - pFont->mStartGlyph))
			continue;

		const VDOutlineFontGlyphInfo& glyph = pFont->mpGlyphArray[index];
		const VDOutlineFontGlyphInfo& glyphNext = pFont->mpGlyphArray[index + 1];
		const uint16 *pPoints = pFont->mpPointArray + glyph.mPointArrayStart;
		const uint8 *pCommands = pFont->mpCommandArray + glyph.mCommandArrayStart;
		int nPoints = glyphNext.mPointArrayStart - glyph.mPointArrayStart;
		int nCommands = glyphNext.mCommandArrayStart - glyph.mCommandArrayStart;

		points.clear();
		points.resize(nPoints);

		for(int i=0; i<nPoints; ++i) {
			uint16 pt = *pPoints++;
			float fx1 = (pt & 255) * xscale;
			float fy1 = (pt >> 8) * yscale;
			points[i].set(VDRoundToInt(fx1*transform[0][0] + fy1*transform[0][1] + xoffset), VDRoundToInt(fx1*transform[1][0] + fy1*transform[1][1] + yoffset));
		}

		const vdint2 *srcpt = points.data();
		const vdint2 *startpt = points.data();

		while(nCommands--) {
			uint8 cmd = *pCommands++;
			int countm1 = (cmd & 0x7f) >> 2;

			for(int i=0; i<=countm1; ++i) {
				switch(cmd & 3) {
				case 2:
					rast.Line(srcpt[0], srcpt[1]);
					++srcpt;
					break;

				case 3:
					rast.QuadraticBezier(srcpt);
					srcpt += 2;
					break;
				}
			}

			if (cmd & 0x80) {
				rast.Line(*srcpt, *startpt);
				startpt = ++srcpt;
			}
		}

		float step = (glyph.mAWidth + glyph.mBWidth + glyph.mCWidth) * (size / (float)pFont->mEmSquare);
		xoffset += step * transform[0][0];
		yoffset += step * transform[1][0];
	}
}

namespace {
	void Fill(const VDPixmap& pxdst, int x, int y, int w, int h, uint32 c) {
		if (x >= pxdst.w || y >= pxdst.h)
			return;

		if (x < 0) {
			w += x;
			x = 0;
		}

		if (y < 0) {
			h += y;
			y = 0;
		}

		if (w > pxdst.w - x)
			w = pxdst.w - x;

		if (h > pxdst.h - y)
			h = pxdst.h - y;

		if (w <= 0 || h <= 0)
			return;

		switch(pxdst.format) {
			case nsVDPixmap::kPixFormat_Pal8:
				VDMemset8Rect((uint8 *)pxdst.data + pxdst.pitch * y + x, pxdst.pitch, (uint8)c, w, h);
				break;

			case nsVDPixmap::kPixFormat_XRGB8888:
				VDMemset32Rect((uint8 *)pxdst.data + pxdst.pitch * y + x * 4, pxdst.pitch, c, w, h);
				break;
		}
	}
}

void VDPixmapDrawText(const VDPixmap& pxdst, const VDBitmapFontInfo *font, int x, int y, uint32 fore, uint32 back, const char *pText) {
	if (pxdst.format != nsVDPixmap::kPixFormat_Pal8 && pxdst.format != nsVDPixmap::kPixFormat_XRGB8888)
		return;

	int textWidth = 0;

	for(const char *s = pText; *s; ++s) {
		uint8 c = *s;

		if (c < font->mStartChar || c > font->mEndChar)
			textWidth += font->mCellWidth;
		else
			textWidth += font->mpPosArray[c - font->mStartChar] & 15;

		textWidth += font->mCellAdvance;
	}

	Fill(pxdst, x, y, textWidth + font->mCellAdvance, font->mCellHeight + 1, back);

	x += font->mCellAdvance;
	++y;

	if (x >= pxdst.w || y >= pxdst.h)
		return;

	if (font->mStartChar > font->mEndChar)
		return;

	const uint8 *bits = font->mpBitsArray;
	while(uint8 c = (uint8)*pText++) {
		if (x >= pxdst.w)
			break;

		if (c < font->mStartChar || c > font->mEndChar) {
			x += font->mCellWidth;
			x += font->mCellAdvance;
			continue;
		}

		uint32 posInfo = font->mpPosArray[c - font->mStartChar];
		int cx = x;
		int cw = posInfo & 15;
		int cy = y;
		int ch = font->mCellHeight;

		x += cw;
		x += font->mCellAdvance;

		if (posInfo < 16)
			continue;

		uint32 bitOffset = ((posInfo >> 4) - 1) * font->mCellHeight;

		if (cy < 0) {
			if (cy < -ch)
				continue;

			ch += cy;
			bitOffset += -cy * font->mCellWidth;
		}

		if (cy + ch > pxdst.h)
			ch = pxdst.h - cy;

		int cskip = 0;
		if (cx < 0) {
			if (cx < -(int)pxdst.w)
				continue;

			bitOffset += -cx;
			cskip += -cx;
			cx = 0;
		}

		if (cx + cw > pxdst.w) {
			cskip += cx + cw - pxdst.w;
			cw = pxdst.w - cx;
		}

		uint8 *dstrow = (uint8 *)pxdst.data + pxdst.pitch * cy;
		for(int yi = 0; yi < ch; ++yi) {
			if (pxdst.format == nsVDPixmap::kPixFormat_Pal8) {
				uint8 *dst8 = dstrow + cx;
				for(int xi = 0; xi < cw; ++xi) {
					if ((sint8)(bits[bitOffset >> 3] << (bitOffset & 7)) < 0)
						*dst8 = (uint8)fore;

					++dst8;
					++bitOffset;
				}
			} else if (pxdst.format == nsVDPixmap::kPixFormat_XRGB8888) {
				uint32 *dst32 = (uint32 *)dstrow + cx;
				for(int xi = 0; xi < cw; ++xi) {
					if ((sint8)(bits[bitOffset >> 3] << (bitOffset & 7)) < 0)
						*dst32 = fore;

					++dst32;
					++bitOffset;
				}
			}

			bitOffset += cskip;
			dstrow += pxdst.pitch;
		}
	}
}
