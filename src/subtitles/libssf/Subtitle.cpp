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
#include "Subtitle.h"
#include "Split.h"
#include <math.h>

namespace ssf
{
	struct Subtitle::n2n_t Subtitle::m_n2n;

	Subtitle::Subtitle(File* pFile) 
		: m_pFile(pFile)
	{
		if(m_n2n.align[0].IsEmpty())
		{
			m_n2n.align[0][_T("left")] = 0;
			m_n2n.align[0][_T("center")] = 0.5;
			m_n2n.align[0][_T("middle")] = 0.5;
			m_n2n.align[0][_T("right")] = 1;
		}
		
		if(m_n2n.align[1].IsEmpty())
		{
			m_n2n.align[1][_T("top")] = 0;
			m_n2n.align[1][_T("middle")] = 0.5;
			m_n2n.align[1][_T("center")] = 0.5;
			m_n2n.align[1][_T("bottom")] = 1;
		}
		
		if(m_n2n.weight.IsEmpty())
		{
			m_n2n.weight[_T("thin")] = FW_THIN;
			m_n2n.weight[_T("normal")] = FW_NORMAL;
			m_n2n.weight[_T("bold")] = FW_BOLD;
		}

		if(m_n2n.transition.IsEmpty())
		{
			m_n2n.transition[_T("start")] = 0;
			m_n2n.transition[_T("stop")] = 1e10;
			m_n2n.transition[_T("linear")] = 1;
		}
	}

	Subtitle::~Subtitle() 
	{
	}

	bool Subtitle::Parse(Definition* pDef, double start, double stop, double at)
	{
		ASSERT(m_pFile && pDef);

		m_text.RemoveAll();

		m_time.start = start;
		m_time.stop = stop;

		at -= start;

		Fill::gen_id = 0;

		try
		{
			Definition& frame = (*pDef)[_T("frame")];

			m_frame.reference = frame[_T("reference")];
			m_frame.resolution.cx = frame[_T("resolution")][_T("cx")];
			m_frame.resolution.cy = frame[_T("resolution")][_T("cy")];

			Definition& direction = (*pDef)[_T("direction")];

			m_direction.primary = direction[_T("primary")];
			m_direction.secondary = direction[_T("secondary")];

			m_wrap = (*pDef)[_T("wrap")];

			m_layer = (*pDef)[_T("layer")];

			m_pFile->Commit();

			Style style;
			GetStyle(&(*pDef)[_T("style")], style);

			CAtlStringMap<double> offset;
			Definition& block = (*pDef)[_T("@")];
			Parse(CharacterStream((LPCTSTR)block), style, at, offset, dynamic_cast<Reference*>(block.m_parent));

			m_pFile->Rollback();

			// TODO: trimming should be done by the renderer later, after breaking the words into lines

			while(!m_text.IsEmpty() && (m_text.GetHead().str == Text::SP || m_text.GetHead().str == Text::LSEP))
				m_text.RemoveHead();

			while(!m_text.IsEmpty() && (m_text.GetTail().str == Text::SP || m_text.GetTail().str == Text::LSEP))
				m_text.RemoveTail();
		}
		catch(Exception& e)
		{
			TRACE(_T("%s"), e.ToString());
			return false;
		}

		return true;
	}

