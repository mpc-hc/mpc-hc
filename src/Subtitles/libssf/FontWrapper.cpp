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
#include "FontWrapper.h"

namespace ssf
{
	FontWrapper::FontWrapper(HDC hDC, HFONT hFont, const CStringW& key)
		: m_hFont(hFont)
		, m_key(key)
	{
		HFONT hFontOld = SelectFont(hDC, hFont);

		GetTextMetrics(hDC, &m_tm);

		if (DWORD nNumPairs = GetKerningPairs(hDC, 0, NULL)) {
			KERNINGPAIR* kp = DNew KERNINGPAIR[nNumPairs];
			GetKerningPairs(hDC, nNumPairs, kp);
			for (DWORD i = 0; i < nNumPairs; i++) {
				m_kerning[(kp[i].wFirst<<16)|kp[i].wSecond] = kp[i].iKernAmount;
			}
			delete [] kp;
		}

		SelectFont(hDC, hFontOld);
	}

	FontWrapper::~FontWrapper()
	{
		DeleteFont(m_hFont);
	}

	int FontWrapper::GetKernAmount(WCHAR c1, WCHAR c2)
	{
		int size = 0;
		return m_kerning.Lookup((c1<<16)|c2, size) ? size : 0;
	}
}