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
#include "SubtitleFile.h"

namespace ssf
{
	SubtitleFile::SubtitleFile()
	{
	}

	SubtitleFile::~SubtitleFile()
	{
	}

	void SubtitleFile::Parse(Stream& s)
	{
		m_segments.RemoveAll();

		__super::Parse(s, s_predef);

		// TODO: check file.format == "ssf" and file.version == 1

		CAtlList<Definition*> defs;
		GetRootRef()->GetChildDefs(defs, _T("subtitle"));
		
		CAtlStringMap<double> offset;

		POSITION pos = defs.GetHeadPosition();
		while(pos)
		{
			Definition* pDef = defs.GetNext(pos);

			try
			{
				Definition::Time time;

				if(pDef->GetAsTime(time, offset) && (*pDef)[_T("@")].IsValue())
				{
					m_segments.Insert(time.start.value, time.stop.value, pDef);
				}
			}
			catch(Exception&)
			{
			}
		}
	}

	bool SubtitleFile::Lookup(double at, CAutoPtrList<Subtitle>& subs)
	{
		if(!subs.IsEmpty()) {ASSERT(0); return false;}

		CAtlList<SegmentItem> sis;
		m_segments.Lookup(at, sis);

		POSITION pos = sis.GetHeadPosition();
		while(pos)
		{
			SegmentItem& si = sis.GetNext(pos);

			CAutoPtr<Subtitle> s(new Subtitle(this));

			if(s->Parse(si.pDef, si.start, si.stop, at))
			{
				for(POSITION pos = subs.GetHeadPosition(); pos; subs.GetNext(pos))
				{
					if(s->m_layer < subs.GetAt(pos)->m_layer)
					{
						subs.InsertBefore(pos, s);
						break;
					}
				}

				if(s)
				{
					subs.AddTail(s);
				}
			}
		}

		return !subs.IsEmpty();
	}

	//

	SubtitleFile::Segment::Segment(double start, double stop, const SegmentItem* si)
	{
		m_start = start;
		m_stop = stop;
		if(si) AddTail(*si);
	}

	SubtitleFile::Segment::Segment(const Segment& s)
	{
		*this = s;
	}

	void SubtitleFile::Segment::operator = (const Segment& s)
	{
		m_start = s.m_start; 
		m_stop = s.m_stop; 
		RemoveAll(); 
		AddTailList(&s);
	}

	//

	void SubtitleFile::SegmentList::RemoveAll()
	{
		__super::RemoveAll();
		m_index.RemoveAll();
	}

