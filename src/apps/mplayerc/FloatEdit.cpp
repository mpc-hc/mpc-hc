/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "FloatEdit.h"


// CFloatEdit

IMPLEMENT_DYNAMIC(CFloatEdit, CEdit)

bool CFloatEdit::GetFloat(float& f)
{
	CString s;
	GetWindowText(s);
	return(_stscanf(s, _T("%f"), &f) == 1);
}

double CFloatEdit::operator = (double d)
{
	CString s;
	s.Format(_T("%.4f"), d);
	SetWindowText(s);
	return(d);
}

CFloatEdit::operator double()
{
	CString s;
	GetWindowText(s);
	float f;
	return(_stscanf(s, _T("%f"), &f) == 1 ? f : 0);
}

BEGIN_MESSAGE_MAP(CFloatEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CFloatEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!(nChar >= '0' && nChar <= '9' || nChar == '.' || nChar == '\b'))
		return;

	CString str;
	GetWindowText(str);

	if(nChar == '.' && (str.Find('.') >= 0 || str.IsEmpty()))
		return;

	int nStartChar, nEndChar;
	GetSel(nStartChar, nEndChar);

	if(nChar == '\b' && nStartChar <= 0)
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

// CIntEdit

IMPLEMENT_DYNAMIC(CIntEdit, CEdit)

BEGIN_MESSAGE_MAP(CIntEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CIntEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!(nChar >= '0' && nChar <= '9' || nChar == '-' || nChar == '\b'))
		return;

	CString str;
	GetWindowText(str);

	if(nChar == '-' && !str.IsEmpty() && str[0] == '-')
		return;

	int nStartChar, nEndChar;
	GetSel(nStartChar, nEndChar);

	if(nChar == '\b' && nStartChar <= 0)
		return;

	if(nChar == '-' && (nStartChar != 0 || nEndChar != 0))
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

// CHexEdit

IMPLEMENT_DYNAMIC(CHexEdit, CEdit)

bool CHexEdit::GetDWORD(DWORD& dw)
{
	CString s;
	GetWindowText(s);
	return(_stscanf(s, _T("%x"), &dw) == 1);
}

DWORD CHexEdit::operator = (DWORD dw)
{
	CString s;
	s.Format(_T("%08x"), dw);
	SetWindowText(s);
	return(dw);
}

CHexEdit::operator DWORD()
{
	CString s;
	GetWindowText(s);
	DWORD dw;
	return(_stscanf(s, _T("%x"), &dw) == 1 ? dw : 0);
}

BEGIN_MESSAGE_MAP(CHexEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CHexEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!(nChar >= 'A' && nChar <= 'F' || nChar >= 'a' && nChar <= 'f'
		|| nChar >= '0' && nChar <= '9' || nChar == '\b'))
		return;

	CString str;
	GetWindowText(str);

	int nStartChar, nEndChar;
	GetSel(nStartChar, nEndChar);

	if(nChar == '\b' && nStartChar <= 0)
		return;

	if(nChar != '\b' && nEndChar - nStartChar == 0 && str.GetLength() >= 8)
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
