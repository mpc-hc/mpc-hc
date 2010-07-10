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

namespace ssf
{
	template <class T = CString, class S = CString> 
	class StringMap : public CAtlMap<S, T, CStringElementTraits<S> >
	{
	public:
		StringMap() {}
		StringMap(const StringMap& s2t) {*this = s2t;}
		StringMap& operator = (const StringMap& s2t)
		{
			RemoveAll();
			POSITION pos = s2t.GetStartPosition();
			while(pos) {const StringMap::CPair* p = s2t.GetNext(pos); SetAt(p->m_key, p->m_value);}
			return *this;
		}
	};

	template <class T = CStringA, class S = CStringA> 
	class StringMapA : public StringMap<T, S> {};

	template <class T = CStringW, class S = CStringW> 
	class StringMapW : public StringMap<T, S> {};
}