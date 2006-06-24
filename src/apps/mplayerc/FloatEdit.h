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

// CFloatEdit

class CFloatEdit : public CEdit
{
public:
	bool GetFloat(float& f);
	double operator = (double d);
	operator double();

	DECLARE_DYNAMIC(CFloatEdit)
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CIntEdit

class CIntEdit : public CEdit
{
public:
	DECLARE_DYNAMIC(CIntEdit)
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CHexEdit

class CHexEdit : public CEdit
{
public:
	bool GetDWORD(DWORD& dw);
	DWORD operator = (DWORD dw);
	operator DWORD();

	DECLARE_DYNAMIC(CHexEdit)
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};
