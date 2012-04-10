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

namespace ssf
{
	class FontWrapper
	{
		HFONT m_hFont;
		CStringW m_key;
		TEXTMETRIC m_tm;
		CAtlMap<DWORD, int> m_kerning;

	public:
		FontWrapper(HDC hDC, HFONT hFont, const CStringW& key);
		virtual ~FontWrapper();
		operator HFONT() const {
			return m_hFont;
		}
		operator LPCWSTR() const {
			return m_key;
		}
		const TEXTMETRIC& GetTextMetric() const {
			return m_tm;
		}
		int GetKernAmount(WCHAR c1, WCHAR c2);
	};
}