	void Subtitle::GetStyle(Definition* pDef, Style& style)
	{
		style.placement.pos.x = 0;
		style.placement.pos.y = 0;
		style.placement.pos.auto_x = true;
		style.placement.pos.auto_y = true;

		Rect frame = {0, m_frame.resolution.cx, m_frame.resolution.cy, 0};

		style.placement.clip = frame;

		//

		style.linebreak = (*pDef)[_T("linebreak")];

		Definition& placement = (*pDef)[_T("placement")];

		Definition& clip = placement[_T("clip")];

		if(clip.IsValue(Definition::string))
		{
			CString str = clip;

			if(str == _T("frame")) style.placement.clip = frame;
			// else ?
		}
		else
		{
			if(clip[_T("t")].IsValue()) style.placement.clip.t = clip[_T("t")];
			if(clip[_T("r")].IsValue()) style.placement.clip.r = clip[_T("r")];
			if(clip[_T("b")].IsValue()) style.placement.clip.b = clip[_T("b")];
			if(clip[_T("l")].IsValue()) style.placement.clip.l = clip[_T("l")];
		}

		CAtlStringMap<double> n2n_margin;

		n2n_margin[_T("top")] = frame.t;
		n2n_margin[_T("right")] = frame.r;
		n2n_margin[_T("bottom")] = frame.b;
		n2n_margin[_T("left")] = frame.l;

		placement[_T("margin")][_T("t")].GetAsNumber(style.placement.margin.t, &n2n_margin);
		placement[_T("margin")][_T("r")].GetAsNumber(style.placement.margin.r, &n2n_margin);
		placement[_T("margin")][_T("b")].GetAsNumber(style.placement.margin.b, &n2n_margin);
		placement[_T("margin")][_T("l")].GetAsNumber(style.placement.margin.l, &n2n_margin);

		placement[_T("align")][_T("h")].GetAsNumber(style.placement.align.h, &m_n2n.align[0]);
		placement[_T("align")][_T("v")].GetAsNumber(style.placement.align.v, &m_n2n.align[1]);

		if(placement[_T("pos")][_T("x")].IsValue()) {style.placement.pos.x = placement[_T("pos")][_T("x")]; style.placement.pos.auto_x = false;}
		if(placement[_T("pos")][_T("y")].IsValue()) {style.placement.pos.y = placement[_T("pos")][_T("y")]; style.placement.pos.auto_y = false;}

		placement[_T("offset")][_T("x")].GetAsNumber(style.placement.offset.x);
		placement[_T("offset")][_T("y")].GetAsNumber(style.placement.offset.y);

		style.placement.angle.x = placement[_T("angle")][_T("x")];
		style.placement.angle.y = placement[_T("angle")][_T("y")];
		style.placement.angle.z = placement[_T("angle")][_T("z")];

		Definition& font = (*pDef)[_T("font")];

		style.font.face = font[_T("face")];
		style.font.size = font[_T("size")];
		font[_T("weight")].GetAsNumber(style.font.weight, &m_n2n.weight);
		style.font.color[0] = font[_T("color")][_T("a")];
		style.font.color[1] = font[_T("color")][_T("r")];
		style.font.color[2] = font[_T("color")][_T("g")];
		style.font.color[3] = font[_T("color")][_T("b")];
		style.font.underline = font[_T("underline")];
		style.font.strikethrough = font[_T("strikethrough")];
		style.font.italic = font[_T("italic")];
		style.font.spacing = font[_T("spacing")];
		style.font.scale.cx = font[_T("scale")][_T("cx")];
		style.font.scale.cy = font[_T("scale")][_T("cy")];

		Definition& background = (*pDef)[_T("background")];

		style.background.color[0] = background[_T("color")][_T("a")];
		style.background.color[1] = background[_T("color")][_T("r")];
		style.background.color[2] = background[_T("color")][_T("g")];
		style.background.color[3] = background[_T("color")][_T("b")];
		style.background.size = background[_T("size")];
		style.background.type = background[_T("type")];

		Definition& shadow = (*pDef)[_T("shadow")];

		style.shadow.color[0] = shadow[_T("color")][_T("a")];
		style.shadow.color[1] = shadow[_T("color")][_T("r")];
		style.shadow.color[2] = shadow[_T("color")][_T("g")];
		style.shadow.color[3] = shadow[_T("color")][_T("b")];
		style.shadow.depth = shadow[_T("depth")];
		style.shadow.angle = shadow[_T("angle")];
		style.shadow.blur = shadow[_T("blur")];

		Definition& fill = (*pDef)[_T("fill")];

		style.fill.color[0] = fill[_T("color")][_T("a")];
		style.fill.color[1] = fill[_T("color")][_T("r")];
		style.fill.color[2] = fill[_T("color")][_T("g")];
		style.fill.color[3] = fill[_T("color")][_T("b")];
		style.fill.width = fill[_T("width")];
	}

