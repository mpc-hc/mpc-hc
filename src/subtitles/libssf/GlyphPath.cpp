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

#include "stdafx.h"
#include "GlyphPath.h"

namespace ssf
{
	GlyphPath::GlyphPath(const GlyphPath& path)
	{
		*this = path;
	}

	void GlyphPath::operator = (const GlyphPath& path)
	{
		types.Copy(path.types);
		points.Copy(path.points);
	}

	bool GlyphPath::IsEmpty()
	{
		return types.IsEmpty() || points.IsEmpty();
	}

	void GlyphPath::RemoveAll()
	{
		types.RemoveAll();
		points.RemoveAll();
	}

	void GlyphPath::MovePoints(const CPoint& o)
	{
		size_t n = points.GetCount();
		POINT* p = points.GetData();

		unsigned int i = 0;

		if(!!(g_cpuid.m_flags & CCpuID::sse2) && !((DWORD_PTR)p & 7))
		{
			for( ; i < n && ((DWORD_PTR)&p[i] & 15); i++)
			{
				p[i].x += o.x;
				p[i].y += o.y;
			}

			__m128i oo = _mm_set_epi32(o.y, o.x, o.y, o.x);

			for(unsigned int j = i + ((n - i) & ~7); i < j; i += 8)
			{
				__m128i r0 = _mm_load_si128((__m128i*)&p[i+0]);
				__m128i r1 = _mm_load_si128((__m128i*)&p[i+2]);
				__m128i r2 = _mm_load_si128((__m128i*)&p[i+4]);
				__m128i r3 = _mm_load_si128((__m128i*)&p[i+6]);
				_mm_store_si128((__m128i*)&p[i+0], _mm_add_epi32(r0, oo));
				_mm_store_si128((__m128i*)&p[i+2], _mm_add_epi32(r1, oo));
				_mm_store_si128((__m128i*)&p[i+4], _mm_add_epi32(r2, oo));
				_mm_store_si128((__m128i*)&p[i+6], _mm_add_epi32(r3, oo));
			}
		}

		for(; i < n; i++)
		{
			p[i].x += o.x;
			p[i].y += o.y;
		}
	}

	void GlyphPath::Enlarge(const GlyphPath& src, float size)
	{
		types.SetCount(src.types.GetCount());
		points.SetCount(src.points.GetCount());

		memcpy(types.GetData(), src.types.GetData(), types.GetCount());

		size_t start = 0, end = 0;

		for(size_t i = 0, j = src.types.GetCount(); i <= j; i++)
		{
			if(i > 0 && (i == j || (src.types[i] & ~PT_CLOSEFIGURE) == PT_MOVETO))
			{
				end = i-1;

				bool cw = true; // TODO: determine orientation (ttf is always cw and we are sill before Transform)
				float rotate = cw ? -M_PI_2 : M_PI_2;

				CPoint prev = src.points[end];
				CPoint cur = src.points[start];

				for(size_t k = start; k <= end; k++)
				{
					CPoint next = k < end ? src.points[k+1] : src.points[start];

					for(int l = k-1; prev == cur; l--)
					{
						if(l < (int)start) l = end;
						prev = src.points[l];
					}

					for(int l = k+1; next == cur; l++)
					{
						if(l > (int)end) l = start;
						next = src.points[l];
					}

					CPoint in = cur - prev;
					CPoint out = next - cur;

					float angle_in = atan2((float)in.y, (float)in.x);
					float angle_out = atan2((float)out.y, (float)out.x);
					float angle_diff = angle_out - angle_in;
					if(angle_diff < 0) angle_diff += M_PI*2;
					if(angle_diff > M_PI) angle_diff -= M_PI*2;
					float scale = cos(angle_diff / 2);

					CPoint p;

					if(angle_diff < 0)
					{
						if(angle_diff > -M_PI/8) {if(scale < 1) scale = 1;}
						else {if(scale < 0.50) scale = 0.50;}
					}
					else
					{
						if(angle_diff < M_PI/8) {if(scale < 0.75) scale = 0.75;}
						else {if(scale < 0.25) scale = 0.25;}
					}

					if(scale < 0.1) scale = 0.1;

					float angle = angle_in + angle_diff / 2 - rotate;
					float radius = -size / scale; // FIXME

					p.x = (int)(radius * cos(angle) + 0.5);
					p.y = (int)(radius * sin(angle) + 0.5);

					points[k] = cur + p;

					prev = cur;
					cur = next;
				}

				start = end+1;
			}
		}
	}
}