	void SubtitleFile::SegmentList::Insert(double start, double stop, Definition* pDef)
	{
		if(start >= stop) {ASSERT(0); return;}

		m_index.RemoveAll();

		SegmentItem si = {pDef, start, stop};

		if(IsEmpty())
		{
			AddTail(Segment(start, stop, &si));
			return;
		}
		
		Segment& head = GetHead();
		Segment& tail = GetTail();
		
		if(start >= tail.m_stop && stop > tail.m_stop)
		{
			if(start > tail.m_stop) AddTail(Segment(tail.m_stop, start));
			AddTail(Segment(start, stop, &si));
		}
		else if(start < head.m_start && stop <= head.m_start)
		{
			if(stop < head.m_start) AddHead(Segment(stop, head.m_start));
			AddHead(Segment(start, stop, &si));
		}
		else 
		{
			if(start < head.m_start)
			{
				AddHead(Segment(start, head.m_start, &si));
				start = head.m_start;
			}

			if(stop > tail.m_stop)
			{
				AddTail(Segment(tail.m_stop, stop, &si));
				stop = tail.m_stop;
			}

			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos))
			{
				Segment& s = GetAt(pos);

				if(start >= s.m_stop) continue;
				if(stop <= s.m_start) break;

				if(s.m_start < start && start < s.m_stop)
				{
					Segment s2 = s;
					s2.m_start = start;
					InsertAfter(pos, s2);
					s.m_stop = start;
				}
				else if(s.m_start == start)
				{
					if(stop > s.m_stop)
					{
						start = s.m_stop;
					}
					else if(stop < s.m_stop)
					{
						Segment s2 = s;
						s2.m_start = stop;
						InsertAfter(pos, s2);
						s.m_stop = stop;
					}

					s.AddTail(si);
				}
			}
		}
	}

	size_t SubtitleFile::SegmentList::Index(bool fForce)
	{
		if(m_index.IsEmpty() || fForce)
		{
			m_index.RemoveAll();
			POSITION pos = GetHeadPosition();
			while(pos) m_index.Add(&GetNext(pos));
		}

		return m_index.GetCount();
	}

	void SubtitleFile::SegmentList::Lookup(double at, CAtlList<SegmentItem>& sis)
	{
		sis.RemoveAll();

		size_t k;
		if(Lookup(at, k)) 
		{
			sis.AddTailList(GetSegment(k));
		}
	}

	bool SubtitleFile::SegmentList::Lookup(double at, size_t& k)
	{
		if(!Index()) return false;

		size_t i = 0, j = m_index.GetCount()-1;

		if(m_index[i]->m_start <= at && at < m_index[j]->m_stop)
		do
		{
			k = (i+j)/2;
			if(m_index[k]->m_start <= at && at < m_index[k]->m_stop) {return true;}
			else if(at < m_index[k]->m_start) {if(j == k) k--; j = k;}
			else if(at >= m_index[k]->m_stop) {if(i == k) k++; i = k;}
		}
		while(i <= j);

		return false;
	}

	const SubtitleFile::Segment* SubtitleFile::SegmentList::GetSegment(size_t k)
	{
		return 0 <= k && k < m_index.GetCount() ? m_index[k] : NULL;
	}

	// TODO: this should be overridable from outside

	LPCTSTR SubtitleFile::s_predef = 
		_T("color#white {a: 255; r: 255; g: 255; b: 255;}; \n")
		_T("color#black {a: 255; r: 0; g: 0; b: 0;}; \n")
		_T("color#gray {a: 255; r: 128; g: 128; b: 128;}; \n") 
		_T("color#red {a: 255; r: 255; g: 0; b: 0;}; \n")
		_T("color#green {a: 255; r: 0; g: 255; b: 0;}; \n")
		_T("color#blue {a: 255; r: 0; g: 0; b: 255;}; \n")
		_T("color#cyan {a: 255; r: 0; g: 255; b: 255;}; \n")
		_T("color#yellow {a: 255; r: 255; g: 255; b: 0;}; \n")
		_T("color#magenta {a: 255; r: 255; g: 0; b: 255;}; \n")
		_T(" \n")
		_T("align#topleft {v: \"top\"; h: \"left\";}; \n")
		_T("align#topcenter {v: \"top\"; h: \"center\";}; \n")
		_T("align#topright {v: \"top\"; h: \"right\";}; \n")
		_T("align#middleleft {v: \"middle\"; h: \"left\";}; \n")
		_T("align#middlecenter {v: \"middle\"; h: \"center\";}; \n")
		_T("align#middleright {v: \"middle\"; h: \"right\";}; \n")
		_T("align#bottomleft {v: \"bottom\"; h: \"left\";}; \n")
		_T("align#bottomcenter {v: \"bottom\"; h: \"center\";}; \n")
		_T("align#bottomright {v: \"bottom\"; h: \"right\";}; \n")
		_T(" \n")
		_T("time#time {scale: 1;}; \n")
		_T("time#startstop {start: \"start\"; stop: \"stop\";}; \n")
		_T(" \n")
		_T("#b {font.weight: \"bold\"}; \n")
		_T("#i {font.italic: \"true\"}; \n")
		_T("#u {font.underline: \"true\"}; \n")
		_T("#s {font.strikethrough: \"true\"}; \n")
		_T(" \n")
		_T("#nobr {linebreak: \"none\"}; \n")
		_T(" \n")
		_T("subtitle#subtitle \n")
		_T("{ \n")
		_T("	frame \n")
		_T("	{ \n")
		_T("		reference: \"video\"; \n")
		_T("		resolution: {cx: 640; cy: 480;}; \n")
		_T("	}; \n")
		_T(" \n")
		_T("	direction \n")
		_T("	{ \n")
		_T("		primary: \"right\"; \n")
		_T("		secondary: \"down\"; \n")
		_T("	}; \n")
		_T(" \n")
		_T("	wrap: \"normal\"; \n")
		_T(" \n")
		_T("	layer: 0; \n")
		_T(" \n")
		_T("	style \n")
		_T("	{ \n")
		_T("		linebreak: \"word\"; \n")
		_T(" \n")		
		_T("		placement \n")
		_T("		{ \n")
		_T("			clip: \"frame\"; \n")
		_T("			margin: {t: 0; r: 0; b: 0; l: 0;}; \n")
		_T("			align: bottomcenter; \n")
		_T("			pos: \"auto\" \n")
		_T("			offset: {x: 0; y: 0;}; \n")
		_T("			angle: {x: 0; y: 0; z: 0;}; \n")
		_T("		}; \n")
		_T(" \n")
		_T("		font \n")
		_T("		{ \n")
		_T("			face: \"Arial\"; \n")
		_T("			size: 20; \n")
		_T("			weight: \"bold\"; \n")
		_T("			color: white; \n")
		_T("			underline: \"false\"; \n")
		_T("			strikethrough: \"false\"; \n")
		_T("			italic: \"false\"; \n")
		_T("			spacing: 0; \n")
		_T("			scale: {cx: 1; cy: 1;}; \n")
		_T("		}; \n")
		_T(" \n")
		_T("		background \n")
		_T("		{ \n")
		_T("			color: black; \n")
		_T("			size: 2; \n")
		_T("			type: \"outline\"; \n")
		_T("		}; \n")
		_T(" \n")
		_T("		shadow \n")
		_T("		{ \n")
		_T("			color: black {a: 128;}; \n")
		_T("			depth: 2; \n")
		_T("			angle: -45; \n")
		_T("			blur: 0; \n")
		_T("		}; \n")
		_T(" \n")
		_T("		fill \n")
		_T("		{ \n")
		_T("			color: yellow; \n")
		_T("			width: 0; \n")
		_T("		}; \n")
		_T("	}; \n")
		_T("}; \n");
}
