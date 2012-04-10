/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <atlcoll.h>
#include "SubtitleFile.h"
#include "Array.h"
#include "GlyphPath.h"
#include "../../SubPic/ISubPic.h"

namespace ssf
{
	class Rasterizer
	{
		bool fFirstSet;
		CPoint firstp, lastp;

	private:
		int mWidth, mHeight;

		union Span {
			struct {
				int x1, y1, x2, y2;
			};
			struct {
				unsigned __int64 first, second;
			};
			union Span() {}
			union Span(int _x1, int _y1, int _x2, int _y2) {
				x1 = _x1;
				y1 = _y1;
				x2 = _x2;
				y2 = _y2;
			}
			union Span(unsigned __int64 _first, unsigned __int64 _second) {
				first = _first;
				second = _second;
			}
		};

		Array<Span> mOutline, mWideOutline, mWideOutlineTmp;
		int mWideBorder;

		struct Edge {
			int next, posandflag;
		}* mpEdgeBuffer;
		unsigned int mEdgeHeapSize;
		unsigned int mEdgeNext;
		unsigned int* mpScanBuffer;

	protected:
		BYTE* mpOverlayBuffer;
		int mOverlayWidth, mOverlayHeight;
		int mPathOffsetX, mPathOffsetY;
		int mOffsetX, mOffsetY;

	private:
		void _TrashOverlay();
		void _ReallocEdgeBuffer(int edges);
		void _EvaluateBezier(const CPoint& p0, const CPoint& p1, const CPoint& p2, const CPoint& p3);
		void _EvaluateLine(CPoint p0, CPoint p1);
		void _OverlapRegion(Array<Span>& dst, Array<Span>& src, int dx, int dy);

	public:
		Rasterizer();
		virtual ~Rasterizer();

		bool ScanConvert(GlyphPath& path, const CRect& bbox);
		bool CreateWidenedRegion(int r);
		bool Rasterize(int xsub, int ysub);
		void Reuse(Rasterizer& r);

		void Blur(float n, int plane);
		CRect Draw(const SubPicDesc& spd, const CRect& clip, int xsub, int ysub, const DWORD* switchpts, int plane);
	};
}