/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlcoll.h>
#include "GlyphPath.h"
#include "FontWrapper.h"
#include "Rasterizer.h"

namespace ssf
{
	class Glyph
	{
		void Transform(GlyphPath& path, CPoint org, const CRect& subrect);

		struct SplineCoeffs {float cx[4], cy[4];};

	public:
		WCHAR c;
		Style style;
		CAtlArray<SplineCoeffs> spline;
		Size scale;
		bool vertical;
		FontWrapper* font;
		int ascent, descent, width, spacing, fill;
		int row_ascent, row_descent;
		GlyphPath path, path_bkg;
		CRect bbox;
		CPoint tl, tls;
		Rasterizer ras, ras_bkg, ras_shadow;

	public:
		Glyph();

		void CreateBkg();
		void CreateSplineCoeffs(const CRect& spdrc);
		void Transform(CPoint org, const CRect& subrect);
		void Rasterize();

		float GetBackgroundSize() const;
		float GetShadowDepth() const;
		CRect GetClipRect() const;
	};
}