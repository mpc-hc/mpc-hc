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

	void SubtitleFile::Parse(InputStream& s)
	{
		m_segments.RemoveAll();

		__super::Parse(s, s_predef);

		// TODO: check file.format == "ssf" and file.version == 1

		CAtlList<Definition*> defs;
		GetRootRef()->GetChildDefs(defs, L"subtitle");
		
		StringMapW<float> offset;

		POSITION pos = defs.GetHeadPosition();
		while(pos)
		{
			Definition* pDef = defs.GetNext(pos);

			try
			{
				Definition::Time time;

				if(pDef->GetAsTime(time, offset) && (*pDef)[L"@"].IsValue())
				{
					m_segments.Insert(time.start.value, time.stop.value, pDef);
				}
			}
			catch(Exception&)
			{
			}
		}
	}

	void SubtitleFile::Append(InputStream& s, float start, float stop, bool fSetTime)
	{
		Reference* pRootRef = GetRootRef();

		ParseDefs(s, pRootRef);

		CAtlList<Definition*> defs;
		GetNewDefs(defs);

		POSITION pos = defs.GetHeadPosition();
		while(pos)
		{
			Definition* pDef = defs.GetNext(pos);

			if(pDef->m_parent == pRootRef && pDef->m_type == L"subtitle" && (*pDef)[L"@"].IsValue())
			{
				m_segments.Insert(start, stop, pDef);

				if(fSetTime) 
				{
					try
					{
						Definition::Time time;
						StringMapW<float> offset;
						pDef->GetAsTime(time, offset);
						if(time.start.value == start && time.stop.value == stop)
							continue;
					}
					catch(Exception&)
					{
					}

					CStringW str;
					str.Format(L"%.3f", start);
					pDef->SetChildAsNumber(L"time.start", str, L"s");
					str.Format(L"%.3f", stop);
					pDef->SetChildAsNumber(L"time.stop", str, L"s");
				}
			}
		}

		Commit();
	}

	bool SubtitleFile::Lookup(float at, CAutoPtrList<Subtitle>& subs)
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

	SubtitleFile::Segment::Segment(float start, float stop, const SegmentItem* si)
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

	void SubtitleFile::SegmentList::Insert(float start, float stop, Definition* pDef)
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

	void SubtitleFile::SegmentList::Lookup(float at, CAtlList<SegmentItem>& sis)
	{
		sis.RemoveAll();

		size_t k;
		if(Lookup(at, k)) 
		{
			sis.AddTailList(GetSegment(k));
		}
	}

	bool SubtitleFile::SegmentList::Lookup(float at, size_t& k)
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

	LPCWSTR SubtitleFile::s_predef = 
		L"color#white {a: 255; r: 255; g: 255; b: 255;}; \n"
		L"color#black {a: 255; r: 0; g: 0; b: 0;}; \n"
		L"color#gray {a: 255; r: 128; g: 128; b: 128;}; \n" 
		L"color#red {a: 255; r: 255; g: 0; b: 0;}; \n"
		L"color#green {a: 255; r: 0; g: 255; b: 0;}; \n"
		L"color#blue {a: 255; r: 0; g: 0; b: 255;}; \n"
		L"color#cyan {a: 255; r: 0; g: 255; b: 255;}; \n"
		L"color#yellow {a: 255; r: 255; g: 255; b: 0;}; \n"
		L"color#magenta {a: 255; r: 255; g: 0; b: 255;}; \n"
		L" \n"
		L"align#topleft {v: \"top\"; h: \"left\";}; \n"
		L"align#topcenter {v: \"top\"; h: \"center\";}; \n"
		L"align#topright {v: \"top\"; h: \"right\";}; \n"
		L"align#middleleft {v: \"middle\"; h: \"left\";}; \n"
		L"align#middlecenter {v: \"middle\"; h: \"center\";}; \n"
		L"align#middleright {v: \"middle\"; h: \"right\";}; \n"
		L"align#bottomleft {v: \"bottom\"; h: \"left\";}; \n"
		L"align#bottomcenter {v: \"bottom\"; h: \"center\";}; \n"
		L"align#bottomright {v: \"bottom\"; h: \"right\";}; \n"
		L" \n"
		L"time#time {scale: 1;}; \n"
		L"time#startstop {start: \"start\"; stop: \"stop\";}; \n"
		L" \n"
		L"#b {font.weight: \"bold\"}; \n"
		L"#i {font.italic: \"true\"}; \n"
		L"#u {font.underline: \"true\"}; \n"
		L"#s {font.strikethrough: \"true\"}; \n"
		L" \n"
		L"#nobr {linebreak: \"none\"}; \n"
		L" \n"
		L"subtitle#subtitle \n"
		L"{ \n"
		L"	frame \n"
		L"	{ \n"
		L"		reference: \"video\"; \n"
		L"		resolution: {cx: 640; cy: 480;}; \n"
		L"	}; \n"
		L" \n"
		L"	direction \n"
		L"	{ \n"
		L"		primary: \"right\"; \n"
		L"		secondary: \"down\"; \n"
		L"	}; \n"
		L" \n"
		L"	wrap: \"normal\"; \n"
		L" \n"
		L"	layer: 0; \n"
		L" \n"
		L"	style \n"
		L"	{ \n"
		L"		linebreak: \"word\"; \n"
		L" \n"		
		L"		placement \n"
		L"		{ \n"
		L"			clip: \"none\"; \n"
		L"			margin: {t: 0; r: 0; b: 0; l: 0;}; \n"
		L"			align: bottomcenter; \n"
		L"			pos: \"auto\" \n"
		L"			offset: {x: 0; y: 0;}; \n"
		L"			angle: {x: 0; y: 0; z: 0;}; \n"
		L"			org: \"auto\" \n"
		L"			path: \"\"; \n"
		L"		}; \n"
		L" \n"
		L"		font \n"
		L"		{ \n"
		L"			face: \"Arial\"; \n"
		L"			size: 20; \n"
		L"			weight: \"bold\"; \n"
		L"			color: white; \n"
		L"			underline: \"false\"; \n"
		L"			strikethrough: \"false\"; \n"
		L"			italic: \"false\"; \n"
		L"			spacing: 0; \n"
		L"			scale: {cx: 1; cy: 1;}; \n"
		L"			kerning: \"true\"; \n"
		L"		}; \n"
		L" \n"
		L"		background \n"
		L"		{ \n"
		L"			color: black; \n"
		L"			size: 2; \n"
		L"			type: \"outline\"; \n"
		L"			blur: 0; \n"
		L"		}; \n"
		L" \n"
		L"		shadow \n"
		L"		{ \n"
		L"			color: black {a: 128;}; \n"
		L"			depth: 2; \n"
		L"			angle: -45; \n"
		L"			blur: 0; \n"
		L"		}; \n"
		L" \n"
		L"		fill \n"
		L"		{ \n"
		L"			color: yellow; \n"
		L"			width: 0; \n"
		L"		}; \n"
		L"	}; \n"
		L"}; \n";
}
