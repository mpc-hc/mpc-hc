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

#include "stdafx.h"
#include "Split.h"
#include "Exception.h"

namespace ssf
{
	Split::Split(LPCWSTR sep, CStringW str, size_t limit, SplitType type)
	{
		DoSplit(sep, str, limit, type);
	}

	Split::Split(WCHAR sep, CStringW str, size_t limit, SplitType type)
	{
		DoSplit(CStringW(sep), str, limit, type);
	}

	void Split::DoSplit(LPCWSTR sep, CStringW str, size_t limit, SplitType type)
	{
		RemoveAll();

		if (size_t seplen = wcslen(sep)) {
			for (int i = 0, j = 0, len = str.GetLength();
					i <= len && (limit == 0 || GetCount() < limit);
					i = j + (int)seplen) {
				j = str.Find(sep, i);
				if (j < 0) {
					j = len;
				}

				CStringW s = i < j ? str.Mid(i, j - i) : L"";

				switch (type) {
					case Min:
						s.Trim(); // fall through
					case Def:
						if (s.IsEmpty()) {
							break;    // else fall through
						}
					case Max:
						Add(s);
						break;
				}
			}
		}
	}

	int Split::GetAtInt(size_t i)
	{
		if (i >= GetCount()) {
			throw Exception(_T("Index out of bounds"));
		}
		return _wtoi(GetAt(i));
	}

	float Split::GetAtFloat(size_t i)
	{
		if (i >= GetCount()) {
			throw Exception(_T("Index out of bounds"));
		}
		return (float)_wtof(GetAt(i));
	}
}