	double Subtitle::GetMixWeight(Definition* pDef, double at, CAtlStringMap<double>& offset, int default_id)
	{
		double t = 1;

		try
		{
			CAtlStringMap<double> n2n;

			n2n[_T("start")] = 0;
			n2n[_T("stop")] = m_time.stop - m_time.start;

			Definition::Time time;
			if(pDef->GetAsTime(time, offset, &n2n, default_id) && time.start.value < time.stop.value)
			{
				t = (at - time.start.value) / (time.stop.value - time.start.value);

				if(t < 0) t = 0;
				else if(t > 1) t = 1;

				if((*pDef)[_T("loop")].IsValue()) t *= (double)(*pDef)[_T("loop")];

				CString direction = (*pDef)[_T("direction")].IsValue() ? (*pDef)[_T("direction")] : _T("fw");
				if(direction == _T("fwbw") || direction == _T("bwfw")) t *= 2;

				double n;
				t = modf(t, &n);

				if(direction == _T("bw") 
				|| direction == _T("fwbw") && ((int)n & 1)
				|| direction == _T("bwfw") && !((int)n & 1)) 
					t = 1 - t;

				double accel = 1;

				if((*pDef)[_T("transition")].IsValue())
				{
					Definition::Number<double> n;
					(*pDef)[_T("transition")].GetAsNumber(n, &m_n2n.transition);
					if(n.value >= 0) accel = n.value;
				}

				t = accel == 0 ? 1 : 
					accel == 1 ? t : 
					accel >= 1e10 ? 0 :
					pow(t, accel);
			}
		}
		catch(Exception&)
		{
		}

		return t;
	}

	template<class T> 
	bool Subtitle::MixValue(Definition& def, T& value, double t)
	{
		CAtlStringMap<T> n2n;
		return MixValue(def, value, t, &n2n);
	}

	template<> 
	bool Subtitle::MixValue(Definition& def, double& value, double t)
	{
		CAtlStringMap<double> n2n;
		return MixValue(def, value, t, &n2n);
	}

	template<class T> 
	bool Subtitle::MixValue(Definition& def, T& value, double t, CAtlStringMap<T>* n2n)
	{
		if(!def.IsValue()) return false;

		if(t >= 0.5)
		{
			if(n2n && def.IsValue(Definition::string))
			{
				if(CAtlStringMap<T>::CPair* p = n2n->Lookup(def))
				{
					value = p->m_value;
					return true;
				}
			}

			value = (T)def;
		}

		return true;
	}

	template<> 
	bool Subtitle::MixValue(Definition& def, double& value, double t, CAtlStringMap<double>* n2n)
	{
		if(!def.IsValue()) return false;

		if(t > 0)
		{
			if(n2n && def.IsValue(Definition::string))
			{
				if(CAtlStringMap<double>::CPair* p = n2n->Lookup(def))
				{
					value += (p->m_value - value) * t;
					return true;
				}
			}

			value += ((double)def - value) * t;
		}

		return true;
	}

