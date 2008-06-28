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

#include "StringMap.h"
#include "Glyph.h"

namespace ssf
{
	template<class T>
	class Cache
	{
	protected:
		StringMapW<T> m_key2obj;
		CAtlList<CStringW> m_objs;
		size_t m_limit;

	public:
		Cache(size_t limit) {m_limit = max(1, limit);}
		virtual ~Cache() {RemoveAll();}

		void RemoveAll()
		{
			POSITION pos = m_key2obj.GetStartPosition();
			while(pos) delete m_key2obj.GetNextValue(pos);
			m_key2obj.RemoveAll();
			m_objs.RemoveAll();
		}

		void Add(const CStringW& key, T& obj, bool fFlush = true)
		{
			if(StringMapW<T>::CPair* p = m_key2obj.Lookup(key)) delete p->m_value;
			else m_objs.AddTail(key);

			m_key2obj[key] = obj;

			if(fFlush) Flush();
		}

		void Flush()
		{
			while(m_objs.GetCount() > m_limit)
			{
				CStringW key = m_objs.RemoveHead();
				ASSERT(m_key2obj.Lookup(key));
				delete m_key2obj[key];
				m_key2obj.RemoveKey(key);
			}
		}

		bool Lookup(const CStringW& key, T& val)
		{
			return m_key2obj.Lookup(key, val) && val;
		}

		void Invalidate(const CStringW& key)
		{
			T val;
			if(m_key2obj.Lookup(key, val))
			{
				delete val;
				m_key2obj[key] = NULL;
			}
		}
	};

	class FontCache : public Cache<FontWrapper*> 
	{
	public:
		FontCache() : Cache(20) {}
		FontWrapper* Create(HDC hDC, const LOGFONT& lf);
	};

	class GlyphPathCache : public Cache<GlyphPath*>
	{
	public:
		GlyphPathCache() : Cache(100) {}
		GlyphPath* Create(HDC hDC, const FontWrapper* f, WCHAR c);
	};

	class Row : public CAutoPtrList<Glyph>
	{
	public:
		int ascent, descent, border, width;
		Row() {ascent = descent = border = width = 0;}
	};

	class RenderedSubtitle
	{
	public:
		CRect m_spdrc;
		CRect m_clip;
		CAutoPtrList<Glyph> m_glyphs;

		RenderedSubtitle(const CRect& spdrc, const CRect& clip) : m_spdrc(spdrc), m_clip(clip) {}
		virtual ~RenderedSubtitle() {}

		CRect Draw(SubPicDesc& spd) const;
	};

	class RenderedSubtitleCache : public Cache<RenderedSubtitle*>
	{
	public:
		RenderedSubtitleCache() : Cache(10) {}
	};

	class SubRect
	{
	public:
		CRect rect;
		float layer;
		SubRect() {}
		SubRect(const CRect& r, float l) : rect(r), layer(l) {}
	};

	class SubRectAllocator : public StringMapW<SubRect>
	{
		CSize vs;
		CRect vr;
	public:
		void UpdateTarget(const CSize& vs, const CRect& vr);
		void GetRect(CRect& rect, const Subtitle* s, const Align& align, int tlb, int brb);
	};

	class Renderer
	{
		HDC m_hDC;

		FontCache m_fc;
		GlyphPathCache m_gpc;
		RenderedSubtitleCache m_rsc;
		SubRectAllocator m_sra;

	public:
		Renderer();
		virtual ~Renderer();

		void NextSegment(const CAutoPtrList<Subtitle>& subs);
		RenderedSubtitle* Lookup(const Subtitle* s, const CSize& vs, const CRect& vr);
	};
}