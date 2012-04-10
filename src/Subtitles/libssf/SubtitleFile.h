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

#include "File.h"
#include "Subtitle.h"

namespace ssf
{
	class SubtitleFile : public File
	{
		static LPCWSTR s_predef;

	public:
		struct SegmentItem {
			Definition* pDef;
			float start, stop;
		};

		class Segment : public CAtlList<SegmentItem>
		{
		public:
			float m_start, m_stop;
			Segment() {}
			Segment(float start, float stop, const SegmentItem* si = NULL);
			Segment(const Segment& s);
			Segment& operator = (const Segment& s);
		};

		class SegmentList : public CAtlList<Segment>
		{
			CAtlArray<Segment*> m_index;
			size_t Index(bool fForce = false);

		public:
			void RemoveAll();
			void Insert(float start, float stop, Definition* pDef);
			void Lookup(float at, CAtlList<SegmentItem>& sis);
			bool Lookup(float at, size_t& k);
			const Segment* GetSegment(size_t k);
		};

		SegmentList m_segments;

	public:
		SubtitleFile();
		virtual ~SubtitleFile();

		void Parse(InputStream& s);
		void Append(InputStream& s, float start, float stop, bool fSetTime = false);
		bool Lookup(float at, CAutoPtrList<Subtitle>& subs);
	};
}