	void Subtitle::MixStyle(Definition* pDef, Style& dst, double t)
	{
		const Style src = dst;

		if(t <= 0) return;
		else if(t > 1) t = 1;

		MixValue((*pDef)[_T("linebreak")], dst.linebreak, t);

		Definition& placement = (*pDef)[_T("placement")];

		MixValue(placement[_T("clip")][_T("t")], dst.placement.clip.t, t);
		MixValue(placement[_T("clip")][_T("r")], dst.placement.clip.r, t);
		MixValue(placement[_T("clip")][_T("b")], dst.placement.clip.b, t);
		MixValue(placement[_T("clip")][_T("l")], dst.placement.clip.l, t);
		MixValue(placement[_T("align")][_T("h")], dst.placement.align.h, t, &m_n2n.align[0]);
		MixValue(placement[_T("align")][_T("v")], dst.placement.align.v, t, &m_n2n.align[1]);
		dst.placement.pos.auto_x = !MixValue(placement[_T("pos")][_T("x")], dst.placement.pos.x, dst.placement.pos.auto_x ? 1 : t);
		dst.placement.pos.auto_y = !MixValue(placement[_T("pos")][_T("y")], dst.placement.pos.y, dst.placement.pos.auto_y ? 1 : t);
		MixValue(placement[_T("offset")][_T("x")], dst.placement.offset.x, t);
		MixValue(placement[_T("offset")][_T("y")], dst.placement.offset.y, t);
		MixValue(placement[_T("angle")][_T("x")], dst.placement.angle.x, t);
		MixValue(placement[_T("angle")][_T("y")], dst.placement.angle.y, t);
		MixValue(placement[_T("angle")][_T("z")], dst.placement.angle.z, t);

		Definition& font = (*pDef)[_T("font")];

		MixValue(font[_T("face")], dst.font.face, t);
		MixValue(font[_T("size")], dst.font.size, t);
		MixValue(font[_T("weight")], dst.font.weight, t, &m_n2n.weight);
		MixValue(font[_T("color")][_T("a")], dst.font.color[0], t);
		MixValue(font[_T("color")][_T("r")], dst.font.color[1], t);
		MixValue(font[_T("color")][_T("g")], dst.font.color[2], t);
		MixValue(font[_T("color")][_T("b")], dst.font.color[3], t);
		MixValue(font[_T("underline")], dst.font.underline, t);
		MixValue(font[_T("strikethrough")], dst.font.strikethrough, t);
		MixValue(font[_T("italic")], dst.font.italic, t);
		MixValue(font[_T("spacing")], dst.font.spacing, t);
		MixValue(font[_T("scale")][_T("cx")], dst.font.scale.cx, t);
		MixValue(font[_T("scale")][_T("cy")], dst.font.scale.cy, t);

		Definition& background = (*pDef)[_T("background")];

		MixValue(background[_T("color")][_T("a")], dst.background.color[0], t);
		MixValue(background[_T("color")][_T("r")], dst.background.color[1], t);
		MixValue(background[_T("color")][_T("g")], dst.background.color[2], t);
		MixValue(background[_T("color")][_T("b")], dst.background.color[3], t);
		MixValue(background[_T("size")], dst.background.size, t);
		MixValue(background[_T("type")], dst.background.type, t);

		Definition& shadow = (*pDef)[_T("shadow")];

		MixValue(shadow[_T("color")][_T("a")], dst.shadow.color[0], t);
		MixValue(shadow[_T("color")][_T("r")], dst.shadow.color[1], t);
		MixValue(shadow[_T("color")][_T("g")], dst.shadow.color[2], t);
		MixValue(shadow[_T("color")][_T("b")], dst.shadow.color[3], t);
		MixValue(shadow[_T("depth")], dst.shadow.depth, t);
		MixValue(shadow[_T("angle")], dst.shadow.angle, t);
		MixValue(shadow[_T("blur")], dst.shadow.blur, t);

		Definition& fill = (*pDef)[_T("fill")];

		MixValue(fill[_T("color")][_T("a")], dst.fill.color[0], t);
		MixValue(fill[_T("color")][_T("r")], dst.fill.color[1], t);
		MixValue(fill[_T("color")][_T("g")], dst.fill.color[2], t);
		MixValue(fill[_T("color")][_T("b")], dst.fill.color[3], t);
		MixValue(fill[_T("width")], dst.fill.width, t);

		if(fill.m_priority >= PNormal) // this assumes there is no way to set low priority inline overrides
		{
			if(dst.fill.id > 0) throw Exception(_T("cannot apply fill more than once on the same text"));
			dst.fill.id = ++Fill::gen_id;
		}
	}

	void Subtitle::Parse(Stream& s, Style style, double at, CAtlStringMap<double> offset, Reference* pParentRef)
	{
		Text text;
		text.style = style;

		for(int c = s.PeekChar(); c != Stream::EOS; c = s.PeekChar())
		{
			s.GetChar();

			if(c == '[')
			{
				AddText(text);

				style = text.style;

				CAtlStringMap<double> inneroffset = offset;

				int default_id = 0;

				do
				{
					Definition* pDef = m_pFile->CreateDef(pParentRef);

					m_pFile->ParseRefs(s, pDef, _T(",;]"));

					ASSERT(pDef->IsType(_T("style")) || pDef->IsTypeUnknown());

					double t = GetMixWeight(pDef, at, offset, ++default_id);

					MixStyle(pDef, style, t);

					s.SkipWhiteSpace();
					c = s.GetChar();
				}
				while(c == ',' || c == ';');

				if(c != ']') s.ThrowError(_T("unterminated override"));

				bool fWhiteSpaceNext = s.IsWhiteSpace(s.PeekChar());
				c = s.SkipWhiteSpace();

				if(c == '{') 
				{
					s.GetChar();
					Parse(s, style, at, inneroffset, pParentRef);
				}
				else 
				{
					if(fWhiteSpaceNext) text.str += (TCHAR)Text::SP;
					text.style = style;
				}
			}
			else if(c == ']')
			{
				s.ThrowError(_T("unexpected ] found"));
			}
			else if(c == '{')
			{
				AddText(text);
				Parse(s, text.style, at, offset, pParentRef);
			}
			else if(c == '}')
			{
				break;
			}
			else
			{
				if(c == '\\')
				{
					c = s.GetChar();
					if(c == Stream::EOS) break;
					else if(c == 'n') {AddText(text); text.str = (TCHAR)Text::LSEP; AddText(text); continue;}
					else if(c == 'h') c = Text::NBSP;
				}

				AddChar(text, (TCHAR)c);
			}
		}

		AddText(text);
	}

	void Subtitle::AddChar(Text& t, TCHAR c)
	{
		bool f1 = !t.str.IsEmpty() && Stream::IsWhiteSpace(t.str[t.str.GetLength()-1]);
		bool f2 = Stream::IsWhiteSpace(c);
		if(f2) c = Text::SP;
		if(!f1 || !f2) t.str += (TCHAR)c;
	}

	void Subtitle::AddText(Text& t)
	{
		if(t.str.IsEmpty()) return;

		Split sa(_T(" "), t.str, 0, Split::Max);

		for(size_t i = 0, n = sa; i < n; i++)
		{
			CString str = sa[i];

			if(!str.IsEmpty())
			{
				t.str = str;
				m_text.AddTail(t);
			}

			if(i < n-1 && (m_text.IsEmpty() || m_text.GetTail().str != Text::SP))
			{
				t.str = (TCHAR)Text::SP;
				m_text.AddTail(t);
			}
		}
		
		t.str.Empty();
	}

	//

	Point Placement::TopLeft(Rect r, Size s)
	{
		r.l += margin.l;
		r.t += margin.t;
		r.r -= margin.r;
		r.b -= margin.b;

		Point p;
		if(pos.auto_x) p.x = r.l + ((r.r - r.l) - s.cx) * align.h;
		else p.x = pos.x - s.cx * align.h;
		if(pos.auto_y) p.y = r.t + ((r.b - r.t) - s.cy) * align.v;
		else p.y = pos.y - s.cy * align.v;
		return p;
	}

	unsigned int Fill::gen_id = 0;
}